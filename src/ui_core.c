#include "ui_core.h"
#include "main.h"
#include "encoder.h"
#include "st7735.h"
#include "st7735_config.h"


static UIScreen* ui_current_screen = NULL;


void ui_default_element_proc(const struct UIElement* element, UICommand command, int32_t encoder_step)
{
    // Draw/clear frame
    if ((element->styles & UI_STYLE_FOCUSABLE) &&
        (command & (UICommand_Focus | UICommand_LostFocus | UICommand_Capture | UICommand_Release)))
    {
        uint16_t frame_color;

        if (command & (UICommand_Focus | UICommand_Release)) {
            // If an element releases control, it remains focused
            frame_color = UI_FOCUS_FRAME_COLOR;
        } else if (command & UICommand_Capture) {
            frame_color = UI_CAPTURE_FRAME_COLOR;
        } else {
            frame_color = UI_BG_COLOR;
        }

        ST7735_FillRectangle(element->x-1,              element->y-1,               1,                element->height+2, frame_color);
        ST7735_FillRectangle(element->x-1,              element->y-1,               element->width+2, 1,                 frame_color);
        ST7735_FillRectangle(element->x+element->width, element->y-1,               1,                element->height+2, frame_color);
        ST7735_FillRectangle(element->x-1,              element->y+element->height, element->width+2, 1,                 frame_color);
    }
}

void ui_init(UIScreen* screen)
{
    ST7735_FillRectangleFast(0, 0, ST7735_WIDTH, ST7735_HEIGHT, UI_BG_COLOR);
    ui_current_screen = screen;

    for (int32_t i = 0; i < screen->num_elements; ++i)
    {
        const UIElement *element = &(screen->elements[i]);
        UICommand        command = UICommand_Init;

        if ((screen->focused_element_idx == UI_FOCUSED_ELEMENT_IDX_NONE) && (element->styles & UI_STYLE_FOCUSABLE)) {
            screen->focused_element_idx = i;
            command |= UICommand_Focus;
        }

        element->proc(element, command, 0);
    }
}

static bool ui_is_element_focused()
{
    return ui_current_screen != NULL &&
        ui_current_screen->focused_element_idx >= 0 &&
        ui_current_screen->focused_element_idx < ui_current_screen->num_elements;
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

    if (ui_is_element_focused()) {
        if (click) {
            const UIElement* element = &(ui_current_screen->elements[ui_current_screen->focused_element_idx]);
            if (element->styles & UI_STYLE_INPUT_CAPTURING) {
                // Capture/release input on click
                UICommand command = ui_current_screen->is_input_captured
                    ? UICommand_Release
                    : UICommand_Capture;
                element->proc(element, command, 0);
                ui_current_screen->is_input_captured = !ui_current_screen->is_input_captured;
            } else {
                // Send click command to non-capturing element
                element->proc(element, UICommand_Click, 0);
            }
        } else if (ui_current_screen->is_input_captured && step != 0) {
            // Send step command to the element that captured input
            const UIElement* element = &(ui_current_screen->elements[ui_current_screen->focused_element_idx]);
            element->proc(element, UICommand_EncStep, step);
        }
    }

    // Scroll through controls
    if (!click && step != 0 && !ui_current_screen->is_input_captured)
    {
        // Find the next element to focus
        int16_t next_element_idx = ui_current_screen->focused_element_idx;
        do
        {
            next_element_idx += step;
            if (next_element_idx < 0) {
                next_element_idx = ui_current_screen->num_elements - 1;
            }
            if (next_element_idx >= ui_current_screen->num_elements) {
                next_element_idx = 0;
            }
        } while (next_element_idx != ui_current_screen->focused_element_idx &&
                 ((ui_current_screen->elements[next_element_idx].styles & UI_STYLE_FOCUSABLE) == 0));

        // Change focused element
        if (next_element_idx != ui_current_screen->focused_element_idx) {
            if (ui_is_element_focused()) {
                const UIElement *focused_element = &(ui_current_screen->elements[ui_current_screen->focused_element_idx]);
                focused_element->proc(focused_element, UICommand_LostFocus, 0);
            }

            ui_current_screen->focused_element_idx = next_element_idx;
            const UIElement* next_element = &(ui_current_screen->elements[next_element_idx]);
            next_element->proc(next_element, UICommand_Focus, 0);
        }
    }

    // Update all controls
    for (int i = 0; i < ui_current_screen->num_elements; ++i) {
        const UIElement* element = &(ui_current_screen->elements[i]);
        element->proc(element, UICommand_None, step);
    }
}
