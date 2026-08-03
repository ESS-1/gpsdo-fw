#ifndef _UI_064D40EB51E7_H_
#define _UI_064D40EB51E7_H_

#include "ui_core.h"

extern UIScreen ui_main_screen;

typedef enum {
    UI_Trend_HScale_Auto,
    UI_Trend_HScale_2min,
    UI_Trend_HScale_5min,
    UI_Trend_HScale_10min,
    UI_Trend_HScale_20min,
    UI_Trend_HScale_40min,
    UI_Trend_HScale_1h,
    UI_Trend_HScale_2h,

    UI_Trend_HScale_Max = UI_Trend_HScale_2h,
} UI_Trend_HScale;

typedef enum {
    UI_Trend_VScale_Auto,
    UI_Trend_VScale_2ppb,
    UI_Trend_VScale_5ppb,
    UI_Trend_VScale_10ppb,
    UI_Trend_VScale_20ppb,
    UI_Trend_VScale_50ppb,
    UI_Trend_VScale_100ppb,
    UI_Trend_VScale_200ppb,
    UI_Trend_VScale_500ppb,
    UI_Trend_VScale_1000ppb,

    UI_Trend_VScale_Max = UI_Trend_VScale_1000ppb,
} UI_Trend_VScale;

void ui_trend_init();
void ui_trend_run();

#endif
