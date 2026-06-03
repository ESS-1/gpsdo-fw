#ifndef _PLL_8D9F3DA723B8_H_
#define _PLL_8D9F3DA723B8_H_

#include "si5351.h"

void pll_init_primary_vco();
bool pll_wait_primary_lock();
bool pll_enable_primary_output();

#endif
