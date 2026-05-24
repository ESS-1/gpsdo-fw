#ifndef _PLL_H_
#define _PLL_H_

#include "si5351.h"

void pll_init_primary_vco();
bool pll_wait_primary_lock();
bool pll_enable_primary_output();

#endif
