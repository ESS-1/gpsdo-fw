#include "main.h"
#include "encoder.h"

// All times in ms
#define DEBOUNCE_TIME           50

#define ENCODER_INVERT

static volatile uint32_t encoder_down_time      = 0;
static volatile uint32_t encoder_up_time        = 0;
static volatile bool     encoder_press_detected = 0;

static uint32_t          encoder_last_value     = 0;

static bool encoder_is_down()
{
    return encoder_down_time > encoder_up_time && (HAL_GetTick() - encoder_down_time) > DEBOUNCE_TIME;
}

bool encoder_get_click()
{
    bool is_down = encoder_is_down();

    if (is_down && !encoder_press_detected) {
        encoder_press_detected = true;
        return true;
    } else if (!is_down) {
        encoder_press_detected = false;
    }

    return false;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == ROTARY_PRESS_Pin) {
        if (HAL_GPIO_ReadPin(ROTARY_PRESS_GPIO_Port, ROTARY_PRESS_Pin) == GPIO_PIN_SET) {
            encoder_up_time = HAL_GetTick();
        } else {
            encoder_down_time = HAL_GetTick();
        }
    }
}

int32_t encoder_get_step()
{
    // Detect rotary encoder value change
    uint32_t new_value = TIM4->CNT / 2;

    if(new_value != encoder_last_value)
    {
        int32_t encoder_increment = (new_value < encoder_last_value)? -1 : +1;
        // Handle overflow cases
        if(new_value == 32767 && encoder_last_value == 0)
        {
            encoder_increment = -1;
        }
        else if (new_value == 0 && encoder_last_value == 32767)
        {
            encoder_increment = +1;
        }
        encoder_last_value = new_value;

#ifdef ENCODER_INVERT
        return -encoder_increment;
#else
        return encoder_increment;
#endif
    }

    return 0;
}
