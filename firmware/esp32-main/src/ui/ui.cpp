#include "ui.h"
#include <lvgl.h>

void ui_build() {
    lv_obj_t* screen = lv_screen_active();

    lv_obj_t* label = lv_label_create(screen);
    lv_label_set_text(label, "LVGL Skeleton Running");
    lv_obj_center(label);
}
