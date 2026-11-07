#include "haltech_protocol.h"
#include <cassert>
#include <cstdint>
#include <iostream>

using namespace haltech;

static Frame test_frame(uint32_t id, const uint8_t (&bytes)[8]) {
    Frame frame = {};
    frame.id = id;
    frame.len = 8;
    for (int i = 0; i < 8; ++i) frame.data[i] = bytes[i];
    return frame;
}

int main() {
    Decoded d = {};
    const uint8_t trims[]   = {0, 0, 0, 0, 3, 0xFE, 7, 0};
    const uint8_t speed[]   = {0x01, 0x59, 3, 3, 0, 0, 0, 0};
    const uint8_t gears[]   = {0x03, 0xE8, 0x03, 0xE8, 0x03, 0xE8, 3, 3};
    const uint8_t neutral[] = {0, 0, 0, 0, 0, 0, 0, 0};
    const uint8_t reverse[] = {0, 0, 0, 0, 0, 0, 0xFF, 0xFF};
    const uint8_t drive[]   = {0, 0, 0xFC, 3, 0, 0, 0, 0};
    assert(decode(test_frame(0x3E4, trims), d) == DecodeStatus::Ok);
    assert(d.kind == Kind::Trims && d.trim[0] == 3 && d.trim[1] == -2 && d.trim[2] == 7);
    assert(decode(test_frame(0x370, speed), d) == DecodeStatus::Ok);
    assert(d.kind == Kind::SpeedCombined && d.speed_tenths_kmh == 345);
    assert(d.selector == 3 && d.gear == 3);
    assert(decode(test_frame(0x470, gears), d) == DecodeStatus::Ok);
    assert(d.selector == 3 && d.gear == 3);
    assert(decode(test_frame(0x470, neutral), d) == DecodeStatus::Ok);
    assert(d.selector == 0 && d.gear == 0);
    assert(decode(test_frame(0x470, reverse), d) == DecodeStatus::Ok);
    assert(d.selector == -1 && d.gear == -1);
    assert(decode(test_frame(0x370, drive), d) == DecodeStatus::Ok);
    assert(d.selector == -4 && d.gear == 3); // Legitimately different fields.
    const uint8_t boundaries[] = {0, 0, 0, 0, 0x80, 0x7F, 0, 0};
    assert(decode(test_frame(0x3E4, boundaries), d) == DecodeStatus::Ok);
    assert(d.trim[0] == -128 && d.trim[1] == 127 && d.trim[2] == 0);
    auto bad = test_frame(0x470, gears);
    bad.len = 7;
    assert(decode(bad, d) == DecodeStatus::BadLength);
    bad = test_frame(0x470, gears);
    bad.extended = true;
    assert(decode(bad, d) == DecodeStatus::NotStandardData);
    assert(decode(test_frame(0x123, gears), d) == DecodeStatus::UnknownId);
    assert(gear_name(-8) && gear_name(10));
    std::cout << "Haltech decoder test vectors: PASS\n";
}
