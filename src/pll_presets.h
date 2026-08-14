#ifndef _PLL_PRESETS_E8685073DF48_H_
#define _PLL_PRESETS_E8685073DF48_H_

#include <stdint.h>

typedef struct {
    uint16_t    out_div;
    uint8_t     pll_mult;
    bool        direct_xo;
    const char* label;
} OutFreqConfig;

extern const OutFreqConfig pll_out1_presets[];
extern const OutFreqConfig pll_out2_presets[];

extern const uint16_t pll_out1_preset_count;
extern const uint16_t pll_out2_preset_count;

#endif
