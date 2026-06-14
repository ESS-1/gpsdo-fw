#include "ui_core.h"
#include "main.h"
#include "encoder.h"
#include "fonts.h"
#include "st7735.h"
#include "st7735_config.h"

static UIScreen* ui_current_screen = NULL;

void ui_default_element_proc(const UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw/clear frame
    if (command & (UICommand_Focus | UICommand_LostFocus | UICommand_RestoreFocus | UICommand_Capture | UICommand_Release | UICommand_RestoreCapture))
    {
        uint16_t frame_color;

        if (command & (UICommand_Focus | UICommand_Release | UICommand_RestoreFocus)) {
            // If an element releases control, it remains focused
            frame_color = UI_COLOR_FOCUS_FRAME;
        } else if (command & (UICommand_Capture | UICommand_RestoreCapture)) {
            frame_color = UI_COLOR_CAPTURE_FRAME;
        } else {
            frame_color = UI_COLOR_BG;
        }

        uint16_t x1 = element->x - 1;
        uint16_t y1 = element->y - 1;
        uint16_t w  = element->width + 2;
        uint16_t h  = element->height + 2;

        ST7735_FillRectangle    (x1,     y1,     1, h, frame_color);
        ST7735_FillRectangleFast(x1,     y1,     w, 1, frame_color);
        ST7735_FillRectangle    (x1+w-1, y1,     1, h, frame_color);
        ST7735_FillRectangleFast(x1,     y1+h-1, w, 1, frame_color);
    }
}

bool ui_is_captured(const UIElement* element)
{
    return (ui_current_screen != NULL) && ui_current_screen->is_input_captured && (ui_current_screen->focused_element == element);
}

void ui_show_screen(UIScreen* screen)
{
    ST7735_FillRectangleFast(0, 0, ST7735_WIDTH, ST7735_HEIGHT, UI_COLOR_BG);
    ui_current_screen = screen;

    for (int32_t i = 0; i < screen->num_elements; ++i)
    {
        const UIElement *element = &(screen->elements[i]);
        UICommand        command = UICommand_Init;

        if ((screen->focused_element == NULL) && (element->styles & UI_STYLE_FOCUSABLE)) {
            screen->focused_element = element;
            command |= UICommand_Focus;
        } else if (screen->focused_element == element) {
            command |= (screen->is_input_captured ? UICommand_RestoreCapture : UICommand_RestoreFocus);
        }

        element->proc(element, command, 0);
    }
}

void ui_run()
{
    if (ui_current_screen == NULL) {
        return;
    }

    // Read encoder
    bool    click = encoder_get_click();
    int32_t step  = encoder_get_step();
    step          = (step > 0) - (step < 0);

    if (ui_current_screen->focused_element != NULL) {
        if (click) {
            const UIElement* element = ui_current_screen->focused_element;
            if (element->styles & UI_STYLE_INPUT_CAPTURING) {
                // Capture/release input on click
                UICommand command = ui_current_screen->is_input_captured
                    ? UICommand_Release
                    : UICommand_Capture;
                ui_current_screen->is_input_captured = !ui_current_screen->is_input_captured;
                element->proc(element, command, 0);
            } else {
                // Send click command to non-capturing element
                element->proc(element, UICommand_Click, 0);
            }
        } else if (ui_current_screen->is_input_captured && step != 0) {
            // Send step command to the element that captured input
            const UIElement* element = ui_current_screen->focused_element;
            element->proc(element, UICommand_EncoderStep, step);
        }
    }

    // Scroll through controls
    if (!click && step != 0 && !ui_current_screen->is_input_captured)
    {
        int32_t current_idx = (ui_current_screen->focused_element != NULL)
            ? ui_current_screen->focused_element - ui_current_screen->elements
            : -1;

        // Find the next element to focus
        int32_t next_element_idx = current_idx;
        do
        {
            next_element_idx += step;
            if (next_element_idx < 0) {
                next_element_idx = ui_current_screen->num_elements - 1;
            }
            if (next_element_idx >= ui_current_screen->num_elements) {
                next_element_idx = 0;
            }
        } while (next_element_idx != current_idx &&
                 ((ui_current_screen->elements[next_element_idx].styles & UI_STYLE_FOCUSABLE) == 0));

        // Change focused element
        if (next_element_idx != current_idx) {
            const UIElement* focused_element = ui_current_screen->focused_element;
            const UIElement* next_element    = &(ui_current_screen->elements[next_element_idx]);

            ui_current_screen->focused_element = next_element;

            if (focused_element != NULL) {
                focused_element->proc(focused_element, UICommand_LostFocus, 0);
            }

            next_element->proc(next_element, UICommand_Focus, 0);
        }
    }

    // Update all controls
    for (int i = 0; i < ui_current_screen->num_elements; ++i) {
        const UIElement* element = &(ui_current_screen->elements[i]);
        element->proc(element, UICommand_None, step);
    }
}
