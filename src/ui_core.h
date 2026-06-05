#ifndef _UI_CORE_8DE9180F8ED6_H_
#define _UI_CORE_8DE9180F8ED6_H_

#include <stdint.h>
#include <stdbool.h>

#define UI_BG_COLOR            (ST7735_BLACK)
#define UI_FOCUS_FRAME_COLOR   (ST7735_COLOR565(195, 205, 205))
#define UI_CAPTURE_FRAME_COLOR (ST7735_COLOR565(255, 115, 21))

typedef enum {
    UICommand_None      = 0x00000000,
    UICommand_Init      = 0x00000001,
    UICommand_Focus     = 0x00000004,
    UICommand_LostFocus = 0x00000008,
    UICommand_Capture   = 0x00000010,
    UICommand_Release   = 0x00000020,
    UICommand_EncStep   = 0x00000040,
    UICommand_Click     = 0x00000080,
} UICommand;

#define UI_STYLE_NONE            0x0000U
#define UI_STYLE_FOCUSABLE       0x0001U
#define UI_STYLE_INPUT_CAPTURING 0x0002U


#define UI_FOCUSED_ELEMENT_IDX_NONE -1

struct UIElement;
typedef void (*UI_ElementProc)(const struct UIElement* element, UICommand command, int32_t encoder_step);

typedef struct UIElement {
    const uint16_t x;
    const uint16_t y;
    const uint16_t width;
    const uint16_t height;

    const uint16_t       styles;
    const UI_ElementProc proc;
} UIElement;

typedef struct {
    const UIElement* const elements;
    const int32_t          num_elements;
    int16_t                focused_element_idx;
    bool                   is_input_captured;
} UIScreen;

extern void ui_default_element_proc(const struct UIElement* element, UICommand command, int32_t encoder_step);

extern void ui_init(UIScreen *screen);
extern void ui_run();

#endif
