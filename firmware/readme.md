# ESP32-CAN-X2 firmware (ESP-IDF 6.1)

This project is for the **Autosport Labs ESP32-CAN-X2** ESP32-S3 board. The separate ESP32-P4 runs the UI and display. No Arduino, LVGL, PlatformIO, or MCP2515 driver is needed for the initial CAN1 stage.

## Build and flash

Using an ESP-IDF 6.1 shell:

1. cd firmware
2. idf.py set-target esp32s3
3. idf.py build
4. idf.py -p YOUR_SERIAL_PORT flash monitor

Defaults are 8 MB flash and USB Serial/JTAG as the **primary input/output console**, so `gear-echo on` can be entered through the same USB serial monitor used to flash the board. For an existing build, a saved local `sdkconfig` overrides `sdkconfig.defaults`: check **Component config → ESP System Settings → Channel for console output → USB Serial/JTAG Controller** in `idf.py menuconfig`, alongside the flash size and CAN diagnostic source. Secondary USB output alone cannot receive console commands. Use 115200 baud serial monitoring. USB/programming connections and power depend on board revision. See the [vendor manual](https://wiki.autosportlabs.com/ESP32-CAN-X2).

For a hardware-free decoder check:

    g++ -std=c++17 -Wall -Wextra -Werror -I firmware/main firmware/main/haltech_protocol.cpp firmware/tests/test_protocol.cpp -o /tmp/test_protocol && /tmp/test_protocol

## Operational modes

- **Receive-only (default).** CAN1 runs in TWAI normal mode on GPIO6 RX and GPIO7 TX at **1 Mbit/s**. The controller ACKs valid CAN frames in hardware. Application frame transmission is disabled.
- **Receive plus GearEcho.** Serial control is enabled by default for physical CAN1 with UART or USB Serial/JTAG as the primary console. The firmware starts receive-only. Send `gear-echo on` followed by Enter to enable the echo and `gear-echo off` followed by Enter to stop new echoes. Reset also returns to receive-only. For a saved configuration that disabled serial control, enable **Allow serial control of the 0x650 GearEcho diagnostic** under **ESP32-CAN-X2 diagnostics** in `idf.py menuconfig` once.
- **Synthetic.** Select **Software-generated** under **CAN diagnostic source**. The monitor decodes predefined messages with the CAN controller disabled.
- **Raw logging.** An independent menuconfig option logs complete frame data. The monitor decodes three selected IDs from unfiltered CAN1 reception.

Changes in menuconfig take effect when the firmware is rebuilt and flashed. The serial commands switch between the two live modes without reflashing.

Synthetic mode is for application decoding and output behavior only; it does **not** validate CAN1 hardware, transceiver, wiring, or reception. Host decoder tests cover malformed frames and signed values. Flashing and actual CAN receipt must be checked on the user's physical board.

## Initial custom Haltech Generic CAN messages (based partly on Broadcast v2.0 layouts)

All multi-byte values use big-endian byte order; 11-bit standard IDs. **This is a custom Generic CAN stream on Haltech CAN Bus 2**, not the native ECU Broadcast Protocol. Specifically, custom `0x370` sends accelerator pedal position in place of the published broadcast protocol's vehicle speed; do not decode it using the standard `0x370` definition.

| CAN ID | Rate | Bytes | Channel |
|---|---|---|---|
| 0x3E4 | 5 Hz | 4, 5, 6 | Rotary Trim Pot 1, 2, 3: signed int8, raw |
| 0x370 | 20 Hz | 0–1 | Accelerator Pedal Position (APP), raw unsigned 16-bit value scaled to percent |
| 0x370 | 20 Hz | 2–3 | Combined Gear: signed int8 selector (byte 2) and signed int8 actual gear (byte 3) |
| 0x470 | 20 Hz | 6, 7 | Gear Selector Position and Gear: separate signed int8 enumerations |

Gear enumeration: 1–10 = forward gear; 0 = Neutral; -1 = Reverse; -2 = Park; -3 = Unknown; -4 = Drive; -5 = Sport; -6 = Manual; -7 = Low; -8 = Overdrive.

**Important:** selector position and actual gear are two different channels; never assert that they must equal each other. Compare the **selector in 0x370** to the **selector in 0x470**, and likewise compare the gear in 0x370 to gear in 0x470. The monitor flags only discrepancies persisting at least 300 ms, with both frames recently received, to avoid false alerts during shifting or differing frame arrival times.

The program logs trim and gear changes and reports changed **accelerator pedal position** at most once per second. It logs raw APP and the scaled percentage. Set **Accelerator pedal (APP) percent scale numerator** and **Accelerator pedal (APP) percent scale denominator** in menuconfig to match the Generic CAN APP field. Calculation: raw × numerator ÷ denominator; default 1 ÷ 10 (raw 345 → 34.5%). This assumes NSP transmits tenths of a percent; compare raw CAN bytes with the NSP engineering value to verify the actual divisor. The APP signal is pedal position, not throttle position.

Every five seconds the monitor reports CAN status, receive and transmit error counters, and the counts for successful completions, failed completions, rejected submissions, completion timeouts, and bus-off events. It also warns if a previously seen gear ID has been absent over 250 ms or rotary trims over 650 ms.

## GearEcho

On the Haltech Generic CAN receive channel, configure standard ID `0x650`, signed 8-bit at byte 0, with raw units and identity scaling. The ESP32 sends eight-byte data frames at up to 5 Hz. Byte 0 echoes the latest actual gear from byte 7 of a valid standard eight-byte `0x470` data frame. Bytes 1–7 are zero.

The transmitter skips unknown and out-of-range gear codes and uses the receive timestamp to enforce a 250 ms freshness limit with time reserved for TX completion. An echo gets one hardware attempt. A failed completion or rejected submission pauses echoing for one second. After bus-off or a TX completion timeout, correct the bus fault and restart the board. A restart discards any pending frame.

## Synthetic vectors

- 0x3E4: 00 00 00 00 03 FE 07 00 -> trims +3, -2, +7
- 0x370: 01 59 03 03 00 00 00 00 -> accelerator pedal 34.5% (assuming x10), selector 3, gear 3
- 0x470: 03 E8 03 E8 03 E8 03 03 -> selector 3, gear 3
- 0x470: 03 E8 03 E8 03 E8 00 00 -> Neutral, Neutral
- 0x470: 03 E8 03 E8 03 E8 FF FF -> Reverse, Reverse
- 0x370: 01 59 FC 03 00 00 00 00 and 0x470: 03 E8 03 E8 03 E8 FC 03 -> Drive (-4) selector / actual 3rd gear, deliberately different **but consistent across CAN IDs**.

## Stationary accelerator-pedal (APP) bench test

- Edit the first signal in NSP Generic CAN **Transmit Frame `0x370`**: select **Accelerator Pedal Position (APP)**, **not Throttle Position (TPS)**. Keep standard 11-bit ID, 8 bytes, 20 Hz, big-endian unsigned 16-bit at start bit 7 (bytes 0–1), offset 0; retain Combined Gear in bytes 2–3.
- Select units **percent (%)**. Use transmit divisor `0.10` if NSP applies engineering value ÷ divisor as expected; we have not verified the formula. Check raw received bytes against NSP APP reading before relying on the scaled percentage. Keep `0x3E4`, `0x470` and GearEcho `0x650` unchanged.
- Test only with the vehicle securely parked, parking brake engaged, and **engine off**. With ignition on, the DBW throttle may move as the pedal is pressed; keep clear of moving parts. Do not perform the test while driving.

## CAN1 wiring and safety

- Confirm **board revision** and pinout before making a cable: Autosport Labs documents different top connector arrangements across versions.
- Use CAN1-H, CAN1-L, and a suitable signal-ground reference. The top connector may also carry **vehicle power**; do not accidentally attach power pins to signal lines.
- Autosport Labs states its termination is **enabled by default**, via physical TERM pads; a CAN bus needs only two 120 Ω terminations, at its physical ends. Verify the existing vehicle terminations before tapping in. Never add a third 120 Ω resistor to a properly terminated bus.
- Connect Haltech CAN Bus 2 to the ESP32 board's CAN1. An active receiver must ACK the ESP32's GearEcho frames.
- Keep the Haltech disconnected while flashing and running synthetic software tests. Introduce the live ECU only after board programming and appropriate CAN wiring/termination are verified.

## Future stages

P4 communications and sensor acquisition are future stages. CAN2/MCP2515 is unused.

## Sources

- Vendors/Haltech/Broadcast_CAN_Protocol_Document_Version_2.pdf (protocol and nominal rates)
- https://wiki.autosportlabs.com/ESP32-CAN-X2
- https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32s3/api-reference/peripherals/twai.html
