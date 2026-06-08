#ifndef _UI_CORE_8DE9180F8ED6_H_
#define _UI_CORE_8DE9180F8ED6_H_

#include <stdint.h>
#include <stdbool.h>

#define UI_COLOR_BG            (ST7735_BLACK)
#define UI_COLOR_BUTTON_BG     (ST7735_COLOR565(120, 120, 120))
#define UI_COLOR_FOCUS_FRAME   (ST7735_COLOR565(195, 205, 205))
#define UI_COLOR_CAPTURE_FRAME (ST7735_COLOR565(255, 115, 21))
#define UI_COLOR_TREND         (ST7735_COLOR565(250, 170, 80))
#define UI_COLOR_TEXT          (ST7735_WHITE)

typedef enum {
    UICommand_None           = 0x00000000,
    UICommand_Init           = 0x00000001, // The screen containing the element is activated
    UICommand_Focus          = 0x00000002, // The element gains focus
    UICommand_LostFocus      = 0x00000004, // The element loses focus
    UICommand_RestoreFocus   = 0x00000008, // The screen containing an already focused element is activated
    UICommand_Capture        = 0x00000010, // The element captures control
    UICommand_Release        = 0x00000020, // The element releases control
    UICommand_RestoreCapture = 0x00000040, // The screen containing an element that has already captured control is activated
    UICommand_EncoderStep    = 0x00000080, // The encoder is rotated while the element has captured control
    UICommand_Click          = 0x00000100, // The focused element receives a click
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

extern void ui_show_screen(UIScreen* screen);
extern void ui_run();

#endif
