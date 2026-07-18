#include "main.h"
#include "pll.h"
#include "timer.h"
#include "si5351.h"

PllStatus pll_status = PllStatus_Ok;
static uint32_t pll_status_last_update = 0;

static uint8_t pll_enabled_outputs = 0;

static void pll_enable_output(uint8_t output, bool enable)
{
    uint8_t f = (uint8_t)(1u << output);
    if (enable) {
        pll_enabled_outputs |= f;
    } else {
        pll_enabled_outputs &= (uint8_t)~f;
    }

    si5351_EnableOutputs(pll_enabled_outputs);
}

bool pll_init_primary_pll()
{
    si5351_Init(0, SI5351_CRYSTAL_LOAD_6PF);

    // Init PLL A (VCO = 10M * 88 = 880M
    si5351PLLConfig_t pll_config = { 0 };
    pll_config.mult  = 88;
    pll_config.num   = 0;
    pll_config.denom = 1;
    si5351_SetupPLL(SI5351_PLL_A, &pll_config);

    // CLK0: 880M / 110 = 8M
    si5351OutputConfig_t out_config = { 0 };
    out_config.allowIntegerMode = 1;
    out_config.div = 110;
    out_config.num = 0;
    out_config.denom = 1;
    out_config.rdiv  = SI5351_R_DIV_1;
    if (si5351_SetupChannel(0, SI5351_PLL_A, SI5351_DRIVE_STRENGTH_8MA, &out_config, 0) != 0) {
        return false;
    }

    si5351_ResetPLL(SI5351_PLL_A);

    return true;
}

bool pll_wait_primary_lock()
{
    return si5351_WaitPLLReady(SI5351_PLL_A, 2000);
}

void pll_enable_primary_output()
{
    // Enable output
    pll_enable_output(0, true);
}

void pll_configure_output(uint8_t output, const OutFreqConfig* config, uint8_t drive_strength)
{
    // Reset the 'pll_status' update timer to prevent transitions from disturbing the UI
    timer_reset(&pll_status_last_update);

    // Disable output
    pll_enable_output(output, false);

    // Set up the PLL if we are configuring output 2; output 1 uses PLL A,
    // which is shared with the GPSDO control loop, so its settings should not be changed.
    if (output == 2) {
        si5351PLLConfig_t pll_config = {
            .mult = config->pll_mult,
            .num = 0,
            .denom = 1 };
        si5351_SetupPLL(SI5351_PLL_B, &pll_config);
    }

    // Configure output
    bool enableOutput = (config->out_div != 0);

    if (enableOutput) {
        si5351OutputConfig_t out_config = {
            .allowIntegerMode = 1,
            .div = config->out_div,
            .num = 0,
            .denom = 1,
            .rdiv = SI5351_R_DIV_1 };

        si5351PLL_t pll = output == 1 ? SI5351_PLL_A : SI5351_PLL_B;
        si5351_SetupChannel(output, pll, drive_strength, &out_config, 0);
    } else {
        si5351_DisableChannel(output);
    }

    // Reset the PLL if we are configuring output 2
    if (output == 2) {
        si5351_ResetPLL(SI5351_PLL_B);
    }

    // Enable output
    if (enableOutput) {
        pll_enable_output(output, true);
    }
}

void pll_update()
{
    // Update every 0.5 seconds
    if (!timer_is_elapsed(&pll_status_last_update, 500)) {
        return;
    }

    si5351ReadyFlags_t flags = si5351_GetReadyStatus();

    PllStatus new_status = PllStatus_Ok;
    if ((flags & SI5351_READY_SYS_INIT_DONE) == 0 || (flags & SI5351_READY_XTAL_VALID) == 0) {
        new_status |= PllStatus_General_Error;
    }

    if ((flags & SI5351_READY_PLLA_LOCKED) == 0) {
        new_status |= PllStatus_PllA_Error;
    }
    if ((flags & SI5351_READY_PLLB_LOCKED) == 0) {
        new_status |= PllStatus_PllB_Error;
    }

    pll_status = new_status;
}
