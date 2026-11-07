#pragma once

#include "haltech_protocol.h"
#include "esp_err.h"
#include <cstdint>

namespace can_capture {

struct Stats {
    uint32_t dropped_queue = 0;
    uint32_t bus_errors = 0;
    uint16_t rx_error_counter = 0;
    int error_state = -1;
};

esp_err_t start();  // CAN1 only, 1 Mbit/s, listen-only. No transmit path.
bool receive(haltech::Frame& frame, uint32_t timeout_ms);
Stats stats();

} // namespace can_capture
