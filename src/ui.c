#include "ui.h"
#include "ui_helpers.h"
#include "ui_msgbox.h"

#include "int.h"
#include "gps.h"
#include "cdcio.h"
#include "eeprom.h"
#include "frequency.h"
#include "pll.h"
#include "pll_presets.h"

#include "icons.h"
#include "fonts.h"
#include "st7735.h"

#include <stdio.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


static void ui_proc_back_to_main(const UIElement* element, UICommand command, int32_t encoder_step);

// Main UI screen
static void ui_proc_menu(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_save(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_gps(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_ppb(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_usb(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_warmup(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_datetime(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_out1(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_out2(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_pwm(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_trend_h(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_trend_v(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_trend_graph(const UIElement* element, UICommand command, int32_t encoder_step);

static const UIElement ui_main_screen_elements[] = {
    // Line 1
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_menu },
    { 17,  1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_save },
    { 33,  1, 38, 16, UI_STYLE_FOCUSABLE, ui_proc_gps  },
    { 72,  1, 71, 16, UI_STYLE_FOCUSABLE, ui_proc_ppb  },
    { 144, 1, 16, 16, UI_STYLE_NONE,      ui_proc_usb  },
    // Line 2
    { 0,   21, 17,  20, UI_STYLE_NONE,                                 ui_proc_warmup      },
    { 19,  19, 140, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_datetime    },
    { 19,  32, 49,  10, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_out1        },
    { 82,  32, 77,  10, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_out2        },
    { 19,  44, 63,  10, UI_STYLE_FOCUSABLE,                            ui_proc_pwm         },
    { 89,  44, 28,  10, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_trend_h     },
    { 124, 44, 35,  10, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_trend_v     },
    // Trend
    { 0,   55, 160, 25, UI_STYLE_NONE,                                 ui_proc_trend_graph },
};

UIScreen ui_main_screen = {
    ui_main_screen_elements,
    ARRAY_SIZE(ui_main_screen_elements),
    NULL,
    false,
};

// Main menu screen
//TODO: procs

static const UIElement ui_menu_screen_elements[] = {
    { 1, 1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main },
//TODO
};

static UIScreen ui_menu_screen = {
    ui_menu_screen_elements,
    ARRAY_SIZE(ui_menu_screen_elements),
    NULL,
    false,
};


// UI element procedures
static void ui_proc_back_to_main(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_DrawImage(element->x, element->y, 15, 16, icon_back_15x16);
    }
    if (command & UICommand_Click) {
        ui_show_screen(&ui_main_screen);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_DrawImage(element->x, element->y, 15, 16, icon_menu_15x16);
    }

    if (command & UICommand_Click) {
        ui_show_screen(&ui_menu_screen);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static bool ui_cache_ee_is_changed = false;
static void ui_proc_save_handler(UI_MsgBoxButton result);
static void ui_proc_save(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if ((command & UICommand_Init) || (ee_is_changed != ui_cache_ee_is_changed)) {
        ui_cache_ee_is_changed = ee_is_changed;
        ST7735_DrawImage(element->x, element->y, 15, 16, ui_cache_ee_is_changed ? icon_save_15x16 : icon_save_inactive_15x16);
    }

    if ((command & UICommand_Click) && ui_cache_ee_is_changed) {
        const char* msg[] = {
            "Save current device",
            " configuration to",
            "      EEPROM?",
            NULL };
        ui_msgbox(msg, UI_MsgBoxType_YesNo, UI_MsgBoxButton_No, ui_proc_save_handler);
    }
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_save_handler(UI_MsgBoxButton result)
{
    if (result == UI_MsgBoxButton_Yes) {
        // Save configuration
        if (!ee_save_config()) {
            // EEPROM write failed
            const char* msg[] = {
                "    Cannot save",
                "   configuration:",
                "EEPROM write failed",
                NULL };
            ui_msgbox(msg, UI_MsgBoxType_Error, UI_MsgBoxButton_Ok, NULL);
        }
    }
}

static bool    ui_cache_gps_lock_status = false;
static uint8_t ui_cache_num_sats = 0;
static void ui_proc_gps(const UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw icon
    if ((command & UICommand_Init) || (gps_lock_status != ui_cache_gps_lock_status)) {
        ui_cache_gps_lock_status = gps_lock_status;
        ST7735_DrawImage(element->x, element->y, 16, 16, ui_cache_gps_lock_status ? icon_sat_lock_16x16 : icon_sat_nolock_16x16);
    }

    // Draw number of satellites
    if ((command & UICommand_Init) || (num_sats != ui_cache_num_sats)) {
        ui_cache_num_sats = num_sats;

        char s[3] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%2u", ui_cache_num_sats > 99 ? 99 : ui_cache_num_sats);
        ST7735_WriteStringNoWrap(element->x + 16, element->y + 1, 15, s, Font_11x18, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    if (command & UICommand_Click) {
        // todo
    }

    ui_default_element_proc(element, command, encoder_step);
}

static FrequencyStability ui_cache_frequency_stability = FREQ_STABILITY_UNSTABLE;
static int32_t            ui_cache_frequency_ppb_x100  = PPB_UNSET_VALUE;
static void ui_proc_ppb(const UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw icon
    if ((command & UICommand_Init) || (frequency_stability != ui_cache_frequency_stability)) {
        ui_cache_frequency_stability = frequency_stability;

        switch (ui_cache_frequency_stability) {
        case FREQ_STABILITY_STABLE:
            ST7735_DrawImage(element->x, element->y, 16, 16, icon_ppb_good_16x16);
            break;
        case FREQ_STABILITY_MARGINAL:
            ST7735_DrawImage(element->x, element->y, 16, 16, icon_ppb_avg_16x16);
            break;
        case FREQ_STABILITY_UNSTABLE:
        default:
            ST7735_DrawImage(element->x, element->y, 16, 16, icon_ppb_bad_16x16);
            break;
        }
    }

    // Draw value
    if ((command & UICommand_Init) || (frequency_ppb_x100 != ui_cache_frequency_ppb_x100)) {
        ui_cache_frequency_ppb_x100 = frequency_ppb_x100;

        char s[6] = { '\0' };
        ui_format_ppb_5char(ui_cache_frequency_ppb_x100, s, ARRAY_SIZE(s));
        ST7735_WriteStringNoWrap(element->x + 16, element->y + 1, 15, s, Font_11x18, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    if (command & UICommand_Click) {
        //TODO
    }

    ui_default_element_proc(element, command, encoder_step);
}

static cdc_status ui_cache_cdcio_status = CDC_STATUS_NO_CONN;
static void ui_proc_usb(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if ((command & UICommand_Init) || (cdcio_status != ui_cache_cdcio_status)) {
        ui_cache_cdcio_status = cdcio_status;

        switch (ui_cache_cdcio_status) {
        case CDC_STATUS_OK:
            ST7735_DrawImage(element->x, element->y, 16, 16, icon_usb_ok_16x16);
            break;
        case CDC_STATUS_OVERFLOW:
            ST7735_DrawImage(element->x, element->y, 16, 16, icon_usb_warn_16x16);
            break;
        case CDC_STATUS_NO_CONN:
        default:
            ST7735_DrawImage(element->x, element->y, 16, 16, icon_usb_err_16x16);
            break;
        }
    }

    ui_default_element_proc(element, command, encoder_step);
}

static uint32_t ui_cache_warmup_remaining_sec = UINT16_MAX;
static void ui_proc_warmup(const UIElement* element, UICommand command, int32_t encoder_step)
{
    bool draw_icon = false;
    bool draw_timer = false;
    if (command & UICommand_Init) {
        draw_icon  = ui_cache_warmup_remaining_sec > 0;
        draw_timer = true;
    }

    if (ui_cache_warmup_remaining_sec > 0) {
        if (frequency_adjustment_allowed()) {
            ui_cache_warmup_remaining_sec = 0;
            draw_icon  = true;
            draw_timer = true;
        } else {
            uint32_t now_sec = (HAL_GetTick() / 1000);
            uint32_t remaining = ee_storage.warmup_time_seconds > now_sec
                ? ee_storage.warmup_time_seconds - now_sec
                : ui_cache_warmup_remaining_sec;
            draw_timer |= (remaining != ui_cache_warmup_remaining_sec);
            ui_cache_warmup_remaining_sec = remaining;
        }
    }

    // Draw icon
    if (draw_icon) {
        if (ui_cache_warmup_remaining_sec > 0) {
            ST7735_DrawImage(element->x + 1, element->y, 15, 12, icon_warmup_15x12);
        } else {
            ST7735_FillRectangleFast(element->x + 1, element->y, 15, 12, UI_COLOR_BG);
        }
    }

    // Draw countdown timer
    if (draw_timer) {
        if (ui_cache_warmup_remaining_sec > 0) {
            int16_t timer = ui_cache_warmup_remaining_sec;
            if (timer > 999) {
                timer = 999;
            }

            uint8_t digit1 = timer / 100;
            uint8_t digit2 = (timer / 10) % 10;
            uint8_t digit3 = timer % 10;

            // Draw digits
            if (digit1 > 0) {
                ST7735_DrawImage(element->x, element->y + 13, 5, 7, icon_digits_0_9_5x7[digit1]);
            } else {
                ST7735_FillRectangle(element->x, element->y + 13, 5, 7, UI_COLOR_BG);
            }
            if (digit1 > 0 || digit2 > 0) {
                ST7735_DrawImage(element->x + 5 + 1, element->y + 13, 5, 7, icon_digits_0_9_5x7[digit2]);
            } else {
                ST7735_FillRectangle(element->x + 5 + 1, element->y + 13, 5, 7, UI_COLOR_BG);
            }
            ST7735_DrawImage(element->x + 2 * (5 + 1), element->y + 13, 5, 7, icon_digits_0_9_5x7[digit3]);
        } else {
            ST7735_FillRectangleFast(element->x, element->y + 13, 5 + 1 + 5 + 1 + 5, 7, UI_COLOR_BG);
        }
    }

    ui_default_element_proc(element, command, encoder_step);
}

static PackedDate ui_cache_gps_date = { .raw = GPS_EMPTY_DATE_TIME };
static PackedTime ui_cache_gps_time = { .raw = GPS_EMPTY_DATE_TIME };
static void ui_proc_datetime(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (!ui_is_captured(element)) {
        // Draw date
        if ((command & (UICommand_Init | UICommand_Release)) || (gps_date.raw != ui_cache_gps_date.raw)) {
            ui_cache_gps_date = gps_date;

            const char* date_str = NULL;
            char buf_str[12] = { '\0' };

            if (ui_cache_gps_date.raw != GPS_EMPTY_DATE_TIME) {
                snprintf(buf_str, ARRAY_SIZE(buf_str), "%2u %.3s %04u",
                                                       (ui_cache_gps_date.day > 99u) ? 99u : ui_cache_gps_date.day,
                                                       ui_get_month_name_3char(ui_cache_gps_date.month),
                                                       (ui_cache_gps_date.year > 9999u) ? 9999u : ui_cache_gps_date.year);
                date_str = buf_str;
            } else {
                date_str = "           ";
            }

            ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height, date_str, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
        }

        // Draw time
        if ((command & (UICommand_Init | UICommand_Release)) || (gps_time.raw != ui_cache_gps_time.raw)) {
            ui_cache_gps_time = gps_time;

            const char* time_str = NULL;
            char buf_str[9] = { '\0' };

            if (ui_cache_gps_time.raw != GPS_EMPTY_DATE_TIME) {
                snprintf(buf_str, ARRAY_SIZE(buf_str), "%02u:%02u:%02u",
                                                       (ui_cache_gps_time.hours > 99u) ? 99u : ui_cache_gps_time.hours,
                                                       (ui_cache_gps_time.minutes > 99u) ? 99u : ui_cache_gps_time.minutes,
                                                       (ui_cache_gps_time.seconds > 99u) ? 99u : ui_cache_gps_time.seconds);
                time_str = buf_str;
            } else {
                time_str = "--:--:--";
            }

            ST7735_WriteStringNoWrap(element->x + 12 * 7, element->y + 1, element->height, time_str, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
        }
    }

    // Time offset edit
    {
        if (command & UICommand_Capture) {
            // Draw label and erase the remaining element area
            ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height, "Time offset: ", Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
            ST7735_FillRectangleFast(element->x + 13 * 7, element->y + 1, 7*7, element->height, UI_COLOR_BG);
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            ui_change_setting_i8(&gps_time_offset, encoder_step, GPS_MIN_TIME_OFFSET, GPS_MAX_TIME_OFFSET);
        }

        if (command & (UICommand_Capture | UICommand_EncoderStep)) {
            // Draw value
            char s[5] = { '\0' };
            snprintf(s, ARRAY_SIZE(s), "%3d", gps_time_offset);
            ST7735_WriteStringNoWrap(element->x + 13 * 7, element->y + 1, element->height, s, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
        }

        if (command & UICommand_Release) {
            // Save changes
            uint32_t new_time_offset = (uint32_t)(gps_time_offset - GPS_MIN_TIME_OFFSET);
            if (ee_storage.gps_time_offset != new_time_offset) {
                ee_storage.gps_time_offset = new_time_offset;
                ee_is_changed = true;
            }
        }
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_out(const UIElement* element, UICommand command, int32_t encoder_step, uint8_t out)
{
    if (!ui_is_captured(element)) {
        if (command & (UICommand_Init | UICommand_Release)) {
            // Draw icon and label
            ST7735_DrawImage(element->x, element->y + 2, 7, 7, icon_out_7x7);
            char s[3] = { '\0' };
            snprintf(s, ARRAY_SIZE(s), "%1u:", out);
            ST7735_WriteStringNoWrap(element->x + 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);

            // Draw value
            OutFreqConfig preset = (out == 1)
                ? pll_out1_presets[ee_storage.pll_out1_preset]
                : pll_out2_presets[ee_storage.pll_out2_preset];

            ST7735_WriteStringNoWrap(element->x + 3 * 7, element->y + 1, element->height - 1, preset.label, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
        }
    }

    // Preset selection
    {
        if (command & UICommand_Capture) {
            // todo
        }

        if (command & UICommand_EncoderStep) {
            // todo
        }

        if (command & (UICommand_Capture | UICommand_EncoderStep)) {
            // todo
        }

        if (command & UICommand_Release) {
            // todo
        }
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_out1(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_out(element, command, encoder_step, 1);
}

static void ui_proc_out2(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_out(element, command, encoder_step, 2);
}

static uint16_t ui_cache_pwm = 0;
static void ui_proc_pwm(const UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "PWM:", Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    // Draw value
    uint16_t pwm = TIM1->CCR2;
    if ((command & UICommand_Init) || (pwm != ui_cache_pwm)) {
        ui_cache_pwm = pwm;

        char s[6] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%5u", ui_cache_pwm);
        ST7735_WriteStringNoWrap(element->x + 4 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    if (command & UICommand_Click) {
        if (ee_storage.pwm != pwm) {
            ee_storage.pwm = pwm;
            ee_is_changed = true;
        }

        const char* msg[] = {
            " The PWM value has",
            "     been set.",
            NULL };
        ui_msgbox(msg, UI_MsgBoxType_Ok, UI_MsgBoxButton_Ok, NULL);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_trend_h(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (!ui_is_captured(element)) {
        if (command & (UICommand_Init | UICommand_Release)) {
            // todo
            ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "H:", Font_7x10, UI_COLOR_TREND, UI_COLOR_BG);
            ST7735_WriteStringNoWrap(element->x + 2 * 7, element->y + 1, element->height - 1, "10", Font_7x10, UI_COLOR_TREND, UI_COLOR_BG);
        }
    }

    if (command & UICommand_Capture) {
        // todo
    }
    if (command & UICommand_EncoderStep) {
        // todo
    }
    if (command & UICommand_Release) {
        // todo
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_trend_v(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (!ui_is_captured(element)) {
        if (command & (UICommand_Init | UICommand_Release)) {
            // todo
            ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "V:", Font_7x10, UI_COLOR_TREND, UI_COLOR_BG);
            ST7735_WriteStringNoWrap(element->x + 2 * 7, element->y + 1, element->height - 1, "120", Font_7x10, UI_COLOR_TREND, UI_COLOR_BG);
        }
    }

    if (command & UICommand_Capture) {
        // todo
    }
    if (command & UICommand_EncoderStep) {
        // todo
    }
    if (command & UICommand_Release) {
        // todo
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_trend_graph(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        // todo
        ST7735_FillRectangleFast(element->x, element->y, element->width, element->height, ST7735_COLOR565(30, 30, 30));
    }
    // TODO: draw
    ui_default_element_proc(element, command, encoder_step);
}
