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

const char* ui_get_month_name_3char(uint8_t month_num)
{
    static const char months[13][4] = { "###", // Fallback for invalid month number
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    if (month_num < 1 || month_num > 12) {
        return months[0];
    }

    return months[month_num];
}

void ui_change_setting_i8(int8_t* value, int32_t step, int8_t min, int8_t max)
{
    int8_t v = (*value) + step;
    if (v > max) {
        *value = min;
    } else if (v < min) {
        *value = max;
    } else {
        *value = v;
    }
}

void ui_change_setting_u16(uint16_t* value, int32_t step, uint16_t max)
{
    int32_t v = (*value) + step;
    if (v > max) {
        *value = 0;
    } else if (v < 0) {
        *value = max;
    } else {
        *value = (uint16_t)v;
    }
}
