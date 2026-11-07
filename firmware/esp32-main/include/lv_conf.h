#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH 16
#define LV_MEM_SIZE (256U * 1024U)
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

#define LV_USE_LABEL 1
#define LV_USE_BUTTON 1
#define LV_FONT_MONTSERRAT_16 1

#ifdef APP_TARGET_SIM
    #define LV_USE_SDL 1
#else
    #define LV_USE_SDL 0
#endif

#endif
