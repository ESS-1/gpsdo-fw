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

#define UI_MENU_STR_LEN (22u)

static void ui_menu_draw_right_aligned(const UIElement* element, int offset_chars, const char* str, uint16_t text_color);
static void ui_proc_icon_navigation_btn(const UIElement* element, UICommand command, int32_t encoder_step,
    uint16_t icon_width, uint16_t icon_height, const uint16_t* icon, UIScreen* target_screen);
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
static void ui_proc_check_box(const UIElement* element, UICommand command, int32_t encoder_step, const char* label, uint8_t* ui_cache, uint8_t* ee_setting);
static void ui_proc_menu_readonly_u32(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, uint32_t* ui_cache, uint32_t value, uint16_t value_offset, const char* fmt);
static void ui_proc_menu_readonly_i32(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, int32_t* ui_cache, int32_t value, uint16_t value_offset, const char* fmt);
static void ui_proc_menu_edit_custom_u32(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, uint32_t* ui_cache, uint32_t* ui_edit, uint32_t *ee_setting, uint16_t value_offset,
    void (*apply_value)(uint32_t), void (*change_value)(uint32_t*, int32_t), void (*format)(char*, size_t, const char*, uint32_t), const char* format_param);
static void ui_format_u32(char* buf, size_t buf_char_count, const char* fmt, uint32_t value);
static void ui_proc_menu_edit_u32(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, uint32_t* ui_cache, uint32_t* ui_edit, uint32_t *ee_setting, uint16_t value_offset, const char* fmt,
    void (*apply_value)(uint32_t), void (*change_value)(uint32_t*, int32_t));
static void ui_proc_menu_edit_enum_u8(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, uint8_t* ui_edit, uint8_t* ee_setting, uint16_t value_offset, uint8_t max_value,
    void (*before_apply)(uint8_t), const char* (*value_to_string)(uint8_t));
static void ui_proc_menu_string_parameter(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, uint32_t* ui_cache_gga, const char* value, size_t value_char_count);


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
static void ui_proc_out1_drv_str(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_out1_freq(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_out2_drv_str(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_out2_freq(const UIElement* element, UICommand command, int32_t encoder_step);
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
    { 142, 20, 17,  9,  UI_STYLE_FOCUSABLE,                            ui_proc_pps             },
    { 1,   33, 17,  12, UI_STYLE_FOCUSABLE,                            ui_proc_status          },
//  { 1,   46, 17,  7 } - empty space reserved for the warm-up countdown timer drawn by 'ui_proc_status'
    { 23,  32, 21,  10, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_out1_drv_str    },
    { 45,  32, 28,  10, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_out1_freq       },
    { 81,  32, 21,  10, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_out2_drv_str    },
    { 103, 32, 56,  10, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_out2_freq       },
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
static void ui_proc_menu_main_all_settings(const UIElement* element, UICommand command, int32_t encoder_step);

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
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE,  ui_proc_back_to_main             },
    { 27,  6, 28, 10, UI_STYLE_NONE,       ui_proc_menu_main_label          },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE,  ui_proc_menu_main_to_page1       },
    { 127, 6, 21, 10, UI_STYLE_NONE,       ui_proc_menu_label_page_2of2     },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE,  ui_proc_menu_page_right_inactive },
    // Content
    { 1,  20, 154, 11, UI_STYLE_NONE,      ui_proc_menu_main_mcu_flash      },
    { 1,  32, 154, 11, UI_STYLE_NONE,      ui_proc_menu_main_eeprom_writes  },
    { 60, 64, 95,  15, UI_STYLE_FOCUSABLE, ui_proc_menu_main_all_settings   },
};

static UIScreen ui_menu_screen_page2 = {
    ui_menu_screen_elements_page2,
    &(ui_menu_screen_elements_page2[2]),
    ARRAY_SIZE(ui_menu_screen_elements_page2),
    false,
};


//------------------------------------------------------------------------------
// World Map Screen Layout
//------------------------------------------------------------------------------
static void ui_proc_world_map(const UIElement* element, UICommand command, int32_t encoder_step);

static const UIElement ui_world_map_screen_elements[] = {
    { 0, 0, 160, 80, UI_STYLE_FOCUSABLE | UI_STYLE_NOFRAME, ui_proc_world_map },
};

