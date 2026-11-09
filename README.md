# ESP32-CAN-X2 — Vehicle Functions and Sensors

Firmware for an **Autosport Labs ESP32-CAN-X2 (ESP32-S3)** operating as the vehicle communications and sensor/peripheral interface. A separate Waveshare ESP32-P4-WIFI6-DB is the primary application and display/UI controller; that firmware is maintained separately.

## Current scope

- CAN1 uses native ESP32-S3 TWAI in normal mode at **1,000,000 bit/s** with hardware ACK. The default receives Haltech frames. GearEcho on `0x650` is opt-in.
- Initial custom Haltech Generic CAN decoding, borrowing some Broadcast v2.0 layouts: 0x3E4 (rotary trims 1–3), 0x370 (**accelerator pedal position**, not TPS, and packed combined gear), 0x470 (gear selector position and gear).
- Change-only rotary/gear logs; rate-limited accelerator-pedal logs; 5-second reception/error summaries.
- Opt-in synthetic decoder diagnostics which **do not initialize or transmit onto CAN**.
- CAN2/MCP2515 is unused. Sensor acquisition and S3–P4 communication are future work.

See the [firmware guide](firmware/readme.md) for configuration, tests, and wiring.

## References

- [Autosport Labs board documentation](https://wiki.autosportlabs.com/ESP32-CAN-X2)
- [Vendor source submodule](Vendors/ESP32-CAN-X2)
- [Haltech protocol PDFs](Vendors/Haltech)

Clone with submodules: git clone --recurse-submodules <repository-url>.
