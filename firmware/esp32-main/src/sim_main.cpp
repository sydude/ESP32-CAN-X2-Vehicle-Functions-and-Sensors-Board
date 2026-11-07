#ifdef APP_TARGET_SIM

#include <lvgl.h>
#include "app/app.h"
#include "hal/platform_hal.h"
#include <unistd.h>

int main() {
    lv_init();
    platform_hal_init();
    app_init();

    while (true) {
        lv_timer_handler();
        usleep(5000);
    }
    return 0;
}
#endif
