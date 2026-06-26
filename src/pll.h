#ifndef _PLL_8D9F3DA723B8_H_
#define _PLL_8D9F3DA723B8_H_

#include "si5351.h"
#include "pll_presets.h"
#include <stdint.h>

typedef enum : uint8_t {
    PllStatus_Ok            = 0x00,
    PllStatus_General_Error = 0x01,
    PllStatus_PllA_Error    = 0x02,
    PllStatus_PllB_Error    = 0x04,
} PllStatus;

extern PllStatus pll_status;

void pll_init_primary_vco();
bool pll_wait_primary_lock();
bool pll_enable_primary_output();
void pll_configure_output(uint8_t output, const OutFreqConfig *config);

void pll_update();

#endif
