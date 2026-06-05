#include "ui.h"

#include "int.h"
#include "gps.h"
#include "cdcio.h"

#include "icons.h"
#include "fonts.h"
#include "st7735.h"

#include <stdio.h>


// Main UI screen
static void ui_proc_menu(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_gps(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_ppb(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_save(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_usb(const struct UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_datetime(const struct UIElement* element, UICommand command, int32_t encoder_step);

static const UIElement ui_main_screen_elements[] = {
    // Line 1
    { 1,   1,  15,  16, UI_STYLE_FOCUSABLE,                          ui_proc_menu     },
    { 17,  1,  38,  16, UI_STYLE_FOCUSABLE,                          ui_proc_gps      },
    { 56,  1,  71,  16, UI_STYLE_FOCUSABLE,                          ui_proc_ppb      },
    { 128, 1,  15,  16, UI_STYLE_FOCUSABLE,                          ui_proc_save     },
    { 144, 1,  16,  16, UI_STYLE_NONE,                               ui_proc_usb      },
    // Line 2
    { 19,  20, 140, 10, UI_STYLE_FOCUSABLE|UI_STYLE_INPUT_CAPTURING, ui_proc_datetime },
};

UIScreen ui_main_screen = {
    ui_main_screen_elements,
    sizeof(ui_main_screen_elements) / sizeof(UIElement),
    UI_FOCUSED_ELEMENT_IDX_NONE,
    false,
};


static void ui_proc_menu(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_DrawImage(element->x, element->y, 15, 16, icon_menu_15x16);
    }
    // TODO
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_gps(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        //TODO
        ST7735_DrawImage(element->x, element->y, 16, 16, gps_lock_status ? icon_sat_lock_16x16 : icon_sat_nolock_16x16);

        char s[10] = { '\0' };
        snprintf(s, 10, "%2d", num_sats);
        ST7735_WriteStringNoWrap(element->x+16, element->y+1, 15, s, Font_11x18, ST7735_WHITE, UI_BG_COLOR);
    }
    // TODO
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

static void ui_proc_save(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_DrawImage(element->x, element->y, 15, 16, icon_save_15x16);
    }
    // TODO
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_usb(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        //TODO
        switch (cdcio_status)
        {
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
    // TODO
    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_datetime(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y, element->height, "15 Jun 2026 02:41:36", Font_7x10, ST7735_WHITE, UI_BG_COLOR);
    }
    // TODO
    ui_default_element_proc(element, command, encoder_step);
}
