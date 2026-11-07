#include "monitor.h"
#include "can_capture.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include <cstdio>

namespace monitor {
namespace {
constexpr char kTag[] = "haltech";
uint64_t all_frames = 0;
uint64_t targets = 0;
uint32_t invalid = 0;
int64_t last_summary_us = 0;
int64_t last_speed_print_us = 0;
int64_t mismatch_since_us = 0;
bool mismatch_logged = false;

struct State {
    bool seen = false;
    int8_t selector = 0;
    int8_t gear = 0;
    int64_t last_us = 0;
};
State combined, separate;
bool trims_seen = false;
int64_t last_trims_us = 0;
int8_t trims[3] = {};
bool speed_seen = false;
uint16_t latest_speed = 0;
uint16_t printed_speed = 0;

void report_gear(const char* source, State& state, int8_t selector, int8_t gear, int64_t now) {
    bool changed = !state.seen || state.selector != selector || state.gear != gear;
    state = {true, selector, gear, now};
    if (changed) ESP_LOGI(kTag, "%s selector=%d (%s), gear=%d (%s)",
        source, selector, haltech::gear_name(selector), gear, haltech::gear_name(gear));
}

void cross_check(int64_t now) {
    if (!combined.seen || !separate.seen) return;
    // Require both frames to be recent; do not treat asynchronous samples as simultaneous.
    if (now - combined.last_us > 150000 || now - separate.last_us > 150000) return;
    // Compare the same channel across the two representations, never selector vs gear.
    bool mismatch = combined.selector != separate.selector || combined.gear != separate.gear;
    if (!mismatch) {
        mismatch_since_us = 0;
        mismatch_logged = false;
    } else if (mismatch_since_us == 0) {
        mismatch_since_us = now;
    } else if (!mismatch_logged && now - mismatch_since_us >= 300000) {
        ESP_LOGW(kTag, "Persistent packed/separate gear disagreement: 370=%d/%d 470=%d/%d",
            combined.selector, combined.gear, separate.selector, separate.gear);
        mismatch_logged = true;
    }
}
} // namespace

void ingest(const haltech::Frame& frame) {
    ++all_frames;
#if CONFIG_CANX2_RAW_LOG
    char hex[3 * 8 + 1] = {};
    for (unsigned i = 0; i < frame.len && i < 8; ++i) {
        snprintf(hex + 3 * i, sizeof(hex) - 3 * i, "%02X ", frame.data[i]);
    }
    ESP_LOGI(kTag, "%s RX id=0x%03lX dlc=%u t=%llu %s",
        frame.synthetic ? "SYN" : "CAN", static_cast<unsigned long>(frame.id),
        frame.len, static_cast<unsigned long long>(frame.timestamp_us), hex);
#endif
    haltech::Decoded decoded = {};
    const auto result = haltech::decode(frame, decoded);
    if (result == haltech::DecodeStatus::UnknownId) return;
    if (result != haltech::DecodeStatus::Ok) {
        ++invalid;
        ESP_LOGW(kTag, "Rejected CAN 0x%03lX: malformed/unsupported frame",
                 static_cast<unsigned long>(frame.id));
        return;
    }
    ++targets;
    const int64_t now = esp_timer_get_time();

    switch (decoded.kind) {
        case haltech::Kind::Trims: {
            const bool changed = !trims_seen ||
                trims[0] != decoded.trim[0] || trims[1] != decoded.trim[1] ||
                trims[2] != decoded.trim[2];
            trims_seen = true;
            last_trims_us = now;
            for (int i = 0; i < 3; ++i) trims[i] = decoded.trim[i];
            if (changed) ESP_LOGI(kTag, "Rotary trims: %d, %d, %d (raw signed)",
                trims[0], trims[1], trims[2]);
            break;
        }
        case haltech::Kind::SpeedCombined:
            latest_speed = decoded.speed_tenths_kmh;
            // Report first sample immediately; subsequently at most once per second,
            // and only when the value differs from the last printed value.
            if (!speed_seen || (latest_speed != printed_speed &&
                now - last_speed_print_us >= 1000000)) {
                ESP_LOGI(kTag, "Vehicle speed: %u.%u km/h",
                    latest_speed / 10, latest_speed % 10);
                printed_speed = latest_speed;
                last_speed_print_us = now;
            }
            speed_seen = true;
            report_gear("0x370 packed", combined, decoded.selector, decoded.gear, now);
            break;
        case haltech::Kind::GearSeparate:
            report_gear("0x470 separate", separate, decoded.selector, decoded.gear, now);
            break;
    }
    cross_check(now);
}

void poll() {
    const int64_t now = esp_timer_get_time();
    if (last_summary_us && now - last_summary_us < 5000000) return;
    last_summary_us = now;
    const auto stats = can_capture::stats();
    ESP_LOGI(kTag, "Summary received=%llu targeted=%llu invalid=%lu dropped=%lu bus_errors=%lu rx_err=%u state=%d",
        static_cast<unsigned long long>(all_frames),
        static_cast<unsigned long long>(targets),
        static_cast<unsigned long>(invalid),
        static_cast<unsigned long>(stats.dropped_queue),
        static_cast<unsigned long>(stats.bus_errors),
        stats.rx_error_counter, stats.error_state);

    if (trims_seen && now - last_trims_us > 650000) {
        ESP_LOGW(kTag, "No fresh 0x3E4 rotary-trim message for >650ms");
    }
    const State* tracked[] = {&combined, &separate};
    const char* labels[] = {"0x370", "0x470"};
    for (int i = 0; i < 2; ++i) {
        if (tracked[i]->seen && now - tracked[i]->last_us > 250000) {
            ESP_LOGW(kTag, "No fresh %s gear message for >250ms", labels[i]);
        }
    }
}

} // namespace monitor
