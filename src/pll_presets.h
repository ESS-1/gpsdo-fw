#ifndef _PLL_PRESETS_E8685073DF48_H_
#define _PLL_PRESETS_E8685073DF48_H_

#include <stdint.h>

typedef struct {
    uint16_t    out_div;
    uint8_t     pll_mult;
    const char* label;
} OutFreqConfig;

extern const OutFreqConfig pll_out1_options[];
extern const OutFreqConfig pll_out2_options[];

#endif
