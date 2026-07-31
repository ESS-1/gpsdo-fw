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

bool pll_init_primary_pll();
bool pll_wait_primary_lock();
void pll_enable_primary_output();
void pll_configure_output(uint8_t output, const OutFreqConfig* config, uint8_t drive_strength);

void pll_run();

#endif
