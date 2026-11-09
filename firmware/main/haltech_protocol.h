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
    uint16_t speed_raw = 0;
    int8_t selector = 0;
    int8_t gear = 0;
};

bool is_target_id(uint32_t id);
DecodeStatus decode(const Frame& frame, Decoded& output);
const char* gear_name(int8_t value);

class GearEcho {
public:
    static constexpr uint64_t tx_timeout_us = 50000;

    // Call for a decoded 0x470 GearSeparate sample.
    void observe(const Frame& source, int8_t actual_gear);
    bool next_frame(uint64_t now_us, Frame& output);

private:
    uint64_t received_us_ = 0;
    uint64_t next_attempt_us_ = 0;
    int8_t gear_ = 0;
    bool valid_ = false;
};

} // namespace haltech
