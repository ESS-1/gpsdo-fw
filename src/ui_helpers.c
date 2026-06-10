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
