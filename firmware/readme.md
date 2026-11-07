# ESP32-CAN-X2 firmware (ESP-IDF 6.1)

This project is for the **Autosport Labs ESP32-CAN-X2** ESP32-S3 board. The separate ESP32-P4 runs the UI and display. No Arduino, LVGL, PlatformIO, or MCP2515 driver is needed for the initial CAN1 stage.

## Build and flash

Using an ESP-IDF 6.1 shell:

1. cd firmware
2. idf.py set-target esp32s3
3. idf.py build
4. idf.py -p YOUR_SERIAL_PORT flash monitor

Serial logging defaults to 115200 baud. Set the serial port appropriate to your Mac. USB/programming connections and power depend on board revision; follow the [vendor manual](https://wiki.autosportlabs.com/ESP32-CAN-X2).

For a hardware-free decoder check:

    g++ -std=c++17 -Wall -Wextra -Werror -I firmware/main firmware/main/haltech_protocol.cpp firmware/tests/test_protocol.cpp -o /tmp/test_protocol && /tmp/test_protocol

## Operational modes

- **Default / passive CAN1:** receive-only, classical CAN at **1 Mbit/s**, GPIO6 RX and GPIO7 TX, never transmits or acknowledges.
- **Synthetic:** select with idf.py menuconfig > ESP32-CAN-X2 diagnostics > CAN diagnostic source > Software-generated. No CAN controller is initialized; predefined messages are fed through the normal decoder/reporting path. Rebuild/reflash after changing the option.
- **Raw logging:** separately opt in through menuconfig. Defaults off to avoid noisy console output. Physical CAN reception is unfiltered; app only decodes three selected IDs.

Synthetic mode is for application decoding and output behavior only; it does **not** validate CAN1 hardware, transceiver, wiring, or reception. Host decoder tests cover malformed frames and signed values. Flashing and actual CAN receipt must be checked on the user's physical board.

## Initial Haltech broadcast protocol (v2.0)

All multi-byte values use big-endian byte order; 11-bit standard IDs.

| CAN ID | Rate | Bytes | Channel |
|---|---|---|---|
| 0x3E4 | 5 Hz | 4, 5, 6 | Rotary Trim Pot 1, 2, 3: signed int8, raw |
| 0x370 | 20 Hz | 0–1 | Vehicle speed: unsigned int16 / 10 = km/h |
| 0x370 | 20 Hz | 2–3 | Combined Gear: signed int8 selector (byte 2) and signed int8 actual gear (byte 3) |
| 0x470 | 20 Hz | 6, 7 | Gear Selector Position and Gear: separate signed int8 enumerations |

Gear enumeration: 1–10 = forward gear; 0 = Neutral; -1 = Reverse; -2 = Park; -3 = Unknown; -4 = Drive; -5 = Sport; -6 = Manual; -7 = Low; -8 = Overdrive.

**Important:** selector position and actual gear are two different channels; never assert that they must equal each other. Compare the **selector in 0x370** to the **selector in 0x470**, and likewise compare the gear in 0x370 to gear in 0x470. The monitor flags only discrepancies persisting at least 300 ms, with both frames recently received, to avoid false alerts during shifting or differing frame arrival times.

The program prints trim and gear changes; vehicle speed only when changed and at most 1 Hz; receive counters and CAN status every 5 seconds. After a previously seen gear ID has been absent over 250 ms, it warns at summary time. Haltech's listed broadcast frequencies are nominal, not guaranteed.

## Synthetic vectors

- 0x3E4: 00 00 00 00 03 FE 07 00 -> trims +3, -2, +7
- 0x370: 01 59 03 03 00 00 00 00 -> speed 34.5 km/h, selector 3, gear 3
- 0x470: 03 E8 03 E8 03 E8 03 03 -> selector 3, gear 3
- 0x470: 03 E8 03 E8 03 E8 00 00 -> Neutral, Neutral
- 0x470: 03 E8 03 E8 03 E8 FF FF -> Reverse, Reverse
- 0x370: 01 59 FC 03 00 00 00 00 and 0x470: 03 E8 03 E8 03 E8 FC 03 -> Drive (-4) selector / actual 3rd gear, deliberately different **but consistent across CAN IDs**.

## CAN1 wiring and safety

- Confirm **board revision** and pinout before making a cable: Autosport Labs documents different top connector arrangements across versions.
- Use CAN1-H, CAN1-L, and a suitable signal-ground reference. The top connector may also carry **vehicle power**; do not accidentally attach power pins to signal lines.
- Autosport Labs states its termination is **enabled by default**, via physical TERM pads; a CAN bus needs only two 120 Ω terminations, at its physical ends. Verify the existing vehicle terminations before tapping in. Never add a third 120 Ω resistor to a properly terminated bus.
- A listen-only CAN interface will not send ACKs. An isolated ECU plus listen-only receiver might fail to deliver complete acknowledged frames; a bench transmitter needs another active acknowledging node.
- Keep the Haltech disconnected while flashing and running synthetic software tests. Introduce the live ECU only after board programming and appropriate CAN wiring/termination are verified.

## Future stages

CAN2: MCP2515, CS GPIO10, MOSI GPIO11, SCK GPIO12, MISO GPIO13, interrupt GPIO3, 16 MHz crystal (not yet driven). P4 communications, sensor acquisition, and Haltech GCAN *transmission* are deliberately not implemented.

## Sources

- Vendors/Haltech/Broadcast_CAN_Protocol_Document_Version_2.pdf (protocol and nominal rates)
- https://wiki.autosportlabs.com/ESP32-CAN-X2
- https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32s3/api-reference/peripherals/twai.html
