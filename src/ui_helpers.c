#include "ui_helpers.h"
#include "frequency.h"

#include <inttypes.h>
#include <memory.h>
#include <stdio.h>

void ui_format_ppb_5char(int32_t ppb_signed, char* buffer, size_t bufferSize)
{
    if (buffer == NULL || bufferSize == 0) {
        return;
    }

    if (bufferSize < 6) {
        memset(buffer, '#', bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
        return;
    }

    if (ppb_signed == PPB_UNSET_VALUE) {
        snprintf(buffer, bufferSize, " N/A ");
        return;
    }

    // Absolute value of ppb_signed as uint32_t (handles INT32_MIN)
    uint32_t ppb = (ppb_signed < 0) ? -(uint32_t)ppb_signed : (uint32_t)ppb_signed;

    if (ppb < 9995u) {
        // Add sign
        buffer[0] = ppb_signed >= 0 ? ' ' : '-';

        // Format numeric value
        if (ppb < 1000u) {
            snprintf(buffer + 1, bufferSize - 1, "%" PRIu32 ".%02" PRIu32, ppb / 100u, ppb % 100u);
        } else {
            uint32_t tenths = (ppb + 5u) / 10u;
            snprintf(buffer + 1, bufferSize - 1, "%" PRIu32 ".%01" PRIu32, tenths / 10u, tenths % 10u);
        }
    } else {
        int32_t sign = ppb_signed < 0 ? -1 : 1;
        if (ppb < 999950u) {
            snprintf(buffer, bufferSize, "%5" PRId32, sign * (int32_t)((ppb + 50u) / 100u));
        } else if (ppb < 99950000u) {
            snprintf(buffer, bufferSize, "%4" PRId32 "K", sign * (int32_t)((ppb + 50000u) / 100000u));
        } else {
            snprintf(buffer, bufferSize, "%4" PRId32 "M", sign * (int32_t)((ppb + 50000000u) / 100000000u));
        }
    }
}

void ui_format_ppb_9char(int32_t ppb_signed, char* buffer, size_t bufferSize)
{
    if (buffer == NULL || bufferSize == 0) {
        return;
    }

    if (bufferSize < 10) {
        memset(buffer, '#', bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
        return;
    }

    if (ppb_signed == PPB_UNSET_VALUE) {
        snprintf(buffer, bufferSize, "      N/A");
        return;
    }
    if (ppb_signed > 9999999) {
        snprintf(buffer, bufferSize, "   > 100K");
        return;
    }
    if (ppb_signed < -9999999) {
        snprintf(buffer, bufferSize, "  < -100K");
        return;
    }

    uint32_t ppb_abs = (ppb_signed < 0) ? -(uint32_t)ppb_signed : (uint32_t)ppb_signed;
    if (ppb_signed < 0 && ppb_signed > -100) {
        snprintf(buffer, bufferSize, "    -0.%02" PRIu32, ppb_abs);
    } else {
        snprintf(buffer, bufferSize, "%6" PRIi32 ".%02" PRIu32, ppb_signed / 100, ppb_abs % 100u);
    }
}

const char* ui_get_month_name_3char(uint8_t month_num)
{
    static const char months[13][4] = { "###", // Fallback for invalid month number
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    if (month_num < 1 || month_num > 12) {
        return months[0];
    }

    return months[month_num];
}

void ui_change_setting_i8(int8_t* value, int32_t step, int8_t min, int8_t max, bool wrap_around)
{
    int32_t v = (int32_t)(*value) + step;

    if (v > max) {
        *value = wrap_around ? min : max;
    } else if (v < min) {
        *value = wrap_around ? max : min;
    } else {
        *value = (int8_t)v;
    }
}

void ui_change_setting_u8(uint8_t* value, int32_t step, uint8_t min, uint8_t max, bool wrap_around)
{
    int32_t v = (int32_t)(*value) + step;

    if (v > max) {
        *value = wrap_around ? min : max;
    } else if (v < min) {
        *value = wrap_around ? max : min;
    } else {
        *value = (uint8_t)v;
    }
}

void ui_change_setting_u16(uint16_t* value, int32_t step, uint16_t min, uint16_t max, bool wrap_around)
{
    int32_t v = (int32_t)(*value) + step;

    if (v > max) {
        *value = wrap_around ? min : max;
    } else if (v < min) {
        *value = wrap_around ? max : min;
    } else {
        *value = (uint16_t)v;
    }
}

void ui_change_setting_u32(uint32_t* value, int32_t step, uint32_t min, uint32_t max, bool wrap_around)
{
    int64_t v = (int64_t)(*value) + step;

    if (v > (int64_t)max) {
        *value = wrap_around ? min : max;
    } else if (v < (int64_t)min) {
        *value = wrap_around ? max : min;
    } else {
        *value = (uint32_t)v;
    }
}

int32_t ui_limit_i32(int32_t value, int32_t min, int32_t max)
{
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

// Calculates an adaptive step increment (1, 2, 5, 10, 20, 50, 100, etc.)
// based on the magnitude of the current value and encoder direction.
// The step dynamically scales using a 1-2-5 progression per decade.
int32_t ui_get_adaptive_step(uint32_t value, int32_t encoder_step)
{
    int32_t sign = (encoder_step < 0) ? -1 : 1;

    // Shift boundary evaluation by -1 when decrementing to ensure exact
    // reverse symmetry at range transition points
    uint32_t effective_value = (sign < 0 && value > 0) ? (value - 1) : value;

    // Base step size for values below 10
    if (effective_value < 10) {
        return 1 * sign;
    }

    // Normalize value into the [10..99] decade range and calculate power of 10 scale
    uint32_t norm_val = effective_value;
    uint32_t scale    = 1;
    while (norm_val >= 100) {
        norm_val /= 10;
        scale *= 10;
    }

    // Determine step size based on 1-2-5 decade progression
    if (norm_val < 20) {
        return 1 * sign * scale;
    } else if (norm_val < 50) {
        return 2 * sign * scale;
    } else {
        return 5 * sign * scale;
    }
}
