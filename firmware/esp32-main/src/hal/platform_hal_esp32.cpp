#ifdef APP_TARGET_ESP32
#include "platform_hal.h"

bool platform_hal_init() { return true; }
void platform_hal_tick() {}
#endif
