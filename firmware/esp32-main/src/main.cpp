#ifdef APP_TARGET_ESP32

#include <Arduino.h>
#include <lvgl.h>
#include "app/app.h"
#include "hal/platform_hal.h"

void setup() {
    Serial.begin(115200);
    lv_init();
    platform_hal_init();
    app_init();
}

void loop() {
    lv_timer_handler();
    delay(5);
}

#endif
