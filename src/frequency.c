#include "frequency.h"
#include "eeprom.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"
#include "tim.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "int.h"

volatile int32_t            frequency_ppb_x100  = PPB_UNSET_VALUE;
volatile FrequencyStability frequency_stability = FREQ_STABILITY_UNSTABLE;

volatile circbuf_t circular_buffer = {0};

// Quick and dirty circular buffer
void circbuf_add(volatile circbuf_t* circbuf, int32_t val)
{
    circbuf->buf[circbuf->write] = val;
    circbuf->write               = (circbuf->write + 1) % CIRCULAR_BUFFER_LEN;
}

int32_t circbuf_sum(volatile circbuf_t* circbuf)
{
    int32_t sum = 0;
    for (size_t i = 0; i < CIRCULAR_BUFFER_LEN; i++) {
        sum += circbuf->buf[i];
    }
    return sum;
}

void frequency_start()
{
    HAL_TIM_Base_Start_IT(&htim1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
}

void frequency_allow_adjustment(bool allow) { allow_adjustment = allow; }

bool frequency_adjustment_allowed() { return allow_adjustment; }

int32_t frequency_get() { return frequency; }

int32_t frequency_get_error()
{
    if (!frequency) {
        return 0;
    } else {
        int32_t error = frequency - TARGET_FREQ /*HAL_RCC_GetHCLKFreq()*/;
        // Filter out obvious glitches, the OCXO should never be this far from the target frequency
        if (error > 2000 || error < -2000) {
            return 0;
        } else {
            return error;
        }
    }
}

static int32_t frequency_calculate_ppb_x100()
{
    if (num_samples == 0) {
        return PPB_UNSET_VALUE;
    }

    // Get ratio of cumulative error / expected number of cycles. Multiply by 1e9 for PPB and by
    // 100 to get additional digits without using floats.
    // This will be a running average over 128 seconds of the error in PPB*100
    return (int64_t)circbuf_sum(&circular_buffer) * 1000000000 * 100 / ((int64_t)HAL_RCC_GetHCLKFreq() * num_samples);
}

int32_t frequency_get_inst_ppb_x100()
{
    return (int64_t)ppb_error * 1000000000 * 100 / ((int64_t)HAL_RCC_GetHCLKFreq());
}

static FrequencyStability frequency_get_stability_level(uint32_t stableThreshold, uint32_t marginalThreshold)
{
    if (num_samples == CIRCULAR_BUFFER_LEN) {
        int32_t  ppb     = frequency_ppb_x100;
        uint32_t abs_ppb = (ppb < 0) ? -(uint32_t)ppb : (uint32_t)ppb;

        if (abs_ppb <= stableThreshold) {
            return FREQ_STABILITY_STABLE;
        } else if (abs_ppb < marginalThreshold) {
            return FREQ_STABILITY_MARGINAL;
        }
    }

    return FREQ_STABILITY_UNSTABLE;
}

void frequency_update_ppb_and_stability()
{
    int32_t new_ppb = frequency_calculate_ppb_x100();
    if (new_ppb == frequency_ppb_x100) {
        return;
    }

    frequency_ppb_x100 = new_ppb;

    uint32_t threshold = ee_storage.ppb_lock_threshold;
    FrequencyStability new_stability = frequency_get_stability_level(threshold, 5 * threshold);

    bool was_stable = (frequency_stability == FREQ_STABILITY_STABLE);
    bool is_stable  = (new_stability == FREQ_STABILITY_STABLE);

    if (was_stable != is_stable) {
        GPIO_PinState pin_state = is_stable ? GPIO_PIN_RESET : GPIO_PIN_SET; // Active low
        HAL_GPIO_WritePin(PPB_LOCK_OUTPUT_GPIO_Port, PPB_LOCK_OUTPUT_Pin, pin_state);
    }

    frequency_stability = new_stability;
}
