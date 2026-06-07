#include "ui.h"

#include "int.h"
#include "gps.h"
#include "cdcio.h"
#include "eeprom.h"
#include "frequency.h"

#include "icons.h"
#include "fonts.h"
#include "st7735.h"

#include <stdio.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


// Main UI screen
static void ui_proc_back_to_main_icon(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_back_to_main_ok(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_save(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_gps(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_ppb(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_usb(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_warmup(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_datetime(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_pwm(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_trend_h(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_trend_v(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_trend_graph(const struct UIElement* element, UICommand command, int32_t encoder_step);

static const UIElement ui_main_screen_elements[] = {
    // Line 1
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_menu },
    { 17,  1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_save },
    { 33,  1, 38, 16, UI_STYLE_FOCUSABLE, ui_proc_gps  },
    { 72,  1, 71, 16, UI_STYLE_FOCUSABLE, ui_proc_ppb  },
    { 144, 1, 16, 16, UI_STYLE_NONE,      ui_proc_usb  },
    // Line 2
    { 0,   20, 17,  18, UI_STYLE_NONE,                                 ui_proc_warmup      },
    { 19,  20, 140, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_datetime    },
    { 19,  34, 63,  10, UI_STYLE_FOCUSABLE,                            ui_proc_pwm         },
    { 89,  34, 28,  10, UI_STYLE_FOCUSABLE,                            ui_proc_trend_h     },
    { 124, 34, 35,  10, UI_STYLE_FOCUSABLE,                            ui_proc_trend_v     },
    // Trend
    { 0,   45, 160, 35, UI_STYLE_NONE,                                 ui_proc_trend_graph },
};

UIScreen ui_main_screen = {
    ui_main_screen_elements,
    sizeof(ui_main_screen_elements) / sizeof(UIElement),
    UI_FOCUSED_ELEMENT_IDX_NONE,
    false,
};

// Save screen
static void ui_save_proc_label(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_save_proc_yes(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_save_proc_no(const struct UIElement* element, UICommand command, int32_t encoder_step);

static const UIElement ui_save_screen_elements[] = {
    { 1,  1,  15,  16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main_icon },
    { 22, 4,  133, 34, UI_STYLE_NONE,      ui_save_proc_label        },
    { 43, 48, 39,  21, UI_STYLE_FOCUSABLE, ui_save_proc_yes          },
    { 95, 48, 38,  21, UI_STYLE_FOCUSABLE, ui_save_proc_no           },
};

UIScreen ui_save_screen = {
    ui_save_screen_elements,
    sizeof(ui_save_screen_elements) / sizeof(UIElement),
    3,
    false,
};

// Save failed screen
static void ui_save_error_proc_label(const struct UIElement* element, UICommand command, int32_t encoder_step);

static const UIElement ui_save_error_screen_elements[] = {
    { 1,  1,  15,  16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main_icon },
    { 22, 4,  133, 34, UI_STYLE_NONE,      ui_save_error_proc_label  },
    { 70, 48, 38,  21, UI_STYLE_FOCUSABLE, ui_proc_back_to_main_ok   },
};

UIScreen ui_save_error_screen = {
    ui_save_error_screen_elements,
    sizeof(ui_save_error_screen_elements) / sizeof(UIElement),
    2,
    false,
};


static void ui_proc_back_to_main_icon(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_DrawImage(element->x, element->y, 15, 16, icon_back_15x16);
    }
    if (command & UICommand_Click) {
        ui_show_screen(&ui_main_screen);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_back_to_main_ok(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_FillRectangleFast(element->x, element->y, element->width, element->height, UI_BUTTON_BG_COLOR);
        ST7735_WriteStringNoWrap(element->x + 8, element->y + 2, 18, "OK", Font_11x18, ST7735_WHITE, UI_BUTTON_BG_COLOR);
    }
    if (command & UICommand_Click) {
        ui_show_screen(&ui_main_screen);
    }
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_DrawImage(element->x, element->y, 15, 16, icon_menu_15x16);
    }
    // TODO
    ui_default_element_proc(element, command, encoder_step);
}

static bool ui_cache_ee_is_changed = false;
static void ui_proc_save(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if ((command & UICommand_Init) || (ee_is_changed != ui_cache_ee_is_changed)) {
        ui_cache_ee_is_changed = ee_is_changed;
        ST7735_DrawImage(element->x, element->y, 15, 16, ui_cache_ee_is_changed ? icon_save_15x16 : icon_save_inactive_15x16);
    }

    if ((command & UICommand_Click) && ui_cache_ee_is_changed) {
        ui_show_screen(&ui_save_screen);
    }
    ui_default_element_proc(element, command, encoder_step);
}

static bool    ui_cache_gps_lock_status = false;
static uint8_t ui_cache_num_sats = 0;
static void ui_proc_gps(const struct UIElement* element, UICommand command, int32_t encoder_step)
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
        snprintf(s, ARRAY_SIZE(s), "%2d", ui_cache_num_sats > 99 ? 99 : ui_cache_num_sats);
        ST7735_WriteStringNoWrap(element->x + 16, element->y + 1, 15, s, Font_11x18, ST7735_WHITE, UI_BG_COLOR);
    }

    if (command & UICommand_Click) {
        // todo
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_ppb(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        //TODO
        ST7735_DrawImage(element->x, element->y, 16, 16, icon_ppb_avg_16x16);

        char s[10] = { '\0' };
        snprintf(s, 10, "%5d", -1230);
        ST7735_WriteStringNoWrap(element->x + 16, element->y + 1, 15, s, Font_11x18, ST7735_WHITE, UI_BG_COLOR);
    }
    // TODO
    ui_default_element_proc(element, command, encoder_step);
}

static cdc_status ui_cache_cdcio_status = CDC_STATUS_NO_CONN;
static void ui_proc_usb(const struct UIElement* element, UICommand command, int32_t encoder_step)
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
static void ui_proc_warmup(const struct UIElement* element, UICommand command, int32_t encoder_step)
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
            ST7735_DrawImage(element->x + 1, element->y, 15, 10, icon_warmup_15x10);
        } else {
            ST7735_FillRectangleFast(element->x + 1, element->y, 15, 10, UI_BG_COLOR);
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
                ST7735_DrawImage(element->x, element->y + 11, 5, 7, icon_digits_0_9_5x7[digit1]);
            } else {
                ST7735_FillRectangle(element->x, element->y + 11, 5, 7, UI_BG_COLOR);
            }
            if (digit1 > 0 || digit2 > 0) {
                ST7735_DrawImage(element->x + 5 + 1, element->y + 11, 5, 7, icon_digits_0_9_5x7[digit2]);
            } else {
                ST7735_FillRectangle(element->x + 5 + 1, element->y + 11, 5, 7, UI_BG_COLOR);
            }
            ST7735_DrawImage(element->x + 2 * (5 + 1), element->y + 11, 5, 7, icon_digits_0_9_5x7[digit3]);
        } else {
            ST7735_FillRectangleFast(element->x, element->y + 11, 5 + 1 + 5 + 1 + 5, 7, UI_BG_COLOR);
        }
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_datetime(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        //todo
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height, "15 Apr 2026", Font_7x10, ST7735_WHITE, UI_BG_COLOR);
        ST7735_WriteStringNoWrap(element->x + 12 * 7, element->y + 1, element->height, "02:41:36", Font_7x10, ST7735_WHITE, UI_BG_COLOR);
    }
    // TODO
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_pwm(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        //todo
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "PWM:", Font_7x10, ST7735_WHITE, UI_BG_COLOR);
        ST7735_WriteStringNoWrap(element->x + 4 * 7, element->y + 1, element->height - 1, "32768", Font_7x10, ST7735_WHITE, UI_BG_COLOR);
    }
    // TODO
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_trend_h(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        // todo
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "H:", Font_7x10, ST7735_WHITE, UI_BG_COLOR);
        ST7735_WriteStringNoWrap(element->x + 2 * 7, element->y + 1, element->height - 1, "10", Font_7x10, ST7735_WHITE, UI_BG_COLOR);
    }
    // TODO
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_trend_v(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        // todo
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "V:", Font_7x10, ST7735_WHITE, UI_BG_COLOR);
        ST7735_WriteStringNoWrap(element->x + 2 * 7, element->y + 1, element->height - 1, "120", Font_7x10, ST7735_WHITE, UI_BG_COLOR);
    }
    // TODO
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_trend_graph(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        // todo
        ST7735_FillRectangleFast(element->x, element->y, element->width, 1, ST7735_COLOR565(150, 175, 210));
    }
    // TODO
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_save_proc_label(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x,    element->y,    10, "Save current device", Font_7x10, ST7735_WHITE, UI_BG_COLOR);
        ST7735_WriteStringNoWrap(element->x+10, element->y+12, 10, "configuration to",    Font_7x10, ST7735_WHITE, UI_BG_COLOR);
        ST7735_WriteStringNoWrap(element->x+42, element->y+24, 10, "EEPROM?",             Font_7x10, ST7735_WHITE, UI_BG_COLOR);
    }
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_save_proc_yes(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_FillRectangleFast(element->x, element->y, element->width, element->height, UI_BUTTON_BG_COLOR);
        ST7735_WriteStringNoWrap(element->x + 3, element->y + 2, 18, "Yes", Font_11x18, ST7735_WHITE, UI_BUTTON_BG_COLOR);
    }
    if (command & UICommand_Click) {
        // Save configuration
        if (ee_save_config()) {
            // EEPROM write succeeded
            ui_show_screen(&ui_main_screen);
        } else {
            // EEPROM write failed
            ui_show_screen(&ui_save_error_screen);
        }
    }
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_save_proc_no(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_FillRectangleFast(element->x, element->y, element->width, element->height, UI_BUTTON_BG_COLOR);
        ST7735_WriteStringNoWrap(element->x + 8, element->y + 2, 18, "No", Font_11x18, ST7735_WHITE, UI_BUTTON_BG_COLOR);
    }
    if (command & UICommand_Click) {
        ui_show_screen(&ui_main_screen);
    }
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_save_error_proc_label(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x+28, element->y,    10, "Cannot save     ",    Font_7x10, ST7735_RED, UI_BG_COLOR);
        ST7735_WriteStringNoWrap(element->x+18, element->y+12, 10, "configuration:",      Font_7x10, ST7735_RED, UI_BG_COLOR);
        ST7735_WriteStringNoWrap(element->x,    element->y+24, 10, "EEPROM write failed", Font_7x10, ST7735_RED, UI_BG_COLOR);
    }
    ui_default_element_proc(element, command, encoder_step);
}
