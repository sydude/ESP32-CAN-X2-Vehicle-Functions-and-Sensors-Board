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

static void test_gear_echo() {
    GearEcho echo;
    Frame output;
    assert(!echo.next_frame(0, output));

    const uint8_t reverse_in_drive[8] = {0, 0, 0, 0, 0, 0, 0xFC, 0xFF};
    Frame source = test_frame(0x470, reverse_in_drive);
    Decoded decoded;
    assert(decode(source, decoded) == DecodeStatus::Ok);
    assert(decoded.kind == Kind::GearSeparate && decoded.selector == -4 && decoded.gear == -1);
    source.timestamp_us = 100000;
    source.synthetic = true;
    echo.observe(source, decoded.gear);
    assert(!echo.next_frame(100000, output));
    source.synthetic = false;
    echo.observe(source, decoded.gear);
    assert(!echo.next_frame(99999, output));
    assert(echo.next_frame(100000, output));
    assert(output.id == 0x650 && output.len == 8 && !output.extended && !output.remote);
    assert(output.data[0] == 0xFF);
    for (unsigned i = 1; i < 8; ++i) assert(output.data[i] == 0);

    source.timestamp_us = 250000;
    echo.observe(source, 0);
    assert(!echo.next_frame(299999, output));
    assert(echo.next_frame(300000, output) && output.data[0] == 0);
    assert(!echo.next_frame(500000, output)); // The sample expired in the RX queue.

    source.timestamp_us = 500000;
    echo.observe(source, 10);
    echo.observe(source, -3); // An explicit Unknown invalidates the prior sample.
    assert(!echo.next_frame(500000, output));
    echo.observe(source, 11);
    assert(!echo.next_frame(500000, output));
    echo.observe(source, 10);
    assert(!echo.next_frame(690000, output)); // Too old to allow a full TX window.
    assert(echo.next_frame(689999, output) && output.data[0] == 10);
}

int main() {
    Decoded d = {};
    const uint8_t trims[]   = {0, 0, 0, 0, 3, 0xFE, 7, 0};
    const uint8_t pedal[]   = {0x01, 0x59, 3, 3, 0, 0, 0, 0}; // 345 = 34.5% at x10
    const uint8_t gears[]   = {0x03, 0xE8, 0x03, 0xE8, 0x03, 0xE8, 3, 3};
    const uint8_t neutral[] = {0, 0, 0, 0, 0, 0, 0, 0};
    const uint8_t reverse[] = {0, 0, 0, 0, 0, 0, 0xFF, 0xFF};
    const uint8_t drive[]   = {0, 0, 0xFC, 3, 0, 0, 0, 0};
    assert(decode(test_frame(0x3E4, trims), d) == DecodeStatus::Ok);
    assert(d.kind == Kind::Trims && d.trim[0] == 3 && d.trim[1] == -2 && d.trim[2] == 7);
    assert(decode(test_frame(0x370, pedal), d) == DecodeStatus::Ok);
    assert(d.kind == Kind::PedalCombined && d.pedal_raw == 345);
    assert(d.selector == 3 && d.gear == 3);
    const uint8_t app_full[] = {0x03, 0xE8, 3, 3, 0, 0, 0, 0}; // 100.0% APP
    assert(decode(test_frame(0x370, app_full), d) == DecodeStatus::Ok);
    assert(d.kind == Kind::PedalCombined && d.pedal_raw == 1000);
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
    bad.extended = false;
    bad.remote = true;
    assert(decode(bad, d) == DecodeStatus::NotStandardData);
    assert(decode(test_frame(0x123, gears), d) == DecodeStatus::UnknownId);
    assert(gear_name(-8) && gear_name(10));
    test_gear_echo();
    std::cout << "Haltech decoder and GearEcho test vectors: PASS\n";
}
