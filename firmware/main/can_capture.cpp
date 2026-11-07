#include "can_capture.h"
#include "board_pins.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace can_capture {
namespace {
constexpr unsigned kQueueDepth = 128;
QueueHandle_t rx_queue = nullptr;
twai_node_handle_t node = nullptr;
volatile uint32_t queue_drops = 0;

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
        ++queue_drops;
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
    cfg.tx_queue_depth = 5;
    cfg.timestamp_resolution_hz = 1000000;
    cfg.flags.enable_listen_only = true;
    esp_err_t result = twai_new_node_onchip(&cfg, &node);
    if (result != ESP_OK) return result;
    twai_event_callbacks_t callbacks = {};
    callbacks.on_rx_done = on_rx;
    result = twai_node_register_event_callbacks(node, &callbacks, nullptr);
    if (result != ESP_OK) return result;
    return twai_node_enable(node);
}

bool receive(haltech::Frame& frame, uint32_t timeout_ms) {
    return rx_queue && xQueueReceive(rx_queue, &frame, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

Stats stats() {
    Stats out = {};
    out.dropped_queue = queue_drops;
    if (!node) return out;
    twai_node_status_t status = {};
    twai_node_record_t record = {};
    if (twai_node_get_info(node, &status, &record) == ESP_OK) {
        out.bus_errors = record.bus_err_num;
        out.rx_error_counter = status.rx_error_count;
        out.error_state = static_cast<int>(status.state);
    }
    return out;
}

} // namespace can_capture
