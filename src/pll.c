#include "main.h"
#include "pll.h"
#include "si5351.h"

void pll_init_primary_vco()
{
    si5351_Init(0, SI5351_CRYSTAL_LOAD_6PF);

    // Init PLL A (VCO = 25M * 32 = 800M
    si5351PLLConfig_t pll_config = { 0 };
    pll_config.mult  = 32;
    pll_config.num   = 0;
    pll_config.denom = 1;
    si5351_SetupPLL(SI5351_PLL_A, &pll_config);
    si5351_SetupPLL(SI5351_PLL_B, &pll_config);
}

bool pll_wait_primary_lock()
{
    return si5351_WaitPLLReady(SI5351_PLL_A, 150, 2000);
}

bool pll_enable_primary_output()
{
    // CLK0: 800M / 100 = 8M
    si5351OutputConfig_t out_config = { 0 };
    out_config.allowIntegerMode = 1;
    out_config.div = 100;
    out_config.num = 0;
    out_config.denom = 1;
    out_config.rdiv  = SI5351_R_DIV_1;
    if (si5351_SetupOutput(0, SI5351_PLL_A, SI5351_DRIVE_STRENGTH_8MA, &out_config, 0) != 0)
    {
        return false;
    }

/*TODO: temporary code*/    // CLK1: 800M / 80 = 10M
/*TODO: temporary code*/    out_config.allowIntegerMode = 1;
/*TODO: temporary code*/    out_config.div = 80;
/*TODO: temporary code*/    out_config.num = 0;
/*TODO: temporary code*/    out_config.denom = 1;
/*TODO: temporary code*/    out_config.rdiv  = SI5351_R_DIV_1;
/*TODO: temporary code*/    if (si5351_SetupOutput(1, SI5351_PLL_A, SI5351_DRIVE_STRENGTH_8MA, &out_config, 0) != 0)
/*TODO: temporary code*/    {
/*TODO: temporary code*/        return false;
/*TODO: temporary code*/    }

    // Enable output
//TODO:    si5351_EnableOutputs(1);
/*TODO: temporary code*/    si5351_EnableOutputs(0x03);

    return true;
}
