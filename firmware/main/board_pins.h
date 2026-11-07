#pragma once

namespace board {
constexpr int led = 2;
constexpr int can1_rx = 6;
constexpr int can1_tx = 7;
// Reserved for CAN2's MCP2515; not initialized in this revision.
constexpr int can2_cs = 10;
constexpr int can2_mosi = 11;
constexpr int can2_sck = 12;
constexpr int can2_miso = 13;
constexpr int can2_int = 3;
constexpr int can2_osc_hz = 16000000;
constexpr int can1_bitrate = 1000000;
} // namespace board
