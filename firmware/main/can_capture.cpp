#include "can_capture.h"
#include "board_pins.h"
#include "esp_log.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <atomic>

namespace can_capture {
namespace {
constexpr unsigned kQueueDepth = 128;
constexpr char kTag[] = "can1";
constexpr uint64_t kTxBackoffUs = 1000000;
QueueHandle_t rx_queue = nullptr;
twai_node_handle_t node = nullptr;
std::atomic<uint32_t> queue_drops{0};
std::atomic<uint32_t> bus_off_events{0};
std::atomic<uint32_t> tx_ok{0};
std::atomic<uint32_t> tx_failed{0};

enum class TxState : uint32_t { Idle, Pending, Failed };
std::atomic<TxState> tx_state{TxState::Idle};
uint32_t tx_rejected = 0;
uint32_t tx_timeouts = 0;
uint64_t tx_started_us = 0;
uint64_t tx_resume_us = 0;
bool bus_off_reported = false;

// The driver retains both pointers until its completion callback runs.
uint8_t tx_data[8] = {};
twai_frame_t tx_frame = {};

bool on_tx_done(twai_node_handle_t, const twai_tx_done_event_data_t* event, void*) {
    if (event->is_tx_success) {
        tx_ok.fetch_add(1, std::memory_order_relaxed);
    } else {
        tx_failed.fetch_add(1, std::memory_order_relaxed);
    }
    tx_state.store(event->is_tx_success ? TxState::Idle : TxState::Failed,
                   std::memory_order_release);
    return false;
}

bool on_state_change(twai_node_handle_t, const twai_state_change_event_data_t* event, void*) {
    if (event->new_sta == TWAI_ERROR_BUS_OFF) {
        bus_off_events.fetch_add(1, std::memory_order_relaxed);
    }
    return false;
}

bool on_rx(twai_node_handle_t handle, const twai_rx_done_event_data_t*, void*) {
    uint8_t bytes[8] = {};
    twai_frame_t rx = {};
    rx.buffer = bytes;
    rx.buffer_len = sizeof(bytes);
    if (twai_node_receive_from_isr(handle, &rx) != ESP_OK) return false;

    haltech::Frame frame = {};
    frame.id = rx.header.id;
    frame.timestamp_us = rx.header.timestamp;
    frame.len = static_cast<uint8_t>(rx.header.dlc);
    frame.extended = rx.header.ide;
    frame.remote = rx.header.rtr;
    if (frame.len > 8) return false; // Classical CAN only.
    for (unsigned i = 0; i < frame.len; ++i) frame.data[i] = bytes[i];

    BaseType_t higher_priority_woken = pdFALSE;
    if (xQueueSendFromISR(rx_queue, &frame, &higher_priority_woken) != pdTRUE) {
        queue_drops.fetch_add(1, std::memory_order_relaxed);
    }
    return higher_priority_woken == pdTRUE;
}
} // namespace

esp_err_t start() {
    rx_queue = xQueueCreate(kQueueDepth, sizeof(haltech::Frame));
    if (!rx_queue) return ESP_ERR_NO_MEM;
    twai_onchip_node_config_t cfg = {};
    cfg.io_cfg.tx = static_cast<gpio_num_t>(board::can1_tx);
    cfg.io_cfg.rx = static_cast<gpio_num_t>(board::can1_rx);
    cfg.io_cfg.quanta_clk_out = static_cast<gpio_num_t>(-1);
    cfg.io_cfg.bus_off_indicator = static_cast<gpio_num_t>(-1);
    cfg.bit_timing.bitrate = board::can1_bitrate;
    cfg.tx_queue_depth = 1;
    cfg.fail_retry_cnt = 0; // Single shot. A failed echo is never retried.
    cfg.timestamp_resolution_hz = 1000000;
    esp_err_t result = twai_new_node_onchip(&cfg, &node);
    if (result != ESP_OK) return result;
    twai_event_callbacks_t callbacks = {};
    callbacks.on_rx_done = on_rx;
    callbacks.on_tx_done = on_tx_done;
    callbacks.on_state_change = on_state_change;
    result = twai_node_register_event_callbacks(node, &callbacks, nullptr);
    if (result != ESP_OK) return result;
    return twai_node_enable(node);
}

bool receive(haltech::Frame& frame, uint32_t timeout_ms) {
    return rx_queue && xQueueReceive(rx_queue, &frame, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void service(uint64_t now_us) {
    if (!node) return;
    if (!bus_off_reported && bus_off_events.load(std::memory_order_relaxed)) {
        bus_off_reported = true;
        // Recovery can replay pending driver frames. Keep CAN1 stopped until restart.
        ESP_LOGE(kTag, "TWAI bus-off. Restart after correcting the CAN fault.");
    }

    TxState state = tx_state.load(std::memory_order_acquire);
    if (state == TxState::Failed) {
        tx_resume_us = now_us + kTxBackoffUs;
        tx_state.store(TxState::Idle, std::memory_order_relaxed);
    } else if (state == TxState::Pending && !bus_off_reported && !tx_timeouts &&
               now_us - tx_started_us >= haltech::GearEcho::tx_timeout_us) {
        // Abort a hung transmission. Re-enabling the node can replay it.
        ++tx_timeouts;
        const esp_err_t err = twai_node_disable(node);
        ESP_LOGE(kTag, "GearEcho TX completion timeout. CAN1 stop result=%s. Restart required.",
                 esp_err_to_name(err));
    }
}

void send_gear_echo(const haltech::Frame& frame, uint64_t now_us) {
    if (tx_timeouts || bus_off_events.load(std::memory_order_relaxed) ||
        now_us < tx_resume_us || tx_state.load(std::memory_order_acquire) != TxState::Idle) return;
    // Wait until the ISR has retired the completed frame before reusing the buffer.
    if (twai_node_transmit_wait_all_done(node, 0) != ESP_OK) return;
    tx_frame.header.id = frame.id;
    tx_frame.header.dlc = frame.len;
    tx_frame.buffer = tx_data;
    tx_frame.buffer_len = sizeof(tx_data);
    for (unsigned i = 0; i < sizeof(tx_data); ++i) tx_data[i] = frame.data[i];
    tx_started_us = now_us;
    tx_state.store(TxState::Pending, std::memory_order_release);
    const esp_err_t err = twai_node_transmit(node, &tx_frame, 0);
    if (err != ESP_OK) {
        tx_state.store(TxState::Idle, std::memory_order_relaxed);
        ++tx_rejected;
        tx_resume_us = now_us + kTxBackoffUs;
        ESP_LOGW(kTag, "GearEcho TX rejected: %s", esp_err_to_name(err));
    }
}

Stats stats() {
    Stats out = {};
    out.dropped_queue = queue_drops.load(std::memory_order_relaxed);
    out.bus_off_events = bus_off_events.load(std::memory_order_relaxed);
    out.tx_ok = tx_ok.load(std::memory_order_relaxed);
    out.tx_failed = tx_failed.load(std::memory_order_relaxed);
    out.tx_rejected = tx_rejected;
    out.tx_timeouts = tx_timeouts;
    if (!node) return out;
    twai_node_status_t status = {};
    twai_node_record_t record = {};
    if (twai_node_get_info(node, &status, &record) == ESP_OK) {
        out.bus_errors = record.bus_err_num;
        out.rx_error_counter = status.rx_error_count;
        out.tx_error_counter = status.tx_error_count;
        out.error_state = static_cast<int>(status.state);
    }
    return out;
}

} // namespace can_capture
