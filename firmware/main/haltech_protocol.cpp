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
        out.kind = Kind::SpeedCombined;
        out.speed_tenths_kmh = static_cast<uint16_t>(
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

} // namespace haltech
