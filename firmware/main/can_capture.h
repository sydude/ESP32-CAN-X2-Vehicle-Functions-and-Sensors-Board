#pragma once

#include "haltech_protocol.h"
#include "esp_err.h"
#include <cstdint>

namespace can_capture {

struct Stats {
    uint32_t dropped_queue = 0;
    uint32_t bus_errors = 0;
    uint32_t tx_ok = 0;
    uint32_t tx_failed = 0;
    uint32_t tx_rejected = 0;
    uint32_t tx_timeouts = 0;
    uint32_t bus_off_events = 0;
    uint16_t rx_error_counter = 0;
    uint16_t tx_error_counter = 0;
    int error_state = -1;
};

esp_err_t start();  // CAN1 only, 1 Mbit/s, normal mode with hardware ACK.
bool receive(haltech::Frame& frame, uint32_t timeout_ms);
void service(uint64_t now_us);
void send_gear_echo(const haltech::Frame& frame, uint64_t now_us);
Stats stats();

} // namespace can_capture
