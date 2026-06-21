#include "main.h"
#include "pll.h"
#include "si5351.h"

void pll_init_primary_vco()
{
    si5351_Init(0, SI5351_CRYSTAL_LOAD_6PF);

    // Init PLL A (VCO = 10M * 88 = 880M
    si5351PLLConfig_t pll_config = { 0 };
    pll_config.mult  = 88;
    pll_config.num   = 0;
    pll_config.denom = 1;
    si5351_SetupPLL(SI5351_PLL_A, &pll_config);
}

bool pll_wait_primary_lock()
{
    return si5351_WaitPLLReady(SI5351_PLL_A, 1000, 1000);
}

bool pll_enable_primary_output()
{
    // CLK0: 880M / 110 = 8M
    si5351OutputConfig_t out_config = { 0 };
    out_config.allowIntegerMode = 1;
    out_config.div = 110;
    out_config.num = 0;
    out_config.denom = 1;
    out_config.rdiv  = SI5351_R_DIV_1;
    if (si5351_SetupOutput(0, SI5351_PLL_A, SI5351_DRIVE_STRENGTH_8MA, &out_config, 0) != 0)
    {
        return false;
    }

    // Enable output
    si5351_EnableOutputs(0x01);

    return true;
}

void pll_configure_output(uint8_t output, OutFreqConfig config)
{
    //TODO
}
