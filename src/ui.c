#include "ui.h"
#include "ui_helpers.h"
#include "ui_msgbox.h"

#include "version.h"
#include "int.h"
#include "gps.h"
#include "cdcio.h"
#include "eeprom.h"
#include "frequency.h"
#include "pll.h"
#include "pll_presets.h"
#include "timer.h"

#include "icons.h"
#include "fonts.h"
#include "st7735.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>


static void ui_proc_back_to_main(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_label(const UIElement* element, UICommand command, int32_t encoder_step, const char* label);
static void ui_proc_menu_label_page_1of2(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_label_page_2of2(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_label_page_1of3(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_label_page_2of3(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_label_page_3of3(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_readonly_entry(const UIElement* element, UICommand command, int32_t encoder_step, const char* label, int32_t value_offset, const char* value);
static void ui_proc_menu_page_switch(const UIElement* element, UICommand command, int32_t encoder_step, const uint16_t* icon_10x15, UIScreen* page);
static void ui_proc_menu_page_left(const UIElement* element, UICommand command, int32_t encoder_step, UIScreen* left_page);
static void ui_proc_menu_page_right(const UIElement* element, UICommand command, int32_t encoder_step, UIScreen* right_page);
static void ui_proc_menu_page_left_inactive(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_page_right_inactive(const UIElement* element, UICommand command, int32_t encoder_step);


//------------------------------------------------------------------------------
// Main UI Screen Layout
//------------------------------------------------------------------------------
static void ui_proc_menu(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_save(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_gps(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_ppb(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_usb(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_datetime(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_pps(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_status(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_out1(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_out2(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_pwm(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_trend_h(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_trend_separator(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_trend_v(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_trend_graph(const UIElement* element, UICommand command, int32_t encoder_step);

static const UIElement ui_main_screen_elements[] = {
    // Top line
    { 1,   1, 14, 16, UI_STYLE_FOCUSABLE, ui_proc_menu },
    { 16,  1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_save },
    { 32,  1, 38, 16, UI_STYLE_FOCUSABLE, ui_proc_gps  },
    { 71,  1, 71, 16, UI_STYLE_FOCUSABLE, ui_proc_ppb  },
    { 143, 1, 16, 16, UI_STYLE_FOCUSABLE, ui_proc_usb  },
    // Main part
    { 1,   19, 140, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_datetime        },
    { 142, 20, 18,  9,  UI_STYLE_NONE,                                 ui_proc_pps             },
    { 1,   33, 17,  12, UI_STYLE_FOCUSABLE,                            ui_proc_status          },
//  { 1,   46, 17,  7 } - empty space reserved for the warm-up countdown timer drawn by 'ui_proc_status'
    { 23,  32, 49,  10, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_out1            },
    { 82,  32, 77,  10, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_out2            },
    { 23,  44, 63,  10, UI_STYLE_FOCUSABLE,                            ui_proc_pwm             },
    // Trend
    { 91,  44, 28,  10, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_trend_h         },
    { 120, 44, 3,   10, UI_STYLE_NONE,                                 ui_proc_trend_separator },
    { 124, 44, 35,  10, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_trend_v         },
    { 0,   55, 160, 25, UI_STYLE_NONE,                                 ui_proc_trend_graph     },
};

UIScreen ui_main_screen = {
    ui_main_screen_elements,
    NULL,
    ARRAY_SIZE(ui_main_screen_elements),
    false,
};


//------------------------------------------------------------------------------
// Main Menu Screen Layout
//------------------------------------------------------------------------------
// Header
static void ui_proc_menu_main_label(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_main_to_page1(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_main_to_page2(const UIElement* element, UICommand command, int32_t encoder_step);
// Page 1
static void ui_proc_menu_main_brightness(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_main_uptime(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_main_model(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_main_sn(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_main_version(const UIElement* element, UICommand command, int32_t encoder_step);
// Page 2
static void ui_proc_menu_main_mcu_flash(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_main_eeprom_writes(const UIElement* element, UICommand command, int32_t encoder_step);

// Page 1
static const UIElement ui_menu_screen_elements_page1[] = {
    // Header
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main            },
    { 27,  6, 28, 10, UI_STYLE_NONE,      ui_proc_menu_main_label         },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_page_left_inactive },
    { 127, 6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_label_page_1of2    },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_main_to_page2      },
    // Content
    { 1, 20, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_main_brightness },
    { 1, 32, 154, 11, UI_STYLE_NONE,                                 ui_proc_menu_main_uptime     },
    { 1, 44, 154, 11, UI_STYLE_NONE,                                 ui_proc_menu_main_model      },
    { 1, 56, 154, 11, UI_STYLE_NONE,                                 ui_proc_menu_main_sn         },
    { 1, 68, 154, 11, UI_STYLE_NONE,                                 ui_proc_menu_main_version    },
};

static UIScreen ui_menu_screen_page1 = {
    ui_menu_screen_elements_page1,
    NULL,
    ARRAY_SIZE(ui_menu_screen_elements_page1),
    false,
};

// Page 2
static const UIElement ui_menu_screen_elements_page2[] = {
    // Header
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main             },
    { 27,  6, 28, 10, UI_STYLE_NONE,      ui_proc_menu_main_label          },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_main_to_page1       },
    { 127, 6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_label_page_2of2     },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_page_right_inactive },
    // Content
    { 1,  20, 154, 11, UI_STYLE_NONE, ui_proc_menu_main_mcu_flash     },
    { 1,  32, 154, 11, UI_STYLE_NONE, ui_proc_menu_main_eeprom_writes },
};

static UIScreen ui_menu_screen_page2 = {
    ui_menu_screen_elements_page2,
    &(ui_menu_screen_elements_page2[2]),
    ARRAY_SIZE(ui_menu_screen_elements_page2),
    false,
};


//------------------------------------------------------------------------------
// GPS Menu Screen Layout
//------------------------------------------------------------------------------
// Header
static void ui_proc_menu_gps_label(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_to_page1(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_to_page2_right(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_to_page2_left(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_to_page3(const UIElement* element, UICommand command, int32_t encoder_step);
// Page 1
// Page 2
// Page 3
static void ui_proc_menu_gps_module(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_baud_rate(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_gga_frames(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_errors(const UIElement* element, UICommand command, int32_t encoder_step);


// Page 1
static const UIElement ui_gps_screen_elements_page1[] = {
    // Header
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main            },
    { 27,  6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_gps_label          },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_page_left_inactive },
    { 127, 6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_label_page_1of3    },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_gps_to_page2_right },
    // Content
//TODO
};

static UIScreen ui_gps_screen_page1 = {
    ui_gps_screen_elements_page1,
    NULL,
    ARRAY_SIZE(ui_gps_screen_elements_page1),
    false,
};

// Page 2
static const UIElement ui_gps_screen_elements_page2[] = {
    // Header
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main         },
    { 27,  6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_gps_label       },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_gps_to_page1    },
    { 127, 6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_label_page_2of3 },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_gps_to_page3    },
    // Content
//TODO
};

static UIScreen ui_gps_screen_page2 = {
    ui_gps_screen_elements_page2,
    &(ui_gps_screen_elements_page2[4]),
    ARRAY_SIZE(ui_gps_screen_elements_page2),
    false,
};

// Page 3
static const UIElement ui_gps_screen_elements_page3[] = {
    // Header
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main             },
    { 27,  6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_gps_label           },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_gps_to_page2_left   },
    { 127, 6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_label_page_3of3     },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_page_right_inactive },
    // Content
    { 1,  20, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_gps_module     },
    { 1,  32, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_gps_baud_rate  },
    { 1,  44, 154, 11, UI_STYLE_NONE,                                 ui_proc_menu_gps_gga_frames },
    { 1,  56, 154, 23, UI_STYLE_FOCUSABLE,                            ui_proc_menu_gps_errors     },
};

static UIScreen ui_gps_screen_page3 = {
    ui_gps_screen_elements_page3,
    &(ui_gps_screen_elements_page3[2]),
    ARRAY_SIZE(ui_gps_screen_elements_page3),
    false,
};


//------------------------------------------------------------------------------
// PPB Menu Screen Layout
//------------------------------------------------------------------------------
// Header
static void ui_proc_menu_ppb_label(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_ppb_to_page1(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_ppb_to_page2(const UIElement* element, UICommand command, int32_t encoder_step);
// Page 1
static void ui_proc_menu_ppb_mean(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_ppb_inst(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_ppb_freq(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_ppb_pps_err(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_ppb_pwm_corr(const UIElement* element, UICommand command, int32_t encoder_step);
// Page 2
static void ui_proc_menu_ppb_ocxo(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_ppb_warmup(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_ppb_ppb_thr(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_ppb_alg(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_ppb_corr_fact(const UIElement* element, UICommand command, int32_t encoder_step);

// Page 1
static const UIElement ui_ppb_screen_elements_page1[] = {
    // Header
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main            },
    { 27,  6, 28, 10, UI_STYLE_NONE,      ui_proc_menu_ppb_label          },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_page_left_inactive },
    { 127, 6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_label_page_1of2    },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_ppb_to_page2       },
    // Content
    { 1, 20, 154, 11, UI_STYLE_NONE, ui_proc_menu_ppb_mean     },
    { 1, 32, 154, 11, UI_STYLE_NONE, ui_proc_menu_ppb_inst     },
    { 1, 44, 154, 11, UI_STYLE_NONE, ui_proc_menu_ppb_freq     },
    { 1, 56, 154, 11, UI_STYLE_NONE, ui_proc_menu_ppb_pps_err  },
    { 1, 68, 154, 11, UI_STYLE_NONE, ui_proc_menu_ppb_pwm_corr },
};

static UIScreen ui_ppb_screen_page1 = {
    ui_ppb_screen_elements_page1,
    NULL,
    ARRAY_SIZE(ui_ppb_screen_elements_page1),
    false,
};

// Page 2
static const UIElement ui_ppb_screen_elements_page2[] = {
    // Header
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main            },
    { 27,  6, 28, 10, UI_STYLE_NONE,      ui_proc_menu_ppb_label          },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_ppb_to_page1       },
    { 127, 6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_label_page_2of2     },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_page_right_inactive },
    // Content
    { 1, 20, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_ppb_ocxo      },
    { 1, 32, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_ppb_warmup    },
    { 1, 44, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_ppb_ppb_thr   },
    { 1, 56, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_ppb_alg       },
    { 1, 68, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_ppb_corr_fact },
};

static UIScreen ui_ppb_screen_page2 = {
    ui_ppb_screen_elements_page2,
    &(ui_ppb_screen_elements_page2[2]),
    ARRAY_SIZE(ui_ppb_screen_elements_page2),
    false,
};


//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
static void ui_menu_draw_right_aligned(const UIElement* element, int offset_chars, const char* str, uint16_t text_color)
{
    int len = strlen(str);
    ST7735_FillRectangleFast(element->x + offset_chars * 7, element->y + 1, (22 - offset_chars - len) * 7, element->height - 1, UI_COLOR_BG);
    ST7735_WriteStringNoWrap(element->x + (22 - len) * 7, element->y + 1, element->height - 1, str, Font_7x10, text_color, UI_COLOR_BG);
}


//------------------------------------------------------------------------------
// Common Procedures
//------------------------------------------------------------------------------
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

static void ui_proc_menu_label(const UIElement* element, UICommand command, int32_t encoder_step, const char* label)
{
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y, 10, label, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_label_page_1of2(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_label(element, command, encoder_step, "1/2");
}

static void ui_proc_menu_label_page_2of2(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_label(element, command, encoder_step, "2/2");
}

static void ui_proc_menu_label_page_1of3(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_label(element, command, encoder_step, "1/3");
}

static void ui_proc_menu_label_page_2of3(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_label(element, command, encoder_step, "2/3");
}

static void ui_proc_menu_label_page_3of3(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_label(element, command, encoder_step, "3/3");
}

static void ui_proc_menu_readonly_entry(const UIElement* element, UICommand command, int32_t encoder_step, const char* label, int32_t value_offset, const char* value)
{
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, label, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
        ST7735_WriteStringNoWrap(element->x + value_offset * 7, element->y + 1, element->height - 1, value, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_page_switch(const UIElement* element, UICommand command, int32_t encoder_step, const uint16_t* icon_10x15, UIScreen* page)
{
    if (command & UICommand_Init) {
        ST7735_DrawImage(element->x, element->y, 10, 15, icon_10x15);
    }
    if ((command & UICommand_Click) && page != NULL) {
        ui_show_screen(page);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_page_left(const UIElement* element, UICommand command, int32_t encoder_step, UIScreen *left_page)
{
    ui_proc_menu_page_switch(element, command, encoder_step, icon_page_left_10x15, left_page);
}

static void ui_proc_menu_page_right(const UIElement* element, UICommand command, int32_t encoder_step, UIScreen *right_page)
{
    ui_proc_menu_page_switch(element, command, encoder_step, icon_page_right_10x15, right_page);
}

static void ui_proc_menu_page_left_inactive(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_switch(element, command, encoder_step, icon_page_left_inactive_10x15, NULL);
}

static void ui_proc_menu_page_right_inactive(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_switch(element, command, encoder_step, icon_page_right_inactive_10x15, NULL);
}


//------------------------------------------------------------------------------
// Main Menu
//------------------------------------------------------------------------------
static void ui_proc_menu(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_DrawImage(element->x, element->y, 14, 16, icon_menu_14x16);
    }

    if (command & UICommand_Click) {
        ui_show_screen(&ui_menu_screen_page1);
    }

    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// Save
//------------------------------------------------------------------------------
static bool ui_cache_ee_is_changed = false;
static void ui_proc_save_handler(UI_MsgBoxButton result);
static void ui_proc_save(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if ((command & UICommand_Init) || (ee_is_changed != ui_cache_ee_is_changed)) {
        ui_cache_ee_is_changed = ee_is_changed;
        ST7735_DrawImage(element->x, element->y, 15, 16, ui_cache_ee_is_changed ? icon_save_15x16 : icon_save_inactive_15x16);
    }

    if ((command & UICommand_Click) && ui_cache_ee_is_changed) {
        static const char* const msg[] = {
            "Save current device",
            "configuration to",
            "EEPROM?",
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
            static const char* const msg[] = {
                "Cannot save",
                "configuration:",
                "EEPROM write failed",
                NULL };
            ui_msgbox(msg, UI_MsgBoxType_Error, UI_MsgBoxButton_Ok, NULL);
        }
    }
}


//------------------------------------------------------------------------------
// GPS
//------------------------------------------------------------------------------
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
        ui_show_screen(&ui_gps_screen_page1);
    }

    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// PPB
//------------------------------------------------------------------------------
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
        ui_show_screen(&ui_ppb_screen_page1);
    }

    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// USB
//------------------------------------------------------------------------------
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

    if (command & UICommand_Click) {
        switch (ui_cache_cdcio_status) {
        case CDC_STATUS_OK:
            {
                static const char* const msg[] = {
                    "USB connected",
                    NULL };
                ui_msgbox(msg, UI_MsgBoxType_Ok, UI_MsgBoxButton_Ok, NULL);
            }
            break;

        case CDC_STATUS_OVERFLOW:
            {
                static const char* const msg[] = {
                    "USB connected",
                    "PC software is not",
                    "running",
                    NULL };
                ui_msgbox(msg, UI_MsgBoxType_Ok, UI_MsgBoxButton_Ok, NULL);
            }
            break;

        case CDC_STATUS_NO_CONN:
        default:
            {
                static const char* const msg[] = {
                    "No USB connection",
                    NULL };
                ui_msgbox(msg, UI_MsgBoxType_Ok, UI_MsgBoxButton_Ok, NULL);
            }
            break;
        }
    }

    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// Date and Time
//------------------------------------------------------------------------------
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

            ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, date_str, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
        }

        // Draw time
        if ((command & (UICommand_Init | UICommand_Release)) || (gps_time.raw != ui_cache_gps_time.raw)) {
            ui_cache_gps_time = gps_time;

            const char* time_str = NULL;
            char buf_str[10] = { '\0' };

            if (ui_cache_gps_time.raw != GPS_EMPTY_DATE_TIME) {
                snprintf(buf_str, ARRAY_SIZE(buf_str), " %02u:%02u:%02u",
                                                       (ui_cache_gps_time.hours > 99u) ? 99u : ui_cache_gps_time.hours,
                                                       (ui_cache_gps_time.minutes > 99u) ? 99u : ui_cache_gps_time.minutes,
                                                       (ui_cache_gps_time.seconds > 99u) ? 99u : ui_cache_gps_time.seconds);
                time_str = buf_str;
            } else {
                time_str = " --:--:--";
            }

            ST7735_WriteStringNoWrap(element->x + 11 * 7, element->y + 1, element->height - 1, time_str, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
        }
    }

    // Time offset edit
    {
        if (command & (UICommand_Capture | UICommand_RestoreCapture)) {
            // Draw label and erase the remaining element area
            ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "Time offset: ", Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
            ST7735_FillRectangleFast(element->x + 13 * 7, element->y + 1, 7*7, element->height, UI_COLOR_BG);
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            ui_change_setting_i8(&gps_time_offset, encoder_step, GPS_MIN_TIME_OFFSET, GPS_MAX_TIME_OFFSET, false);
        }

        if (command & (UICommand_Capture | UICommand_RestoreCapture | UICommand_EncoderStep)) {
            // Draw value
            char s[5] = { '\0' };
            snprintf(s, ARRAY_SIZE(s), "%3d", gps_time_offset);
            ST7735_WriteStringNoWrap(element->x + 13 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
        }

        if (command & UICommand_Release) {
            // Apply changes
            uint32_t new_time_offset = (uint32_t)(gps_time_offset - GPS_MIN_TIME_OFFSET);
            if (ee_storage.gps_time_offset != new_time_offset) {
                ee_storage.gps_time_offset = new_time_offset;
                ee_is_changed = true;
            }
        }
    }

    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// PPS Indicator and Spinner
//------------------------------------------------------------------------------
static uint32_t ui_spinner_last_update = 0;
static uint8_t  ui_spinner_frame       = 0;
static bool     ui_cache_pps_active    = false;
static void ui_proc_pps(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ui_spinner_last_update = 0;
        ui_spinner_frame       = 0;
    }

    // Draw spinner
    if (timer_is_elapsed(&ui_spinner_last_update, 125)) {
        ST7735_DrawImage(element->x + 10, element->y + 1, 7, 7, icon_spinner_12st_7x7[ui_spinner_frame]);

        if (++ui_spinner_frame > 11) {
            ui_spinner_frame = 0;
        }
    }

    // Draw PPS
    bool pps_active = (HAL_GPIO_ReadPin(PPS_GPIO_Port, PPS_Pin) == GPIO_PIN_SET);
    if ((command & UICommand_Init) || (pps_active != ui_cache_pps_active)) {
        ui_cache_pps_active = pps_active;

        uint16_t color = pps_active ? UI_COLOR_PPS_INDICATOR : UI_COLOR_BG;
        ST7735_FillRectangleFast(element->x + 1, element->y + 1, 7, 7, color);
    }

    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// Status and Warmup Countdown
//------------------------------------------------------------------------------
static uint32_t ui_cache_warmup_remaining_sec = UINT16_MAX;
static bool     ui_cache_pll_status_error     = false;
static void ui_proc_status_show_status();
static void ui_proc_status(const UIElement* element, UICommand command, int32_t encoder_step)
{
    bool draw_icon = false;
    bool draw_timer = false;
    if (command & UICommand_Init) {
        draw_icon  = true;
        draw_timer = true;
    }

    bool pll_status_error = (pll_status != PllStatus_Ok);
    if (pll_status_error != ui_cache_pll_status_error) {
        ui_cache_pll_status_error = pll_status_error;
        draw_icon = true;
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
            if (ui_cache_pll_status_error) {
                ST7735_DrawImage(element->x + 1, element->y, 15, 12, icon_status_err_warmup_15x12);
            } else {
                ST7735_DrawImage(element->x + 1, element->y, 15, 12, icon_status_ok_warmup_15x12);
            }
        } else {
            if (ui_cache_pll_status_error) {
                ST7735_DrawImage(element->x + 1, element->y, 15, 12, icon_status_err_15x12);
            } else {
                ST7735_DrawImage(element->x + 1, element->y, 15, 12, icon_status_ok_15x12);
            }
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

    // Show status
    if (command & UICommand_Click) {
        ui_proc_status_show_status();
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_status_show_status()
{
    // Messages
    static const char* const msg_ok[] = {
        "Normal operation",
        "No errors",
        NULL };
    static const char* const msg_warmup[] = {
        "Normal operation",
        "OCXO warming up...",
        NULL };
    static const char* const msg_err_gen[] = {
        "General PLL error",
        NULL };
    static const char* const msg_err_a[] = {
        "Main PLL: no lock",
        NULL };
    static const char* const msg_err_b[] = {
        "PLL 2: no lock",
        NULL };
    static const char* const msg_err_ab[] = {
        "PLLs 1 & 2: No Lock",
        NULL };

    PllStatus status      = pll_status;
    bool      err_general = ((status & PllStatus_General_Error) != 0);
    bool      err_pll_a   = ((status & PllStatus_PllA_Error) != 0);
    bool      err_pll_b   = ((status & PllStatus_PllB_Error) != 0);

    // Select message
    UI_MsgBoxType type = UI_MsgBoxType_Error;
    const char* const* msg;
    if (err_general) { // General PLL error
        msg = msg_err_gen;
    } else if (err_pll_a && err_pll_b) { // Both PLL: no lock
        msg = msg_err_ab;
    } else if (err_pll_a) { // PLL A: no lock
        msg = msg_err_a;
    } else if (err_pll_b) { // PLL B: no lock
        msg = msg_err_b;
    } else { // No error
        msg  = (ui_cache_warmup_remaining_sec > 0) ? msg_warmup : msg_ok;
        type = UI_MsgBoxType_Ok;
    }

    ui_msgbox(msg, type, UI_MsgBoxButton_Ok, NULL);
}


//------------------------------------------------------------------------------
// Outputs
//------------------------------------------------------------------------------
static void ui_proc_out(const UIElement* element, UICommand command, int32_t encoder_step,
    uint8_t out, const OutFreqConfig *pll_presets, uint16_t preset_count, uint16_t *ui_edit_preset, uint16_t *ee_preset)
{
    const OutFreqConfig *preset_to_draw = NULL;

    if (command & UICommand_Init) {
        // Draw icon and label
        ST7735_DrawImage(element->x, element->y + 2, 7, 7, icon_out_7x7);
        char s[3] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%1u:", out);
        ST7735_WriteStringNoWrap(element->x + 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_OUT_LABEL, UI_COLOR_BG);

        // Draw value
        if (!ui_is_captured(element)) {
            preset_to_draw = &(pll_presets[*ee_preset]);
        } else {
            preset_to_draw = &(pll_presets[*ui_edit_preset]);
        }
    }

    // Preset selection
    {
        if (command & UICommand_Capture) {
            *ui_edit_preset = *ee_preset;
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            ui_change_setting_u16(ui_edit_preset, encoder_step, 0, preset_count - 1u, true);

            // Draw new value
            preset_to_draw = &(pll_presets[*ui_edit_preset]);
        }

        if (command & UICommand_Release) {
            // Apply changes and configure PLL
            if (*ee_preset != *ui_edit_preset) {
                *ee_preset = *ui_edit_preset;
                ee_is_changed = true;

                pll_configure_output(out, &(pll_presets[*ee_preset]));
            }
        }
    }

    if (preset_to_draw) {
        ST7735_WriteStringNoWrap(element->x + 3 * 7, element->y + 1, element->height - 1, preset_to_draw->label, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static uint16_t ui_edit_pll_out1_preset = 0;
static void ui_proc_out1(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_out(element, command, encoder_step, 1, pll_out1_presets, pll_out1_preset_count, &ui_edit_pll_out1_preset, &(ee_storage.pll_out1_preset));
}

static uint16_t ui_edit_pll_out2_preset = 0;
static void ui_proc_out2(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_out(element, command, encoder_step, 2, pll_out2_presets, pll_out2_preset_count, &ui_edit_pll_out2_preset, &(ee_storage.pll_out2_preset));
}


//------------------------------------------------------------------------------
// PWM
//------------------------------------------------------------------------------
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
        snprintf(s, ARRAY_SIZE(s), "%5u", pwm);
        ST7735_WriteStringNoWrap(element->x + 4 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    if (command & UICommand_Click) {
        if (ee_storage.pwm != pwm) {
            ee_storage.pwm = pwm;
            ee_is_changed = true;
        }

        static const char* const msg[] = {
            "The PWM value has",
            "been set",
            NULL };
        ui_msgbox(msg, UI_MsgBoxType_Ok, UI_MsgBoxButton_Ok, NULL);
    }

    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// Trend
//------------------------------------------------------------------------------
static void ui_proc_trend_h(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (!ui_is_captured(element)) {
        if (command & (UICommand_Init | UICommand_Release)) {
            // todo
            ST7735_FillRectangleFast(element->x, element->y, element->width, 1, UI_COLOR_TREND_BG);
            ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "H:", Font_7x10, UI_COLOR_TREND_BAR, UI_COLOR_TREND_BG);
            ST7735_WriteStringNoWrap(element->x + 2 * 7, element->y + 1, element->height - 1, "10", Font_7x10, UI_COLOR_TREND_BAR, UI_COLOR_TREND_BG);
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

static void ui_proc_trend_separator(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_FillRectangle(element->x, element->y, element->width, element->height, UI_COLOR_TREND_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_trend_v(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (!ui_is_captured(element)) {
        if (command & (UICommand_Init | UICommand_Release)) {
            // todo
            ST7735_FillRectangleFast(element->x, element->y, element->width, 1, UI_COLOR_TREND_BG);
            ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "V:", Font_7x10, UI_COLOR_TREND_BAR, UI_COLOR_TREND_BG);
            ST7735_WriteStringNoWrap(element->x + 2 * 7, element->y + 1, element->height - 1, "120", Font_7x10, UI_COLOR_TREND_BAR, UI_COLOR_TREND_BG);
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
        ST7735_FillRectangleFast(element->x, element->y, element->width, element->height, UI_COLOR_TREND_BG);
    }
    // TODO: draw
    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// Main Menu Procedures
//------------------------------------------------------------------------------
static void ui_proc_menu_main_label(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_label(element, command, encoder_step, "MENU");
}

static void ui_proc_menu_main_to_page1(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_left(element, command, encoder_step, &ui_menu_screen_page1);
}

static void ui_proc_menu_main_to_page2(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_right(element, command, encoder_step, &ui_menu_screen_page2);
}

static uint8_t ui_edit_brightness = 0;
static void ui_proc_menu_main_brightness(const UIElement* element, UICommand command, int32_t encoder_step)
{
    uint8_t* brightness_to_draw = NULL;

    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "Brightness:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        if (!ui_is_captured(element)) {
            brightness_to_draw = &(ee_storage.brightness);
        } else {
            brightness_to_draw = &ui_edit_brightness;
        }
    }

    // Brightness adjustment
    {
        if (command & UICommand_Capture) {
            ui_edit_brightness = ee_storage.brightness;
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            ui_change_setting_u8(&ui_edit_brightness, encoder_step * 5, 5, 100, false);

            // Set and draw the new value
            set_brightness(ui_edit_brightness);
            brightness_to_draw = &ui_edit_brightness;
        }
 
        if (command & UICommand_Release) {
            // Apply changes
            if (ee_storage.brightness != ui_edit_brightness) {
                ee_storage.brightness = ui_edit_brightness;
                ee_is_changed = true;
            }
        }
    }

    if (brightness_to_draw) {
        char s[5] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%3u%%", *brightness_to_draw);
        ST7735_WriteStringNoWrap(element->x + 18 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static uint32_t ui_cache_device_uptime = 0;
static void ui_proc_menu_main_uptime(const UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "Uptime:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw value
    uint32_t uptime = device_uptime;
    if ((command & UICommand_Init) || (uptime != ui_cache_device_uptime)) {
        ui_cache_device_uptime = uptime;

        char s[13] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%10" PRIu32 " s", uptime);
        ST7735_WriteStringNoWrap(element->x + 10 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_main_model(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_readonly_entry(element, command, encoder_step, "Model:", 16, DEVICE_MODEL);
}

static void ui_proc_menu_main_sn(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "S/N:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        uint32_t uid_w0 = *(volatile uint32_t*)UID_BASE;
        uint32_t uid_w1 = *(volatile uint32_t*)(UID_BASE + 4);
        char     s[17]  = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%08" PRIX32 "%08" PRIX32, uid_w0, uid_w1);
        ST7735_WriteStringNoWrap(element->x + 6 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_main_version(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_readonly_entry(element, command, encoder_step, "Version:", 12, FIRMWARE_VERSION);
}

static void ui_proc_menu_main_mcu_flash(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "MCU FLASH:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        char s[8] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%5" PRIu16 "KB", *(uint16_t*)FLASHSIZE_BASE);
        ST7735_WriteStringNoWrap(element->x + 15 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static uint32_t ui_cache_eeprom_total_writes = 0;
static void ui_proc_menu_main_eeprom_writes(const UIElement* element, UICommand command, int32_t encoder_step)
{
     // Draw label
     if (command & UICommand_Init) {
         ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "EEPROM Writes:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
     }

     // Draw value
     uint32_t total_writes = ee_storage.total_writes;
     if ((command & UICommand_Init) || (total_writes != ui_cache_eeprom_total_writes)) {
         ui_cache_eeprom_total_writes = total_writes;

         char s[11] = { '\0' };
         snprintf(s, ARRAY_SIZE(s), "%8" PRIu32, total_writes);
         ST7735_WriteStringNoWrap(element->x + 14 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
     }

     ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// GPS Menu Procedures
//------------------------------------------------------------------------------
static void ui_proc_menu_gps_label(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_label(element, command, encoder_step, "GPS");
}

static void ui_proc_menu_gps_to_page1(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_left(element, command, encoder_step, &ui_gps_screen_page1);
}

static void ui_proc_menu_gps_to_page2_right(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_right(element, command, encoder_step, &ui_gps_screen_page2);
}

static void ui_proc_menu_gps_to_page2_left(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_left(element, command, encoder_step, &ui_gps_screen_page2);
}

static void ui_proc_menu_gps_to_page3(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_right(element, command, encoder_step, &ui_gps_screen_page3);
}

static uint8_t ui_edit_gps_model = 0;
static void ui_proc_menu_gps_module(const UIElement* element, UICommand command, int32_t encoder_step)
{
    const uint8_t* model_to_draw = NULL;

    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "Module:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        if (!ui_is_captured(element)) {
            model_to_draw = &(ee_storage.gps_model);
        } else {
            model_to_draw = &ui_edit_gps_model;
        }
    }

    // Model selection
    {
        if (command & UICommand_Capture) {
            ui_edit_gps_model = ee_storage.gps_model;
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            ui_change_setting_u8(&ui_edit_gps_model, encoder_step, 0, GPS_MODEL_MAX, true);

            // Draw new value
            model_to_draw = &ui_edit_gps_model;
        }

        if (command & UICommand_Release) {
            // Apply changes
            if (ee_storage.gps_model != ui_edit_gps_model) {
                if ((ui_edit_gps_model == GPS_MODEL_ATGM336H) && (ee_storage.gps_baudrate > 115200)) {
                    ee_storage.gps_baudrate = 115200;
                    gps_setbaudrate(ee_storage.gps_baudrate);
                }

                ee_storage.gps_model = ui_edit_gps_model;
                ee_is_changed = true;
            }
        }
    }

    // Draw value
    if (model_to_draw) {
        ui_menu_draw_right_aligned(element, 14, gps_model_type_to_string(*model_to_draw), UI_COLOR_TEXT);
    }

    ui_default_element_proc(element, command, encoder_step);
}

void ui_change_baudrate(uint32_t* baudrate, int32_t step)
{
    uint32_t b = *baudrate;

    if (step > 0)
    {
        if (b < 9600)        b = 9600;
        else if (b < 19200)  b = 19200;
        else if (b < 38400)  b = 38400;
        else if (b < 57600)  b = 57600;
        else if (b < 115200) b = 115200;
        else if (b < 230400) b = 230400;
        else if (b < 460800) b = 460800;
        else if (b < 921600) b = 921600;
    }
    else
    {
        if (b > 921600)      b = 921600;
        else if (b > 460800) b = 460800;
        else if (b > 230400) b = 230400;
        else if (b > 115200) b = 115200;
        else if (b > 57600)  b = 57600;
        else if (b > 38400)  b = 38400;
        else if (b > 19200)  b = 19200;
        else if (b > 9600)   b = 9600;
    }

    *baudrate = b;
}

static uint32_t ui_cache_gps_baudrate = 0;
static uint32_t ui_edit_gps_baudrate = 0;
static void ui_proc_menu_gps_baud_rate(const UIElement* element, UICommand command, int32_t encoder_step)
{
    uint32_t *baudrate_to_draw = NULL;
    bool is_captured = ui_is_captured(element);

    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "Baud Rate:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        if (!is_captured) {
            baudrate_to_draw = &(ee_storage.gps_baudrate);
        } else {
            baudrate_to_draw = &ui_edit_gps_baudrate;
        }
    }

    if (!is_captured && (ee_storage.gps_baudrate != ui_cache_gps_baudrate)) {
        baudrate_to_draw = &(ee_storage.gps_baudrate);
    }

    // Baud rate selection
    {
        if (command & UICommand_Capture) {
            ui_edit_gps_baudrate = ee_storage.gps_baudrate;
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            ui_change_baudrate(&ui_edit_gps_baudrate, encoder_step);
            if ((ee_storage.gps_model == GPS_MODEL_ATGM336H) && ui_edit_gps_baudrate > 115200) {
                ui_edit_gps_baudrate = 115200;
            }

            // Draw new value
            baudrate_to_draw = &ui_edit_gps_baudrate;
        }

        if (command & UICommand_Release) {
            // Apply changes and re-configure UART
            if (ee_storage.gps_baudrate != ui_edit_gps_baudrate) {
                ee_storage.gps_baudrate = ui_edit_gps_baudrate;
                ee_is_changed = true;

                gps_setbaudrate(ui_edit_gps_baudrate);
            }
        }
    }

    // Draw value
    if (baudrate_to_draw) {
        ui_cache_gps_baudrate = *baudrate_to_draw;
        char s[11] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%6" PRIu32, *baudrate_to_draw);
        ST7735_WriteStringNoWrap(element->x + 16 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static uint32_t ui_cache_gga_frames = 0;
static void ui_proc_menu_gps_gga_frames(const UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "GGA Frames:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw value
    uint32_t frames = gga_frames;
    if ((command & UICommand_Init) || (frames != ui_cache_gga_frames)) {
        ui_cache_gga_frames = frames;

        char s[11] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%10" PRIu32, frames);
        ST7735_WriteStringNoWrap(element->x + 12 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static uint32_t ui_cache_gps_invalid_frames     = 0;
static uint32_t ui_cache_gps_fifo_overflow_gps  = 0;
static uint32_t ui_cache_gps_fifo_overflow_comm = 0;
static void ui_proc_menu_gps_errors(const UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "GPS Err/GPS OVF/PC OVF", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    if (command & UICommand_Click)
    {
        static const char* const msg[] = {
            "Invalid GPS frames/",
            "GPS FIFO overflows/",
            "PC FIFO overflows",
            NULL };
        ui_msgbox(msg, UI_MsgBoxType_Ok, UI_MsgBoxButton_Ok, NULL);
    }

    // Draw values
    uint32_t invalid_frames = gps_invalid_frames;
    uint32_t overflow_gps   = gps_fifo_overflow_gps;
    uint32_t overflow_comm  = gps_fifo_overflow_comm;
    if ((command & UICommand_Init) ||
        (invalid_frames != ui_cache_gps_invalid_frames) ||
        (overflow_gps != ui_cache_gps_fifo_overflow_gps) ||
        (overflow_comm != ui_cache_gps_fifo_overflow_comm))
    {
        ui_cache_gps_invalid_frames = invalid_frames;
        ui_cache_gps_fifo_overflow_gps = overflow_gps;
        ui_cache_gps_fifo_overflow_comm = overflow_comm;

        char s[33] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%" PRIu32 "/%" PRIu32 "/%" PRIu32, invalid_frames, overflow_gps, overflow_comm);
        ST7735_WriteStringNoWrap(element->x, element->y + 13, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// PPB Menu Procedures
//------------------------------------------------------------------------------
static void ui_proc_menu_ppb_label(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_label(element, command, encoder_step, "PPB");
}

static void ui_proc_menu_ppb_to_page1(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_left(element, command, encoder_step, &ui_ppb_screen_page1);
}

static void ui_proc_menu_ppb_to_page2(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_right(element, command, encoder_step, &ui_ppb_screen_page2);
}

static int32_t ui_cache_menu_frequency_ppb_x100 = PPB_UNSET_VALUE;
static void ui_proc_menu_ppb_mean(const UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "Mean PPB:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw value
    int32_t ppb = frequency_ppb_x100;
    if ((command & UICommand_Init) || (ppb != ui_cache_menu_frequency_ppb_x100)) {
        ui_cache_menu_frequency_ppb_x100 = ppb;

        char s[10] = { '\0' };
        ui_format_ppb_9char(ppb, s, ARRAY_SIZE(s));
        ST7735_WriteStringNoWrap(element->x + 13 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static int32_t ui_cache_frequency_inst_ppb_x100 = PPB_UNSET_VALUE;
static void ui_proc_menu_ppb_inst(const UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "Inst. PPB:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw value
    int32_t ppb = frequency_get_inst_ppb_x100();
    if ((command & UICommand_Init) || (ppb != ui_cache_frequency_inst_ppb_x100)) {
        ui_cache_frequency_inst_ppb_x100 = ppb;

        char s[10] = { '\0' };
        ui_format_ppb_9char(ppb, s, ARRAY_SIZE(s));
        ST7735_WriteStringNoWrap(element->x + 13 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static int32_t ui_cache_ppb_frequency = 0;
static void ui_proc_menu_ppb_freq(const UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "Core Freq.:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw value
    int32_t freq = ppb_frequency;
    if ((command & UICommand_Init) || (freq != ui_cache_ppb_frequency)) {
        ui_cache_ppb_frequency = freq;

        if (freq == 0) {
            ST7735_WriteStringNoWrap(element->x + 19 * 7, element->y + 1, element->height - 1, "N/A", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
        } else {
            char s[11] = { '\0' };
            snprintf(s, ARRAY_SIZE(s), "%10" PRIi32, freq);
            ST7735_WriteStringNoWrap(element->x + 12 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
        }
    }

    ui_default_element_proc(element, command, encoder_step);
}

static int32_t ui_cache_ppb_millis = 0;
static void ui_proc_menu_ppb_pps_err(const UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "PPS Error:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw value
    int32_t millis = ppb_millis;
    if ((command & UICommand_Init) || (millis != ui_cache_ppb_millis)) {
        ui_cache_ppb_millis = millis;

        char s[13] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%8" PRIi32 " ms", ui_limit_i32(millis, -9999999, 9999999));
        ST7735_WriteStringNoWrap(element->x + 11 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static uint16_t ui_cache_menu_pwm         = 0;
static int32_t  ui_cache_menu_corr        = 0;
static bool     ui_cache_menu_warmup_done = false;
static void ui_proc_menu_ppb_pwm_corr(const UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "PWM/Corr.:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw value
    uint16_t pwm = TIM1->CCR2;
    int32_t  corr = ppb_correction;
    bool     warmup_done = frequency_adjustment_allowed();
    if ((command & UICommand_Init) || (pwm != ui_cache_menu_pwm) || (corr != ui_cache_menu_corr) || (warmup_done != ui_cache_menu_warmup_done)) {
        ui_cache_menu_pwm         = pwm;
        ui_cache_menu_corr        = corr;
        ui_cache_menu_warmup_done = warmup_done;

        char s[13] = { '\0' };
        if (warmup_done) {
            snprintf(s, ARRAY_SIZE(s), "%" PRIu16 "/%" PRIi32, pwm, ui_limit_i32(corr, -99999, 99999));
        } else {
            snprintf(s, ARRAY_SIZE(s), "%" PRIu16 "/WARMUP", pwm);
        }

        ui_menu_draw_right_aligned(element, 10, s, UI_COLOR_MENU_LABEL);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static uint8_t ui_edit_ocxo_model = 0;
static void ui_proc_menu_ppb_ocxo(const UIElement* element, UICommand command, int32_t encoder_step)
{
    const uint8_t* model_to_draw = NULL;

    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "OCXO:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        if (!ui_is_captured(element)) {
            model_to_draw = &(ee_storage.ocxo_model);
        } else {
            model_to_draw = &ui_edit_ocxo_model;
        }
    }

    // Model selection
    {
        if (command & UICommand_Capture) {
            ui_edit_ocxo_model = ee_storage.ocxo_model;
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            ui_change_setting_u8(&ui_edit_ocxo_model, encoder_step, 0, OCXO_MODEL_MAX, true);

            // Draw new value
            model_to_draw = &ui_edit_ocxo_model;
        }

        if (command & UICommand_Release) {
            // Apply changes
            if (ee_storage.ocxo_model != ui_edit_ocxo_model) {
                // Set warm-up time for the selected OCXO
                ee_storage.warmup_time_seconds = get_default_warmup_time(ui_edit_ocxo_model);

                ee_storage.ocxo_model = ui_edit_ocxo_model;
                ee_is_changed = true;
            }
        }
    }

    // Draw value
    if (model_to_draw) {
        ui_menu_draw_right_aligned(element, 15, ocxo_model_type_to_string(*model_to_draw), UI_COLOR_TEXT);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static uint32_t ui_cache_warmup_time_seconds = 0;
static uint32_t ui_edit_warmup_time_seconds = 0;
static void ui_proc_menu_ppb_warmup(const UIElement* element, UICommand command, int32_t encoder_step)
{
    uint32_t *value_to_draw = NULL;
    bool is_captured = ui_is_captured(element);

    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "Warm-up Time:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        if (!is_captured) {
            value_to_draw = &(ee_storage.warmup_time_seconds);
        } else {
            value_to_draw = &ui_edit_warmup_time_seconds;
        }
    }

    if (!is_captured && (ee_storage.warmup_time_seconds != ui_cache_warmup_time_seconds)) {
        value_to_draw = &(ee_storage.warmup_time_seconds);
    }

    // Value edit
    {
        if (command & UICommand_Capture) {
            ui_edit_warmup_time_seconds = ee_storage.warmup_time_seconds;
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            ui_change_setting_u32(&ui_edit_warmup_time_seconds, encoder_step, 0, 1800, false);

            // Draw new value
            value_to_draw = &ui_edit_warmup_time_seconds;
        }

        if (command & UICommand_Release) {
            // Apply changes
            if (ee_storage.warmup_time_seconds != ui_edit_warmup_time_seconds) {
                ee_storage.warmup_time_seconds = ui_edit_warmup_time_seconds;
                ee_is_changed = true;
            }
        }
    }

    // Draw value
    if (value_to_draw) {
        ui_cache_warmup_time_seconds = *value_to_draw;

        char s[9] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%6" PRIu32 " s", *value_to_draw);
        ST7735_WriteStringNoWrap(element->x + 14 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static uint32_t ui_edit_ppb_lock_threshold = 0;
static void ui_proc_menu_ppb_ppb_thr(const UIElement* element, UICommand command, int32_t encoder_step)
{
    const uint32_t* value_to_draw = NULL;

    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "PPB Lock Thr.:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        if (!ui_is_captured(element)) {
            value_to_draw = &(ee_storage.ppb_lock_threshold);
        } else {
            value_to_draw = &ui_edit_ppb_lock_threshold;
        }
    }

    // Value edit
    {
        if (command & UICommand_Capture) {
            ui_edit_ppb_lock_threshold = ee_storage.ppb_lock_threshold;
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            ui_change_setting_u32(&ui_edit_ppb_lock_threshold, 5*encoder_step, 0, MAX_PPB_LOCK_THRESHOLD, false);

            // Draw new value
            value_to_draw = &ui_edit_ppb_lock_threshold;
        }

        if (command & UICommand_Release) {
            // Apply changes
            if (ee_storage.ppb_lock_threshold != ui_edit_ppb_lock_threshold) {
                ee_storage.ppb_lock_threshold = ui_edit_ppb_lock_threshold;
                ee_is_changed = true;
            }
        }
    }

    // Draw value
    if (value_to_draw) {
        char s[12] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%3" PRIu32 ".%02" PRIu32, (*value_to_draw) / 100, (*value_to_draw) % 100);
        ST7735_WriteStringNoWrap(element->x + 16 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static uint8_t ui_edit_correction_algorithm = 0;
static void ui_proc_menu_ppb_alg(const UIElement* element, UICommand command, int32_t encoder_step)
{
    const uint8_t* value_to_draw = NULL;

    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "Algorithm:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        if (!ui_is_captured(element)) {
            value_to_draw = &(ee_storage.correction_algorithm);
        } else {
            value_to_draw = &ui_edit_correction_algorithm;
        }
    }

    // Algorithm selection
    {
        if (command & UICommand_Capture) {
            ui_edit_correction_algorithm = ee_storage.correction_algorithm;
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            ui_change_setting_u8(&ui_edit_correction_algorithm, encoder_step, 0, CORRECTION_ALGO_MAX, true);

            // Draw new value
            value_to_draw = &ui_edit_correction_algorithm;
        }

        if (command & UICommand_Release) {
            // Apply changes
            if (ee_storage.correction_algorithm != ui_edit_correction_algorithm) {
                // Reset correction factor to default value when algorithm is changed
                ee_storage.correction_factor = get_default_correction_factor(ui_edit_correction_algorithm);

                ee_storage.correction_algorithm = ui_edit_correction_algorithm;
                ee_is_changed = true;
            }
        }
    }

    // Draw value
    if (value_to_draw) {
        ui_menu_draw_right_aligned(element, 15, correction_algo_type_to_string(*value_to_draw), UI_COLOR_TEXT);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static uint32_t ui_cache_correction_factor = 0;
static uint32_t ui_edit_correction_factor = 0;
static void ui_proc_menu_ppb_corr_fact(const UIElement* element, UICommand command, int32_t encoder_step)
{
    uint32_t *value_to_draw = NULL;
    bool is_captured = ui_is_captured(element);

    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "Corr. Factor:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        if (!is_captured) {
            value_to_draw = &(ee_storage.correction_factor);
        } else {
            value_to_draw = &ui_edit_correction_factor;
        }
    }

    if (!is_captured && (ee_storage.correction_factor != ui_cache_correction_factor)) {
        value_to_draw = &(ee_storage.correction_factor);
    }

    // Value edit
    {
        if (command & UICommand_Capture) {
            ui_edit_correction_factor = ee_storage.correction_factor;
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            increment_correction_factor_value(&ui_edit_correction_factor, ee_storage.correction_algorithm, encoder_step);

            // Draw new value
            value_to_draw = &ui_edit_correction_factor;
        }

        if (command & UICommand_Release) {
            // Apply changes
            if (ee_storage.correction_factor != ui_edit_correction_factor) {
                ee_storage.correction_factor = ui_edit_correction_factor;
                ee_is_changed = true;
            }
        }
    }

    // Draw value
    if (value_to_draw) {
        ui_cache_correction_factor = *value_to_draw;

        char s[9] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%8" PRIu32, *value_to_draw);
        ST7735_WriteStringNoWrap(element->x + 14 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}
