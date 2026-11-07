#ifdef APP_TARGET_SIM
#include "platform_hal.h"
#include <lvgl.h>

bool platform_hal_init() {
    lv_sdl_window_create(400, 960);
    lv_sdl_mouse_create();
    return true;
}

void platform_hal_tick() {}
#endif