UIScreen ui_world_map_screen = {
    ui_world_map_screen_elements,
    NULL,
    ARRAY_SIZE(ui_world_map_screen_elements),
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
static void ui_proc_menu_gps_latitude_dd(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_latitude_ddm(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_longitude_dd(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_longitude_ddm(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_altitude(const UIElement* element, UICommand command, int32_t encoder_step);
// Page 2
static void ui_proc_menu_gps_locator(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_hdop(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_geoid_sep(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_gps_map(const UIElement* element, UICommand command, int32_t encoder_step);
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
    { 1, 20, 154, 11, UI_STYLE_NONE, ui_proc_menu_gps_latitude_dd   },
    { 1, 32, 154, 11, UI_STYLE_NONE, ui_proc_menu_gps_latitude_ddm  },
    { 1, 44, 154, 11, UI_STYLE_NONE, ui_proc_menu_gps_longitude_dd  },
    { 1, 56, 154, 11, UI_STYLE_NONE, ui_proc_menu_gps_longitude_ddm },
    { 1, 68, 154, 11, UI_STYLE_NONE, ui_proc_menu_gps_altitude      },
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
    { 1,  20, 154, 11, UI_STYLE_NONE,      ui_proc_menu_gps_locator   },
    { 1,  32, 154, 11, UI_STYLE_NONE,      ui_proc_menu_gps_hdop      },
    { 1,  44, 154, 11, UI_STYLE_NONE,      ui_proc_menu_gps_geoid_sep },
    { 60, 64, 95,  15, UI_STYLE_FOCUSABLE, ui_proc_menu_gps_map       },
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
// PPS Menu Screen Layout
//------------------------------------------------------------------------------
// Header
static void ui_proc_menu_pps_label(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_pps_to_page1(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_pps_to_page2(const UIElement* element, UICommand command, int32_t encoder_step);
// Page 1
static void ui_proc_menu_pps_shift_cyc(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_pps_shift_ms(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_pps_sync_cnt(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_pps_force_sync(const UIElement* element, UICommand command, int32_t encoder_step);
// Page 2
static void ui_proc_menu_pps_sync_on_ppb_lock(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_pps_auto_sync(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_pps_sync_delay(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_pps_sync_threshold(const UIElement* element, UICommand command, int32_t encoder_step);

// Page 1
static const UIElement ui_pps_screen_elements_page1[] = {
    // Header
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main            },
    { 27,  6, 28, 10, UI_STYLE_NONE,      ui_proc_menu_pps_label          },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_page_left_inactive },
    { 127, 6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_label_page_1of2    },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_pps_to_page2       },
    // Content
    { 1,  20, 154, 11, UI_STYLE_NONE,      ui_proc_menu_pps_shift_cyc  },
    { 1,  32, 154, 11, UI_STYLE_NONE,      ui_proc_menu_pps_shift_ms   },
    { 1,  44, 154, 11, UI_STYLE_NONE,      ui_proc_menu_pps_sync_cnt   },
    { 60, 64, 95,  15, UI_STYLE_FOCUSABLE, ui_proc_menu_pps_force_sync },
};

static UIScreen ui_pps_screen_page1 = {
    ui_pps_screen_elements_page1,
    NULL,
    ARRAY_SIZE(ui_pps_screen_elements_page1),
    false,
};

// Page 2
static const UIElement ui_pps_screen_elements_page2[] = {
    // Header
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main            },
    { 27,  6, 28, 10, UI_STYLE_NONE,      ui_proc_menu_pps_label          },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_pps_to_page1       },
    { 127, 6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_label_page_2of2     },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_page_right_inactive },
    // Content
    { 1, 20, 154, 11, UI_STYLE_FOCUSABLE,                            ui_proc_menu_pps_sync_on_ppb_lock },
    { 1, 32, 154, 11, UI_STYLE_FOCUSABLE,                            ui_proc_menu_pps_auto_sync        },
    { 1, 44, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_pps_sync_delay       },
    { 1, 56, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_pps_sync_threshold   },
};

static UIScreen ui_pps_screen_page2 = {
    ui_pps_screen_elements_page2,
    &(ui_pps_screen_elements_page2[2]),
    ARRAY_SIZE(ui_pps_screen_elements_page2),
    false,
};


//------------------------------------------------------------------------------
// All Settings Menu Layout
//------------------------------------------------------------------------------
// Header
static void ui_proc_menu_all_settings_label(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_all_settings_to_page1(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_all_settings_to_page2_right(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_all_settings_to_page2_left(const UIElement* element, UICommand command, int32_t encoder_step);
static void ui_proc_menu_all_settings_to_page3(const UIElement* element, UICommand command, int32_t encoder_step);

// Page 1
static const UIElement ui_all_settings_screen_elements_page1[] = {
    // Header
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main                     },
    { 27,  6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_all_settings_label          },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_page_left_inactive          },
    { 127, 6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_label_page_1of3             },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_all_settings_to_page2_right },
    // Content
    { 1,  20, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_main_brightness      },
    { 1,  32, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_gps_module           },
    { 1,  44, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_gps_baud_rate        },
    { 1,  56, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_datetime                  },
    { 1,  68, 154, 11, UI_STYLE_FOCUSABLE,                            ui_proc_pwm                       },
};

static UIScreen ui_all_settings_screen_page1 = {
    ui_all_settings_screen_elements_page1,
    NULL,
    ARRAY_SIZE(ui_all_settings_screen_elements_page1),
    false,
};

// Page 2
static const UIElement ui_all_settings_screen_elements_page2[] = {
    // Header
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main               },
    { 27,  6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_all_settings_label    },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_all_settings_to_page1 },
    { 127, 6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_label_page_2of3       },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_all_settings_to_page3 },
    // Content
    { 1,  20, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_ppb_ocxo             },
    { 1,  32, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_ppb_warmup           },
    { 1,  44, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_ppb_ppb_thr          },
    { 1,  56, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_ppb_alg              },
    { 1,  68, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_ppb_corr_fact        },
};

static UIScreen ui_all_settings_screen_page2 = {
    ui_all_settings_screen_elements_page2,
    &(ui_all_settings_screen_elements_page2[4]),
    ARRAY_SIZE(ui_all_settings_screen_elements_page2),
    false,
};

// Page 3
static const UIElement ui_all_settings_screen_elements_page3[] = {
    // Header
    { 1,   1, 15, 16, UI_STYLE_FOCUSABLE, ui_proc_back_to_main                    },
    { 27,  6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_all_settings_label         },
    { 116, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_all_settings_to_page2_left },
    { 127, 6, 21, 10, UI_STYLE_NONE,      ui_proc_menu_label_page_3of3            },
    { 149, 2, 10, 15, UI_STYLE_FOCUSABLE, ui_proc_menu_page_right_inactive        },
    // Content
    { 1,  20, 23,  11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_out1_drv_str              },
    { 25, 20, 30,  11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_out1_freq                 },
    { 73, 20, 23,  11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_out2_drv_str              },
    { 97, 20, 58,  11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_out2_freq                 },
    { 1,  32, 154, 11, UI_STYLE_FOCUSABLE,                            ui_proc_menu_pps_sync_on_ppb_lock },
    { 1,  44, 154, 11, UI_STYLE_FOCUSABLE,                            ui_proc_menu_pps_auto_sync        },
    { 1,  56, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_pps_sync_delay       },
    { 1,  68, 154, 11, UI_STYLE_FOCUSABLE | UI_STYLE_INPUT_CAPTURING, ui_proc_menu_pps_sync_threshold   },
};

static UIScreen ui_all_settings_screen_page3 = {
    ui_all_settings_screen_elements_page3,
    &(ui_all_settings_screen_elements_page3[2]),
    ARRAY_SIZE(ui_all_settings_screen_elements_page3),
    false,
};


//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
static void ui_menu_draw_right_aligned(const UIElement* element, int offset_chars, const char* str, uint16_t text_color)
{
    size_t len = strlen(str);
    if (len < (UI_MENU_STR_LEN - offset_chars)) {
        ST7735_FillRectangleFast(element->x + offset_chars * 7, element->y + 1, (UI_MENU_STR_LEN - offset_chars - len) * 7, element->height - 1, UI_COLOR_BG);
    }
    ST7735_WriteStringNoWrap(element->x + (UI_MENU_STR_LEN - len) * 7, element->y + 1, element->height - 1, str, Font_7x10, text_color, UI_COLOR_BG);
}

static void ui_proc_icon_navigation_btn(const UIElement* element, UICommand command, int32_t encoder_step,
    uint16_t icon_width, uint16_t icon_height, const uint16_t* icon, UIScreen* target_screen)
{
    if (command & UICommand_Init) {
        ST7735_DrawImage(element->x, element->y, icon_width, icon_height, icon);
    }

    if (command & UICommand_Click) {
        ui_show_screen(target_screen);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_back_to_main(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_icon_navigation_btn(element, command, encoder_step, 15, 16, icon_back_15x16, &ui_main_screen);
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

static void ui_proc_check_box(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, uint8_t* ui_cache, uint8_t* ee_setting)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, label, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw check box
    if ((command & UICommand_Init) || (*ee_setting != *ui_cache)) {
        *ui_cache = *ee_setting;
        ST7735_DrawImage(element->x + 143, element->y, 11, 11, (*ui_cache) ? icon_check_set_11x11 : icon_check_unset_11x11);
    }

    if (command & UICommand_Click) {
        (*ee_setting) = !(*ee_setting);
        ee_is_changed = true;
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_readonly_u32(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, uint32_t* ui_cache, uint32_t value, uint16_t value_offset, const char* fmt)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, label, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw value
    if ((command & UICommand_Init) || (value != *ui_cache)) {
        *ui_cache = value;

        char s[16] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), fmt, value);
        ST7735_WriteStringNoWrap(element->x + value_offset * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_readonly_i32(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, int32_t* ui_cache, int32_t value, uint16_t value_offset, const char* fmt)
{
    ui_proc_menu_readonly_u32(element, command, encoder_step, label, (uint32_t*)ui_cache, (uint32_t)value, value_offset, fmt);
}

static void ui_proc_menu_edit_custom_u32(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, uint32_t* ui_cache, uint32_t* ui_edit, uint32_t *ee_setting, uint16_t value_offset,
    void (*apply_value)(uint32_t), void (*change_value)(uint32_t*, int32_t), void (*format)(char*, size_t, const char*, uint32_t), const char* format_param)
{
    const uint32_t *value_to_draw = NULL;
    bool is_captured = ui_is_captured(element);

    if (command & UICommand_Release) {
        // Apply changes
        if (*ee_setting != *ui_edit) {
            *ee_setting = *ui_edit;
            ee_is_changed = true;

            if (apply_value != NULL) {
                apply_value(*ui_edit);
            }
        }
    }

    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, label, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        if (!is_captured) {
            value_to_draw = ee_setting;
        } else {
            value_to_draw = ui_edit;
        }
    }

    if ((ui_cache != NULL) && !is_captured && (*ee_setting != *ui_cache)) {
        value_to_draw = ee_setting;
    }

    // Value edit
    {
        if (command & UICommand_Capture) {
            *ui_edit = *ee_setting;
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            if (change_value != NULL) {
                change_value(ui_edit, encoder_step);
            }

            // Draw new value
            value_to_draw = ui_edit;
        }
    }

    // Draw value
    if (value_to_draw) {
        if (ui_cache != NULL) {
            *ui_cache = *value_to_draw;
        }

        char s[16] = { '\0' };

        if (format != NULL) {
            format(s, ARRAY_SIZE(s), format_param, *value_to_draw);
        }
        ST7735_WriteStringNoWrap(element->x + value_offset * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_format_u32(char* buf, size_t buf_char_count, const char* fmt, uint32_t value)
{
    snprintf(buf, buf_char_count, fmt, value);
}

static void ui_proc_menu_edit_u32(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, uint32_t* ui_cache, uint32_t* ui_edit, uint32_t *ee_setting, uint16_t value_offset, const char* fmt,
    void (*apply_value)(uint32_t), void (*change_value)(uint32_t*, int32_t))
{
    ui_proc_menu_edit_custom_u32(element, command, encoder_step,
        label, ui_cache, ui_edit, ee_setting, value_offset,
        apply_value, change_value, ui_format_u32, fmt);
}

static void ui_proc_menu_edit_enum_u8(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, uint8_t* ui_edit, uint8_t* ee_setting, uint16_t value_offset, uint8_t max_value,
    void (*before_apply)(uint8_t), const char* (*value_to_string)(uint8_t))
{
    const uint8_t* value_to_draw = NULL;

    if (command & UICommand_Release) {
        // Apply changes
        if (*ee_setting != *ui_edit) {
            if (before_apply != NULL) {
                before_apply(*ui_edit);
            }

            *ee_setting = *ui_edit;
            ee_is_changed = true;
        }
    }

    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, label, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        if (!ui_is_captured(element)) {
            value_to_draw = ee_setting;
        } else {
            value_to_draw = ui_edit;
        }
    }

    // Value selection
    {
        if (command & UICommand_Capture) {
            *ui_edit = *ee_setting;
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            ui_change_setting_u8(ui_edit, encoder_step, 0, max_value, true);

            // Draw new value
            value_to_draw = ui_edit;
        }
    }

    // Draw value
    if (value_to_draw) {
        ui_menu_draw_right_aligned(element, value_offset, value_to_string(*value_to_draw), UI_COLOR_TEXT);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_string_parameter(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, uint32_t* ui_cache_gga, const char* value, size_t value_char_count)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, label, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw the value on every GGA frame to avoid expensive string comparisons
    uint32_t frames = gga_frames;
    if ((command & UICommand_Init) || (frames != *ui_cache_gga)) {
        *ui_cache_gga = frames;
        ui_menu_draw_right_aligned(element, UI_MENU_STR_LEN - value_char_count + 1, value, UI_COLOR_MENU_LABEL);
    }

    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// Main Menu
//------------------------------------------------------------------------------
static void ui_proc_menu(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_icon_navigation_btn(element, command, encoder_step, 14, 16, icon_menu_14x16, &ui_menu_screen_page1);
}


//------------------------------------------------------------------------------
// Save
//------------------------------------------------------------------------------
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

static void ui_proc_save(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static bool ui_cache_ee_is_changed = false;

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


//------------------------------------------------------------------------------
// GPS
//------------------------------------------------------------------------------
static void ui_proc_gps(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static bool    ui_cache_gps_lock_status = false;
    static uint8_t ui_cache_num_sats = 0;

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
static void ui_proc_ppb(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static FrequencyStability ui_cache_frequency_stability = FREQ_STABILITY_UNSTABLE;
    static int32_t            ui_cache_frequency_ppb_x100  = PPB_UNSET_VALUE;

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
static void ui_proc_usb(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static cdc_status ui_cache_cdcio_status = CDC_STATUS_NO_CONN;

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
static void ui_proc_datetime(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static PackedDate ui_cache_gps_date = { .raw = GPS_EMPTY_DATE_TIME };
    static PackedTime ui_cache_gps_time = { .raw = GPS_EMPTY_DATE_TIME };

    if (command & UICommand_Release) {
        // Apply changes
        uint32_t new_time_offset = (uint32_t)(gps_time_offset - GPS_MIN_TIME_OFFSET);
        if (ee_storage.gps_time_offset != new_time_offset) {
            ee_storage.gps_time_offset = new_time_offset;
            ee_is_changed = true;
        }
    }

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
    }

    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// PPS Indicator and Spinner
//------------------------------------------------------------------------------
static void ui_proc_pps(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_spinner_last_update = 0;
    static uint8_t  ui_spinner_frame       = 0;
    static bool     ui_cache_pps_active    = false;

    if (command & UICommand_Init) {
        ui_spinner_last_update = 0;
        ui_spinner_frame       = 0;
    }

    // Draw spinner
    if (timer_is_elapsed(&ui_spinner_last_update, 125)) {
        ST7735_DrawImage(element->x + 9, element->y + 1, 7, 7, icon_spinner_12st_7x7[ui_spinner_frame]);

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

    if (command & UICommand_Click) {
        ui_show_screen(&ui_pps_screen_page1);
    }

    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// Status and Warmup Countdown
//------------------------------------------------------------------------------
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
        msg  = frequency_adjustment_allowed() ? msg_ok : msg_warmup;
        type = UI_MsgBoxType_Ok;
    }

    ui_msgbox(msg, type, UI_MsgBoxButton_Ok, NULL);
}

static void ui_proc_status(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_warmup_remaining_sec = UINT16_MAX;
    static bool     ui_cache_pll_status_error     = false;

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


//------------------------------------------------------------------------------
// Outputs
//------------------------------------------------------------------------------
static const char* ui_drive_strength_to_string(uint8_t drive_strength)
{
    switch (drive_strength) {
    case SI5351_DRIVE_STRENGTH_2MA:
        return "2mA";

    case SI5351_DRIVE_STRENGTH_4MA:
        return "4mA";

    case SI5351_DRIVE_STRENGTH_6MA:
        return "6mA";

    default:
    case SI5351_DRIVE_STRENGTH_8MA:
        return "8mA";
    }
}

static void ui_proc_out_drv_str(const UIElement* element, UICommand command, int32_t encoder_step,
    uint8_t out, const OutFreqConfig *pll_preset, uint8_t *ui_edit_drive_strength, uint8_t *ee_drive_strength)
{
    if (command & UICommand_Release) {
        // Apply changes and configure PLL
        if (*ee_drive_strength != *ui_edit_drive_strength) {
            *ee_drive_strength = *ui_edit_drive_strength;
            ee_is_changed = true;

            pll_configure_output(out, pll_preset, *ee_drive_strength);
        }
    }

    // Drive strength display
    if (!ui_is_captured(element)) {
        // Draw icon and output number
        if ((command & (UICommand_Init | UICommand_Release))) {
            // Draw icon
            const uint16_t *icon = NULL;
            switch (*ee_drive_strength) {
            case SI5351_DRIVE_STRENGTH_2MA:
                icon = icon_out_2ma_7x10;
                break;

            case SI5351_DRIVE_STRENGTH_4MA:
                icon = icon_out_4ma_7x10;
                break;

            case SI5351_DRIVE_STRENGTH_6MA:
                icon = icon_out_6ma_7x10;
                break;

            default:
            case SI5351_DRIVE_STRENGTH_8MA:
                icon = icon_out_8ma_7x10;
                break;
            }

            ST7735_DrawImage(element->x, element->y, 7, 10, icon);

            // Draw output number label
            char s[3] = { '\0' };
            snprintf(s, ARRAY_SIZE(s), "%1u:", out);
            ST7735_WriteStringNoWrap(element->x + 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_OUT_LABEL, UI_COLOR_BG);
        }
    }

    // Drive strength edit
    {
        if (command & UICommand_Capture) {
            *ui_edit_drive_strength = *ee_drive_strength;
        }

        if (command & UICommand_EncoderStep) {
            // Modify setting
            ui_change_setting_u8(ui_edit_drive_strength, encoder_step, 0, SI5351_DRIVE_STRENGTH_8MA, false);
        }

        if (command & (UICommand_Capture | UICommand_RestoreCapture | UICommand_EncoderStep)) {
            // Draw value
            const char* s = ui_drive_strength_to_string(*ui_edit_drive_strength);
            ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_OUT_LABEL, UI_COLOR_BG);
        }
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_out_freq(const UIElement* element, UICommand command, int32_t encoder_step,
    uint8_t out, const OutFreqConfig *pll_presets, uint16_t preset_count, uint16_t *ui_edit_preset, uint16_t *ee_preset, uint8_t drive_strength)
{
    const OutFreqConfig *preset_to_draw = NULL;

    if (command & UICommand_Release) {
        // Apply changes and configure PLL
        if (*ee_preset != *ui_edit_preset) {
            *ee_preset = *ui_edit_preset;
            ee_is_changed = true;

            pll_configure_output(out, &(pll_presets[*ee_preset]), drive_strength);
        }
    }

    if (command & UICommand_Init) {
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
    }

    if (preset_to_draw) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, preset_to_draw->label, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_out1_drv_str(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint8_t ui_edit_pll_out1_drive_strength = SI5351_DRIVE_STRENGTH_2MA;
    ui_proc_out_drv_str(element, command, encoder_step, 1, &(pll_out1_presets[ee_storage.pll_out1_preset]),
        &ui_edit_pll_out1_drive_strength, &(ee_storage.pll_out1_drive_strength));
}

static void ui_proc_out1_freq(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint16_t ui_edit_pll_out1_preset = 0;
    ui_proc_out_freq(element, command, encoder_step, 1, pll_out1_presets, pll_out1_preset_count, &ui_edit_pll_out1_preset,
        &(ee_storage.pll_out1_preset), ee_storage.pll_out1_drive_strength);
}

static void ui_proc_out2_drv_str(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint8_t ui_edit_pll_out2_drive_strength = SI5351_DRIVE_STRENGTH_2MA;
    ui_proc_out_drv_str(element, command, encoder_step, 2, &(pll_out2_presets[ee_storage.pll_out2_preset]),
        &ui_edit_pll_out2_drive_strength, &(ee_storage.pll_out2_drive_strength));
}

static void ui_proc_out2_freq(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint16_t ui_edit_pll_out2_preset = 0;
    ui_proc_out_freq(element, command, encoder_step, 2, pll_out2_presets, pll_out2_preset_count, &ui_edit_pll_out2_preset,
        &(ee_storage.pll_out2_preset), ee_storage.pll_out2_drive_strength);
}


//------------------------------------------------------------------------------
// PWM
//------------------------------------------------------------------------------
static void ui_proc_pwm(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint16_t ui_cache_pwm = 0;

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

static void ui_proc_menu_main_brightness(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint8_t ui_edit_brightness = 0;

    const uint8_t* brightness_to_draw = NULL;

    if (command & UICommand_Release) {
        // Apply changes
        if (ee_storage.brightness != ui_edit_brightness) {
            ee_storage.brightness = ui_edit_brightness;
            ee_is_changed = true;
        }
    }

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
    }

    if (brightness_to_draw) {
        char s[5] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%3u%%", *brightness_to_draw);
        ST7735_WriteStringNoWrap(element->x + 18 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_TEXT, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_main_uptime(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_device_uptime = 0;
    ui_proc_menu_readonly_u32(element, command, encoder_step, "Uptime:", &ui_cache_device_uptime, device_uptime, 10, "%10" PRIu32 " s");
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

static void ui_proc_menu_main_eeprom_writes(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_total_writes = 0;
    ui_proc_menu_readonly_u32(element, command, encoder_step, "EEPROM Writes:", &ui_cache_total_writes, ee_storage.total_writes, 14, "%8" PRIu32);
}

static void ui_proc_menu_main_all_settings(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_FillRectangleFast(element->x, element->y, element->width, element->height, UI_COLOR_BUTTON_BG);
        ST7735_WriteStringNoWrap(element->x + 5, element->y + 3, 10, "All Settings", Font_7x10, UI_COLOR_TEXT, UI_COLOR_BUTTON_BG);
    }

    if (command & UICommand_Click) {
        ui_show_screen(&ui_all_settings_screen_page1);
    }

    ui_default_element_proc(element, command, encoder_step);
}


//------------------------------------------------------------------------------
// World Map
//------------------------------------------------------------------------------
#define UI_MAP_MARKER_BEAM_PX 3

static void ui_world_map_draw_marker(uint16_t base_x, uint16_t base_y, int16_t x, int16_t y, bool erase)
{
    if (x < 0) {
        x = 0;
    } else if (x >= image_world_map_width) {
        x = image_world_map_width - 1;
    }
    if (y < 0) {
        y = 0;
    } else if (y >= image_world_map_height) {
        y = image_world_map_height - 1;
    }

    int16_t start_y = (y >= UI_MAP_MARKER_BEAM_PX) ? y - UI_MAP_MARKER_BEAM_PX : 0;
    int16_t end_y   = (y < image_world_map_height - UI_MAP_MARKER_BEAM_PX) ? y + UI_MAP_MARKER_BEAM_PX : image_world_map_height - 1;

    int16_t start_x = (x >= UI_MAP_MARKER_BEAM_PX)                         ? x - UI_MAP_MARKER_BEAM_PX : 0;
    int16_t end_x   = (x < image_world_map_width - UI_MAP_MARKER_BEAM_PX)  ? x + UI_MAP_MARKER_BEAM_PX : image_world_map_width - 1;
    int16_t width   = end_x - start_x + 1;

    for (int16_t cy = start_y; cy <= end_y; ++cy) {
        if (cy == y) {
            if (erase) {
                // Erase the original image fragment
                size_t left_px_idx = (size_t)cy * image_world_map_width + start_x;
                ST7735_DrawImage(base_x + start_x, base_y + cy, width, 1, image_world_map_160x80 + left_px_idx);
            } else {
                // Draw the horizontal marker line
                ST7735_FillRectangleFast(base_x + start_x, base_y + cy, width, 1, UI_COLOR_MAP_MARKER);
            }
        } else {
            // Draw/erase one pixel of the vertical marker line
            int16_t color = erase ? __REV16(image_world_map_160x80[(size_t)cy * image_world_map_width + x]) : UI_COLOR_MAP_MARKER;
            ST7735_DrawPixel(base_x + x, base_y + cy, color);
        }
    }
}

#define DIV_4096(x) ((x + (1 << 11)) >> 12)

static void ui_proc_world_map(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static int16_t ui_cache_pos_x = INT16_MAX;
    static int16_t ui_cache_pos_y = INT16_MAX;

    if (command & UICommand_Init) {
        ST7735_DrawImage(element->x, element->y, image_world_map_width, image_world_map_height, image_world_map_160x80);
    }

    int16_t pos_x = (gps_longitude_deg_x10M == PPB_EMPTY_DEG_X10M_COORD)
        ? INT16_MAX
        : (int16_t)(DIV_4096((uint32_t)((int64_t)gps_longitude_deg_x10M + 1800'000'000LL)) * image_world_map_width / DIV_4096(3600'000'000u));
    int16_t pos_y = (gps_latitude_deg_x10M == PPB_EMPTY_DEG_X10M_COORD)
        ? INT16_MAX
        : (int16_t)(DIV_4096((uint32_t)(900'000'000 - gps_latitude_deg_x10M)) * image_world_map_height / DIV_4096(1800'000'000u));

    if ((command & UICommand_Init) || (pos_x != ui_cache_pos_x) || (pos_y != ui_cache_pos_y)) {
        // Erase old mark
        if (ui_cache_pos_x != INT16_MAX && ui_cache_pos_y != INT16_MAX) {
            ui_world_map_draw_marker(element->x, element->y, ui_cache_pos_x, ui_cache_pos_y, true);
        }
        // Draw new mark
        if (pos_x != INT16_MAX && pos_y != INT16_MAX) {
            ui_world_map_draw_marker(element->x, element->y, pos_x, pos_y, false);
        }

        ui_cache_pos_x = pos_x;
        ui_cache_pos_y = pos_y;
    }

    if (command & UICommand_Click) {
        ui_show_screen(&ui_gps_screen_page2);
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

static void ui_draw_coord_dd_right_aligned(const UIElement* element, int32_t coord_deg_x10M)
{
    char s[13] = { '\0' };

    if (coord_deg_x10M != PPB_EMPTY_DEG_X10M_COORD) {
        int32_t coord_abs   = (coord_deg_x10M < 0) ? -coord_deg_x10M : coord_deg_x10M;
        int32_t coord_int   = coord_abs / 10000000;
        int32_t coord_fract = coord_abs % 10000000;

        const char* sign = (coord_deg_x10M < 0) ? "-" : "";
        snprintf(s, ARRAY_SIZE(s), "%s%" PRIi32 ".%07" PRIi32, sign, coord_int, coord_fract);
    }

    uint16_t len    = (uint16_t)strlen(s);
    uint16_t offset = UI_MENU_STR_LEN - len;

    if (len < 12) {
        uint16_t empty_chars = 12 - len;
        ST7735_FillRectangleFast(element->x + 10 * 7, element->y + 1, empty_chars * 7, element->height - 1, UI_COLOR_BG);
    }

    ST7735_WriteStringNoWrap(element->x + offset * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
}

static void ui_draw_coord_ddm_right_aligned(const UIElement* element, const char* nmea_coord, char coord_ns_ew)
{
    char      min_str[20] = { 0 };
    char      deg_buf[4]  = { '\0' };
    char*     deg_str     = deg_buf;
    size_t    min_len     = 0;
    ptrdiff_t deg_len     = 0;

    // Find the decimal point separating whole minutes and fractional minutes
    const char* dot = strchr(nmea_coord, '.');
    if (dot != NULL && (dot - nmea_coord >= 2)) {
        // Minutes always occupy exactly two numeric characters immediately preceding the decimal point
        const char* min_start = dot - 2;

        // Format minutes and 'N/S'/'E/W' string
        snprintf(min_str, ARRAY_SIZE(min_str), " %s' %c", min_start, coord_ns_ew);
        min_len = strlen(min_str);

        // Format degrees
        deg_len = min_start - nmea_coord;
        if (deg_len < (ptrdiff_t)ARRAY_SIZE(deg_buf)) {
            strncpy(deg_str, nmea_coord, deg_len);
            deg_str[deg_len] = '\0';

            // Remove leading zeros
            while ((*deg_str == '0') && (deg_len > 1)) {
                ++deg_str;
                --deg_len;
            }
        } else {
            deg_len = 0;
        }
    }

    // Draw minutes
    uint16_t pos = UI_MENU_STR_LEN;
    if (pos >= min_len && min_len > 0) {
        pos -= min_len;
        ST7735_WriteStringNoWrap(element->x + pos * 7, element->y + 1, element->height - 1, min_str, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw degrees
    if (deg_len > 0) {
        // Draw degree sign
        if (pos >= 1) {
            --pos;
            ST7735_DrawImage(element->x + pos * 7, element->y + 1, 7, 10, icon_symbol_degree_7x10);
        }

        // Draw value
        if (pos >= deg_len) {
            pos -= deg_len;
            ST7735_WriteStringNoWrap(element->x + pos * 7, element->y + 1, element->height - 1, deg_str, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
        }
    }

    // Clear remaining space
    if (pos > 0) {
        ST7735_FillRectangleFast(element->x, element->y + 1, pos * 7, element->height - 1, UI_COLOR_BG);
    }
}

static void ui_proc_menu_gps_coordinate_dd(const UIElement* element, UICommand command, int32_t encoder_step, const char* label, int32_t *ui_cache, int32_t coord)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, label, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw value
    if ((command & UICommand_Init) || (coord != *ui_cache)) {
        *ui_cache = coord;
        ui_draw_coord_dd_right_aligned(element, coord);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_gps_coordinate_ddm(const UIElement* element, UICommand command, int32_t encoder_step,
    uint32_t* ui_cache_gga, const char* coord_str, char coord_ns_ew)
{
    // Draw the coordinate on every GGA frame to avoid expensive string comparisons
    uint32_t frames = gga_frames;
    if ((command & UICommand_Init) || (frames != *ui_cache_gga)) {
        *ui_cache_gga = frames;
        ui_draw_coord_ddm_right_aligned(element, coord_str, coord_ns_ew);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_gps_latitude_dd(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static int32_t ui_cache_gps_latitude_deg_x10M = PPB_EMPTY_DEG_X10M_COORD;
    ui_proc_menu_gps_coordinate_dd(element, command, encoder_step, "Latitude:", &ui_cache_gps_latitude_deg_x10M, gps_latitude_deg_x10M);
}

static void ui_proc_menu_gps_latitude_ddm(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_gga_frames = 0;
    char ns = gps_latitude_deg_x10M > 0 ? 'N' : 'S';
    ui_proc_menu_gps_coordinate_ddm(element, command, encoder_step, &ui_cache_gga_frames, gps_latitude_str, ns);
}

static void ui_proc_menu_gps_longitude_dd(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static int32_t ui_cache_gps_longitude_deg_x10M = PPB_EMPTY_DEG_X10M_COORD;
    ui_proc_menu_gps_coordinate_dd(element, command, encoder_step, "Longitude:", &ui_cache_gps_longitude_deg_x10M, gps_longitude_deg_x10M);
}

static void ui_proc_menu_gps_longitude_ddm(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_gga_frames = 0;
    char ew = gps_longitude_deg_x10M > 0 ? 'E' : 'W';
    ui_proc_menu_gps_coordinate_ddm(element, command, encoder_step, &ui_cache_gga_frames, gps_longitude_str, ew);
}

static void ui_proc_menu_gps_altitude(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_gga_frames = 0;
    ui_proc_menu_string_parameter(element, command, encoder_step, "Altitude:", &ui_cache_gga_frames, gps_msl_altitude_str, ARRAY_SIZE(gps_msl_altitude_str));
}

static void ui_proc_menu_gps_locator(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_gga_frames = 0;
    ui_proc_menu_string_parameter(element, command, encoder_step, "Locator:", &ui_cache_gga_frames, gps_locator, ARRAY_SIZE(gps_locator));
}

static void ui_proc_menu_gps_hdop(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_gga_frames = 0;
    ui_proc_menu_string_parameter(element, command, encoder_step, "HDOP:", &ui_cache_gga_frames, gps_hdop_str, ARRAY_SIZE(gps_hdop_str));
}

static void ui_proc_menu_gps_geoid_sep(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_gga_frames = 0;
    ui_proc_menu_string_parameter(element, command, encoder_step, "Geoid Sep.:", &ui_cache_gga_frames, gps_geoid_separation_str, ARRAY_SIZE(gps_geoid_separation_str));
}

static void ui_proc_menu_gps_map(const UIElement* element, UICommand command, int32_t encoder_step)
{
    if (command & UICommand_Init) {
        ST7735_FillRectangleFast(element->x, element->y, element->width, element->height, UI_COLOR_BUTTON_BG);
        ST7735_WriteStringNoWrap(element->x + 37, element->y + 3, 10, "Map", Font_7x10, UI_COLOR_TEXT, UI_COLOR_BUTTON_BG);
    }

    if (command & UICommand_Click) {
        ui_show_screen(&ui_world_map_screen);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_apply_gps_module_model(uint8_t model)
{
    if ((model == GPS_MODEL_ATGM336H) && (ee_storage.gps_baudrate > 115200)) {
        ee_storage.gps_baudrate = 115200;
        gps_setbaudrate(ee_storage.gps_baudrate);
    }
}

static void ui_proc_menu_gps_module(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint8_t ui_edit_gps_model = 0;
    ui_proc_menu_edit_enum_u8(element, command, encoder_step, "Module:",
        &ui_edit_gps_model, &(ee_storage.gps_model), 14, GPS_MODEL_MAX,
        ui_apply_gps_module_model, gps_model_type_to_string);
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

    if ((ee_storage.gps_model == GPS_MODEL_ATGM336H) && b > 115200) {
        b = 115200;
    }

    *baudrate = b;
}

static void ui_proc_menu_gps_baud_rate(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_gps_baudrate = 0;
    static uint32_t ui_edit_gps_baudrate = 0;
    ui_proc_menu_edit_u32(element, command, encoder_step, "Baud Rate:",
        &ui_cache_gps_baudrate, &ui_edit_gps_baudrate, &(ee_storage.gps_baudrate), 16, "%6" PRIu32,
        gps_setbaudrate, ui_change_baudrate);
}

static void ui_proc_menu_gps_gga_frames(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_gga_frames = 0;
    ui_proc_menu_readonly_u32(element, command, encoder_step, "GGA Frames:", &ui_cache_gga_frames, gga_frames, 12, "%10" PRIu32);
}

static void ui_proc_menu_gps_errors(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_gps_invalid_frames     = 0;
    static uint32_t ui_cache_gps_fifo_overflow_gps  = 0;
    static uint32_t ui_cache_gps_fifo_overflow_comm = 0;

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

static void ui_proc_menu_ppb_9char(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, int32_t *ui_cache, int32_t ppb_x100)
{
    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, label, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw value
    if ((command & UICommand_Init) || (ppb_x100 != *ui_cache)) {
        *ui_cache = ppb_x100;

        char s[10] = { '\0' };
        ui_format_ppb_9char(ppb_x100, s, ARRAY_SIZE(s));
        ST7735_WriteStringNoWrap(element->x + 13 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_ppb_mean(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static int32_t ui_cache_frequency_ppb_x100 = PPB_UNSET_VALUE;
    ui_proc_menu_ppb_9char(element, command, encoder_step, "Mean PPB:", &ui_cache_frequency_ppb_x100, frequency_ppb_x100);
}

static void ui_proc_menu_ppb_inst(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static int32_t ui_cache_frequency_inst_ppb_x100 = PPB_UNSET_VALUE;
    ui_proc_menu_ppb_9char(element, command, encoder_step, "Inst. PPB:", &ui_cache_frequency_inst_ppb_x100, frequency_get_inst_ppb_x100());
}

static void ui_proc_menu_ppb_freq(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static int32_t ui_cache_ppb_frequency = 0;

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
            char s[12] = { '\0' };
            snprintf(s, ARRAY_SIZE(s), "%11" PRIi32, freq);
            ST7735_WriteStringNoWrap(element->x + 11 * 7, element->y + 1, element->height - 1, s, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
        }
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_ppb_pps_err(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static int32_t ui_cache_ppb_millis = 0;
    int32_t millis = ui_limit_i32(ppb_millis, -9999999, 9999999);
    ui_proc_menu_readonly_i32(element, command, encoder_step, "PPS Error:", &ui_cache_ppb_millis, millis, 11, "%8" PRIi32 " ms");
}

static void ui_proc_menu_ppb_pwm_corr(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint16_t ui_cache_pwm            = 0;
    static int32_t  ui_cache_ppb_correction = 0;
    static bool     ui_cache_warmup_done    = false;

    // Draw label
    if (command & UICommand_Init) {
        ST7735_WriteStringNoWrap(element->x, element->y + 1, element->height - 1, "PWM/Corr.:", Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);
    }

    // Draw value
    uint16_t pwm = TIM1->CCR2;
    int32_t  corr = ppb_correction;
    bool     warmup_done = frequency_adjustment_allowed();
    if ((command & UICommand_Init) || (pwm != ui_cache_pwm) || (corr != ui_cache_ppb_correction) || (warmup_done != ui_cache_warmup_done)) {
        ui_cache_pwm            = pwm;
        ui_cache_ppb_correction = corr;
        ui_cache_warmup_done    = warmup_done;

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

static void ui_apply_ocxo_model(uint8_t model)
{
    // Set warm-up time for the selected OCXO
    ee_storage.warmup_time_seconds = get_default_warmup_time(model);
}

static void ui_proc_menu_ppb_ocxo(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint8_t ui_edit_ocxo_model = 0;
    ui_proc_menu_edit_enum_u8(element, command, encoder_step, "OCXO:",
        &ui_edit_ocxo_model, &(ee_storage.ocxo_model), 15, OCXO_MODEL_MAX,
        ui_apply_ocxo_model, ocxo_model_type_to_string);
}

static void ui_change_ppb_warmup_time(uint32_t* value, int32_t step)
{
    ui_change_setting_u32(value, step, 0, 1800, false);
}

static void ui_proc_menu_ppb_warmup(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_warmup_time_seconds = 0;
    static uint32_t ui_edit_warmup_time_seconds = 0;
    ui_proc_menu_edit_u32(element, command, encoder_step, "Warm-up Time:",
        &ui_cache_warmup_time_seconds, &ui_edit_warmup_time_seconds, &(ee_storage.warmup_time_seconds), 14, "%6" PRIu32 " s",
        NULL, ui_change_ppb_warmup_time);
}

static void ui_change_ppb_ppb_thr(uint32_t* value, int32_t step)
{
    ui_change_setting_u32(value, 5*step, 0, MAX_PPB_LOCK_THRESHOLD, false);
}

static void ui_format_ppb_thr(char* buf, size_t buf_char_count, const char*, uint32_t value)
{
    snprintf(buf, buf_char_count, "%3" PRIu32 ".%02" PRIu32, value / 100, value % 100);
}

static void ui_proc_menu_ppb_ppb_thr(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_edit_ppb_lock_threshold = 0;
    ui_proc_menu_edit_custom_u32(element, command, encoder_step,
        "PPB Lock Thr.:", NULL, &ui_edit_ppb_lock_threshold, &(ee_storage.ppb_lock_threshold), 16,
        NULL, ui_change_ppb_ppb_thr, ui_format_ppb_thr, NULL);
}

static void ui_apply_ppb_alg(uint8_t algorithm)
{
    // Reset correction factor to default value when algorithm is changed
    ee_storage.correction_factor = get_default_correction_factor(algorithm);
}

static void ui_proc_menu_ppb_alg(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint8_t ui_edit_correction_algorithm = 0;
    ui_proc_menu_edit_enum_u8(element, command, encoder_step, "Algorithm:",
        &ui_edit_correction_algorithm, &(ee_storage.correction_algorithm), 15, CORRECTION_ALGO_MAX,
        ui_apply_ppb_alg, correction_algo_type_to_string);
}

static void ui_change_ppb_corr_fact(uint32_t* value, int32_t step)
{
    increment_correction_factor_value(value, ee_storage.correction_algorithm, step);
}

static void ui_proc_menu_ppb_corr_fact(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_correction_factor = 0;
    static uint32_t ui_edit_correction_factor = 0;
    ui_proc_menu_edit_u32(element, command, encoder_step, "Corr. Factor:",
        &ui_cache_correction_factor, &ui_edit_correction_factor, &(ee_storage.correction_factor), 14, "%8" PRIu32,
        NULL, ui_change_ppb_corr_fact);
}


//------------------------------------------------------------------------------
// PPS Menu Procedures
//------------------------------------------------------------------------------
static void ui_proc_menu_pps_label(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_label(element, command, encoder_step, "PPS");
}

static void ui_proc_menu_pps_to_page1(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_left(element, command, encoder_step, &ui_pps_screen_page1);
}

static void ui_proc_menu_pps_to_page2(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_right(element, command, encoder_step, &ui_pps_screen_page2);
}

static void ui_proc_menu_pps_shift_cyc(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static int32_t ui_cache_pps_error = 0;
    ui_proc_menu_readonly_i32(element, command, encoder_step, "Shift:", &ui_cache_pps_error, pps_error, 7, "%11" PRIi32 " cyc");
}

static void ui_proc_menu_pps_shift_ms(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static int32_t ui_cache_pps_millis = 0;

    // Draw value
    int32_t err = pps_millis;
    if ((command & UICommand_Init) || (err != ui_cache_pps_millis)) {
        ui_cache_pps_millis = err;

        uint32_t err_abs = (err < 0) ? -(uint32_t)err : (uint32_t)err;
        char sign = err < 0 ? '-' : ' ';

        char s[16] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), "%c%" PRIu32 ".%04" PRIu32 " ms", sign, err_abs / 10000, err_abs % 10000);
        ui_menu_draw_right_aligned(element, 7, s, UI_COLOR_MENU_LABEL);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_pps_sync_cnt(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_cache_pps_sync_count = 0;
    ui_proc_menu_readonly_u32(element, command, encoder_step, "Sync. Count:", &ui_cache_pps_sync_count, pps_sync_count, 12, "%10" PRIu32);
}

static void ui_proc_menu_pps_force_sync(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static bool ui_cache_sync_pps_out = false;

    if ((command & UICommand_Init) || (sync_pps_out != ui_cache_sync_pps_out)) {
        ui_cache_sync_pps_out = sync_pps_out;

        // If 'sync_pps_out' is already true, display the inactive button
        uint16_t color_text = ui_cache_sync_pps_out ? UI_COLOR_INACTIVE_TEXT : UI_COLOR_TEXT;
        uint16_t color_bg   = ui_cache_sync_pps_out ? UI_COLOR_INACTIVE_BUTTON_BG : UI_COLOR_BUTTON_BG;

        ST7735_FillRectangleFast(element->x, element->y, element->width, element->height, color_bg);
        ST7735_WriteStringNoWrap(element->x + 9, element->y + 3, 10, "Force Sync.", Font_7x10, color_text, color_bg);
    }

    if (command & UICommand_Click) {
        sync_pps_out = true;
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_pps_sync_on_ppb_lock(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint8_t ui_cache_pps_sync_on_ppb_lock = 0;
    ui_proc_check_box(element, command, encoder_step, "Sync. on PPB Lock:", &ui_cache_pps_sync_on_ppb_lock, &(ee_storage.pps_sync_on_ppb_lock));
}

static void ui_proc_menu_pps_auto_sync(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint8_t ui_cache_pps_auto_sync = 0;
    ui_proc_check_box(element, command, encoder_step, "Auto Sync.:", &ui_cache_pps_auto_sync, &(ee_storage.pps_auto_sync));
}

static void ui_proc_menu_auto_sync_param_edit(const UIElement* element, UICommand command, int32_t encoder_step,
    const char* label, uint8_t* ui_cache_auto_sync, uint32_t* ui_edit, uint32_t *ee_setting, uint32_t max_value, uint16_t value_offset, const char* fmt)
{
    const uint32_t *value_to_draw = NULL;
    bool is_captured = ui_is_captured(element);

    if (command & UICommand_Release) {
        // Apply changes
        if (*ee_setting != *ui_edit) {
            *ee_setting = *ui_edit;
            ee_is_changed = true;
        }
    }

    if (command & UICommand_Init) {
        // Draw label
        ST7735_WriteStringNoWrap(element->x + 2 * 7, element->y + 1, element->height - 1, label, Font_7x10, UI_COLOR_MENU_LABEL, UI_COLOR_BG);

        // Draw value
        if (!is_captured) {
            value_to_draw = ee_setting;
        } else {
            value_to_draw = ui_edit;
        }
    }

    if (!is_captured && (ee_storage.pps_auto_sync != *ui_cache_auto_sync)) {
        value_to_draw = ee_setting;
    }

    // Value edit
    {
        if (command & UICommand_Capture) {
            *ui_edit = *ee_setting;
        }

        if ((command & UICommand_EncoderStep) && ee_storage.pps_auto_sync) {
            // Modify setting
            ui_change_setting_u32(ui_edit, ui_get_adaptive_step(*ui_edit, encoder_step), 0, max_value, false);

            // Draw new value
            value_to_draw = ui_edit;
        }
    }

    // Draw value
    if (value_to_draw) {
        *ui_cache_auto_sync = ee_storage.pps_auto_sync;

        uint16_t text_color = (*ui_cache_auto_sync) ? UI_COLOR_TEXT : UI_COLOR_INACTIVE_TEXT;
        char s[16] = { '\0' };
        snprintf(s, ARRAY_SIZE(s), fmt, *value_to_draw);
        ST7735_WriteStringNoWrap(element->x + value_offset * 7, element->y + 1, element->height - 1, s, Font_7x10, text_color, UI_COLOR_BG);
    }

    ui_default_element_proc(element, command, encoder_step);
}

static void ui_proc_menu_pps_sync_delay(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_edit_pps_sync_delay = 0;
    static uint8_t  ui_cache_pps_auto_sync = 0;
    ui_proc_menu_auto_sync_param_edit(element, command, encoder_step, "Delay:",
        &ui_cache_pps_auto_sync, &ui_edit_pps_sync_delay, &(ee_storage.pps_sync_delay), 950000, 14, "%6" PRIu32 " s");
}

static void ui_proc_menu_pps_sync_threshold(const UIElement* element, UICommand command, int32_t encoder_step)
{
    static uint32_t ui_edit_pps_sync_threshold = 0;
    static uint8_t  ui_cache_pps_auto_sync     = 0;
    ui_proc_menu_auto_sync_param_edit(element, command, encoder_step, "Thr.:",
        &ui_cache_pps_auto_sync, &ui_edit_pps_sync_threshold, &(ee_storage.pps_sync_threshold), 950000000, 9, "%9" PRIu32 " cyc");
}


//------------------------------------------------------------------------------
// All Settings Menu Layout
//------------------------------------------------------------------------------
static void ui_proc_menu_all_settings_label(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_label(element, command, encoder_step, "Settings");
}

static void ui_proc_menu_all_settings_to_page1(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_left(element, command, encoder_step, &ui_all_settings_screen_page1);
}

static void ui_proc_menu_all_settings_to_page2_right(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_right(element, command, encoder_step, &ui_all_settings_screen_page2);
}

static void ui_proc_menu_all_settings_to_page2_left(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_left(element, command, encoder_step, &ui_all_settings_screen_page2);
}

static void ui_proc_menu_all_settings_to_page3(const UIElement* element, UICommand command, int32_t encoder_step)
{
    ui_proc_menu_page_right(element, command, encoder_step, &ui_all_settings_screen_page3);
}
