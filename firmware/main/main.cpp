#include "board_pins.h"
#include "can_capture.h"
#include "monitor.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#if CONFIG_CANX2_GEAR_ECHO
#include <string_view>
#include <unistd.h>
#endif

namespace {
constexpr char kTag[] = "can-x2";

#if CONFIG_CANX2_GEAR_ECHO
void poll_serial(bool& gear_echo_enabled) {
    static char line[16];
    static size_t length = 0;
    char input[32];
    // The supported primary consoles use nonblocking reads by default.
    const ssize_t count = read(STDIN_FILENO, input, sizeof(input));
    for (ssize_t i = 0; i < count; ++i) {
        const char byte = input[i];
        if (byte != '\r' && byte != '\n') {
            if (length < sizeof(line)) line[length++] = byte;
            continue;
        }

        const std::string_view command(line, length);
        length = 0;
        if (command == "gear-echo on" || command == "gear-echo off") {
            gear_echo_enabled = command == "gear-echo on";
            ESP_LOGI(kTag, "GearEcho %s", gear_echo_enabled ? "enabled" : "disabled");
        }
    }
}
#endif

#if CONFIG_CANX2_MODE_SYNTHETIC
void inject(uint32_t id, const uint8_t (&bytes)[8]) {
    haltech::Frame frame = {};
    frame.id = id;
    frame.len = 8;
    frame.synthetic = true;
    frame.timestamp_us = esp_timer_get_time();
    for (unsigned i = 0; i < 8; ++i) frame.data[i] = bytes[i];
    monitor::ingest(frame);
}

void run_synthetic() {
    ESP_LOGW(kTag, "SYNTHETIC TEST MODE: hardware CAN1 is NOT initialized");
    const uint8_t trims[8]   = {0, 0, 0, 0, 3, 0xFE, 7, 0};
    const uint8_t combined_gear[8] = {0, 0, 3, 3, 0, 0, 0, 0};
    const uint8_t pedal[8] = {0, 0, 0x01, 0x59, 0, 0, 0, 0}; // APP 34.5% at x10
    const uint8_t gear3[8]   = {0x03, 0xE8, 0x03, 0xE8, 0x03, 0xE8, 3, 3};
    const uint8_t neutral[8] = {0x03, 0xE8, 0x03, 0xE8, 0x03, 0xE8, 0, 0};
    const uint8_t reverse[8] = {0x03, 0xE8, 0x03, 0xE8, 0x03, 0xE8, 0xFF, 0xFF};
    const uint8_t selector_drive[8] = {0, 0, 0xFC, 3, 0, 0, 0, 0}; // selector Drive, actual 3
    const uint8_t gear_selector_drive[8] = {0x03, 0xE8, 0x03, 0xE8, 0x03, 0xE8, 0xFC, 3};

    inject(0x3E4, trims);
    inject(0x370, combined_gear);
    inject(0x471, pedal);
    inject(0x470, gear3);
    vTaskDelay(pdMS_TO_TICKS(350));
    inject(0x470, neutral);
    vTaskDelay(pdMS_TO_TICKS(350));
    inject(0x470, reverse);
    vTaskDelay(pdMS_TO_TICKS(350));
    // Different selector and actual gear is VALID if both representations agree.
    inject(0x370, selector_drive);
    inject(0x470, gear_selector_drive);
    ESP_LOGI(kTag, "Synthetic vectors dispatched; compare values above with firmware README");
}
#endif
} // namespace

extern "C" void app_main(void) {
    bool gear_echo_enabled = false;
    ESP_LOGI(kTag, "ESP32-CAN-X2 vehicle firmware | IDF %s", esp_get_idf_version());
    ESP_LOGI(kTag, "CAN1 bitrate=%d, RX=%d TX=%d | CAN2 unused",
        board::can1_bitrate, board::can1_rx, board::can1_tx);
#if CONFIG_CANX2_MODE_SYNTHETIC
    run_synthetic();
#else
    ESP_LOGI(kTag, "CAN1 normal mode, hardware ACK enabled | GearEcho off");
#if CONFIG_CANX2_GEAR_ECHO
    ESP_LOGI(kTag, "Serial commands: gear-echo on / gear-echo off");
#endif
    ESP_ERROR_CHECK(can_capture::start());
#endif
    while (true) {
#if CONFIG_CANX2_MODE_LIVE
        haltech::Frame frame = {};
        constexpr uint32_t kReceiveWaitMs =
#if CONFIG_CANX2_GEAR_ECHO
            10;
#else
            100;
#endif
        if (can_capture::receive(frame, kReceiveWaitMs)) monitor::ingest(frame);
#if CONFIG_CANX2_GEAR_ECHO
        poll_serial(gear_echo_enabled);
#endif
#else
        vTaskDelay(pdMS_TO_TICKS(100));
#endif
        monitor::poll(gear_echo_enabled);
    }
}
