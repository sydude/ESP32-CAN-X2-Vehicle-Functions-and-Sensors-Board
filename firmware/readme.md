# Firmware

ESP32-S3 firmware for display controller and vehicle interface.

Planned functions:
- Display UI rendering
- Modbus polling
- GPS parsing
- CAN monitoring (future)

Hardware coupling:
- RS-485 transceiver on UART1
- GPS on UART2
- SPI display interface
- I2C touch controller

Build environment:
- ESP-IDF (planned)