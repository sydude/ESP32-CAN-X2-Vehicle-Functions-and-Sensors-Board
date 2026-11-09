#include "haltech_protocol.h"

namespace haltech {

bool is_target_id(uint32_t id) {
    return id == 0x3E4 || id == 0x370 || id == 0x470;
}

DecodeStatus decode(const Frame& frame, Decoded& out) {
    if (!is_target_id(frame.id)) return DecodeStatus::UnknownId;
    if (frame.extended || frame.remote) return DecodeStatus::NotStandardData;
    if (frame.len != 8) return DecodeStatus::BadLength;

    if (frame.id == 0x3E4) {
        out.kind = Kind::Trims;
        for (int i = 0; i < 3; ++i) out.trim[i] = static_cast<int8_t>(frame.data[4 + i]);
    } else if (frame.id == 0x370) {
        // Custom Generic CAN sends APP position here, NOT native broadcast vehicle speed.
        out.kind = Kind::PedalCombined;
        out.pedal_raw = static_cast<uint16_t>(
            (static_cast<uint16_t>(frame.data[0]) << 8) | frame.data[1]);
        // "Combined Gear" is a packed selector and gear, not a third sensor.
        out.selector = static_cast<int8_t>(frame.data[2]);
        out.gear = static_cast<int8_t>(frame.data[3]);
    } else {
        out.kind = Kind::GearSeparate;
        out.selector = static_cast<int8_t>(frame.data[6]);
        out.gear = static_cast<int8_t>(frame.data[7]);
    }
    return DecodeStatus::Ok;
}

const char* gear_name(int8_t value) {
    switch (value) {
        case 0: return "Neutral";
        case -1: return "Reverse";
        case -2: return "Park";
        case -3: return "Unknown";
        case -4: return "Drive";
        case -5: return "Sport";
        case -6: return "Manual";
        case -7: return "Low";
        case -8: return "Overdrive";
        default: return value >= 1 && value <= 10 ? "Forward" : "Unspecified";
    }
}

void GearEcho::observe(const Frame& source, int8_t actual_gear) {
    if (source.synthetic) return;
    // Haltech defines -3 as Unknown. The other documented codes are echoed as-is.
    valid_ = actual_gear >= -8 && actual_gear <= 10 && actual_gear != -3;
    gear_ = actual_gear;
    received_us_ = source.timestamp_us;
}

bool GearEcho::next_frame(uint64_t now_us, Frame& output) {
    constexpr uint64_t kPeriodUs = 200000;
    constexpr uint64_t kFreshnessUs = 250000;
    constexpr uint64_t kWatchdogPollUs = 10000;
    // Budget for transmission and the next watchdog poll before the sample expires.
    if (!valid_ || now_us < received_us_ || now_us < next_attempt_us_ ||
        now_us - received_us_ >= kFreshnessUs - tx_timeout_us - kWatchdogPollUs) return false;

    output = {};
    output.id = 0x650;
    output.len = 8;
    output.data[0] = static_cast<uint8_t>(gear_);
    next_attempt_us_ = now_us + kPeriodUs;
    return true;
}

} // namespace haltech
