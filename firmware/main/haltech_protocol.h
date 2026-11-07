#pragma once

#include <cstdint>

namespace haltech {

struct Frame {
    uint32_t id = 0;
    uint64_t timestamp_us = 0;
    uint8_t len = 0;
    uint8_t data[8] = {};
    bool extended = false;
    bool remote = false;
    bool synthetic = false;
};

enum class Kind { Trims, SpeedCombined, GearSeparate };
enum class DecodeStatus { Ok, UnknownId, BadLength, NotStandardData };

struct Decoded {
    Kind kind = Kind::Trims;
    int8_t trim[3] = {};
    uint16_t speed_tenths_kmh = 0;
    int8_t selector = 0;
    int8_t gear = 0;
};

bool is_target_id(uint32_t id);
DecodeStatus decode(const Frame& frame, Decoded& output);
const char* gear_name(int8_t value);

} // namespace haltech
