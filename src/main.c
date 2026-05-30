#include "main.h"
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "LCD.h"
#include "eeprom.h"
#include "frequency.h"
#include "gps.h"
#include "menu.h"
#include "int.h"
#include "tim.h"
#include "pll.h"
#include "bootlog.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// All times in ms
#define PPS_PULSE_WIDTH         100
#define GPS_FRAME_WAIT_DELAY    10000

void init_ext_clock()
{
    // MX_GPIO_Init() sets a high level on OCXO_EN; wait for contact bounce to settle before proceeding
    HAL_Delay(750);

    // Initialize minimal peripherals required to configure the external clock
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_SPI1_Init();

    bootlog_init();
    bootlog_add("Micro-DO " FIRMWARE_VERSION);
    bootlog_add("Initializing...");
    bootlog_add("OCXO enabled");

    // Wait for OCXO startup
    HAL_Delay(500);

    // Init SI5351 PLL
    bootlog_add("Init PLL");
    pll_init_primary_vco();
    bootlog_set_status(true);

    // Wait for PLL lock
    bootlog_add("PLL Lock");
    bool pll_fail = false;
    if (pll_wait_primary_lock()) {
        if (pll_enable_primary_output()) {
            bootlog_set_status(true);
        } else {
            bootlog_set_status(false);
            pll_fail = true;
            bootlog_error("PLL output failure!");
        }
    } else {
        bootlog_set_status(false);
        pll_fail = true;
        bootlog_error("PLL lock failure!");
    }

    if (pll_fail) {
        // Turn off OCXO
        HAL_GPIO_WritePin(OCXO_EN_GPIO_Port, OCXO_EN_Pin, 0);
        bootlog_add("OCXO disabled");

        Error_Handler();
        return;
    }

    // Switch to the normal operation mode
    bootlog_add("Use OCXO clock...");
    HAL_Delay(750);

    // De-initialize peripherals
    HAL_I2C_DeInit(&hi2c1);
    HAL_SPI_DeInit(&hspi1);
}

void enable_usb()
{
    // Turn on 1.5K USB D+ pull-up
    HAL_GPIO_WritePin(USB_DP_PULLUP_GPIO_Port, USB_DP_PULLUP_Pin, GPIO_PIN_SET);

    GPIO_InitTypeDef gpio_init = { 0 };
    gpio_init.Pin              = USB_DP_PULLUP_Pin;
    gpio_init.Mode             = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull             = GPIO_NOPULL;
    gpio_init.Speed            = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(USB_DP_PULLUP_GPIO_Port, &gpio_init);
}

void gpsdo()
{
    HAL_TIM_Base_Start_IT(&htim2);

    EE_Init(&ee_storage, sizeof(ee_storage_t));
    EE_Read();
    // Read OCXO model first since we'll use it to choose PWM starting point
    if (ee_storage.ocxo_model == 0xff) {
        ee_storage.ocxo_model = OCXO_MODEL_UNKNOWN;
    }
    ocxo_model = ee_storage.ocxo_model;
    uint16_t startingPwm;
    if (ee_storage.pwm == 0xffff) {
        // Pwm not initialized choose starting point based on OCXO model
        switch(ocxo_model)
        {
            case OCXO_MODEL_OX256B:
                startingPwm = 38000; // about 2.5V - several of the OX256B units I have show zero error at this control voltage
                break;
            case OCXO_MODEL_ISOTEMP:
            case OCXO_MODEL_UNKNOWN:
            default:
                startingPwm = 32000; // about 2V - typical center value of OCXO control voltage
                break;
        }
    }
    else {
        // Use value stored in eeprom as a starting point
        startingPwm = ee_storage.pwm;
    }
    TIM1->CCR2 = startingPwm;
    if (ee_storage.brightness == 0xff) {
        ee_storage.brightness = 50;
    }
    brightness = ee_storage.brightness;
    update_brightness();
    if (ee_storage.pps_sync_on == 0xff) {
        ee_storage.pps_sync_on = true;
    }
    pps_sync_on = ee_storage.pps_sync_on;
    if (ee_storage.pps_sync_delay == 0xffffffff) {
        ee_storage.pps_sync_delay = 10;
    }
    pps_sync_delay = ee_storage.pps_sync_delay;
    if (ee_storage.pps_sync_threshold == 0xffffffff) {
        ee_storage.pps_sync_threshold = 30000;
    }
    pps_sync_threshold = ee_storage.pps_sync_threshold;
    if (ee_storage.pps_ppm_auto_sync == 0xff) {
        ee_storage.pps_ppm_auto_sync = true;
    }
    pps_ppm_auto_sync = ee_storage.pps_ppm_auto_sync;
    if (ee_storage.pwm_auto_save == 0xff) {
        ee_storage.pwm_auto_save = true;
    }
    pwm_auto_save = ee_storage.pwm_auto_save;

    if (ee_storage.trend_auto_v == 0xff) {
        ee_storage.trend_auto_v = true;
    }
    trend_auto_v = ee_storage.trend_auto_v;
    if (ee_storage.trend_auto_h == 0xff) {
        ee_storage.trend_auto_h = true;
    }
    trend_auto_h = ee_storage.trend_auto_h;
    if (ee_storage.trend_v_scale == 0xffffffff) {
        ee_storage.trend_v_scale = 70;
    }
    trend_v_scale = ee_storage.trend_v_scale;
    if (ee_storage.trend_h_scale == 0xffffffff) {
        ee_storage.trend_h_scale = 1;
    }
    trend_h_scale = ee_storage.trend_h_scale;
    // Check for custom gps baudrate
    if (ee_storage.gps_baudrate == 0xffffffff) {
        ee_storage.gps_baudrate = GPS_DEFAULT_BAUDRATE;
    }
    if (ee_storage.gps_time_offset == 0xffffffff) {
        ee_storage.gps_time_offset = -MIN_TIME_OFFSET;
    }
    gps_time_offset = ee_storage.gps_time_offset+MIN_TIME_OFFSET;
    if (ee_storage.gps_date_format == 0xff) {
        ee_storage.gps_date_format = DATE_FORMAT_UTC;
    }
    gps_date_format = ee_storage.gps_date_format;
    if (ee_storage.gps_model == 0xff) {
        ee_storage.gps_model = GPS_MODEL_UNKNOWN;
    }
    gps_model = ee_storage.gps_model;
    // PPB lock threshold (*100)
    if (ee_storage.ppb_lock_threshold == 0xffffffff) {
        ee_storage.ppb_lock_threshold = DEFAULT_PPB_LOCK_THRESHOLD;
    }
    ppb_lock_threshold = ee_storage.ppb_lock_threshold;
    // Correction algorithm
    if (ee_storage.correction_algorithm == 0xff) {
        ee_storage.correction_algorithm = CORRECTION_ALGO_ERIC_H_PLUS;
    }
    correction_algorithm = ee_storage.correction_algorithm;
    // Correction factor
    if (ee_storage.correction_factor == 0xffffffff) {
        ee_storage.correction_factor = get_default_correction_factor(correction_algorithm);
    }
    correction_factor = ee_storage.correction_factor;
    // Warmup time
    if (ee_storage.warmup_time_seconds == 0xffffffff) {
        ee_storage.warmup_time_seconds = get_default_warmup_time(ocxo_model);
    }
    warmup_time_seconds = ee_storage.warmup_time_seconds;
    // PC communication port baud rate
    if (ee_storage.comm_baudrate == 0xffffffff) {
        ee_storage.comm_baudrate = COMM_DEFAULT_BAUDRATE;
    }
    // Custom $PGDOx NMEA frames sending option
    if (ee_storage.gps_comm_send_pgdox == 0xff) {
        ee_storage.gps_comm_send_pgdox = true;
    }
    gps_comm_send_pgdox = ee_storage.gps_comm_send_pgdox;

    enable_usb();
    gps_start_it();

    menu_set_gps_baudrate(ee_storage.gps_baudrate);
    menu_set_comm_baudrate(ee_storage.comm_baudrate);
    menu_set_correction_algorithm(correction_algorithm);

    LCD_Init();

    lcd_create_chars();
    init_trend_values();

    // warmup();

    LCD_Clear();

    HAL_Delay(100);
    frequency_start();

    HAL_TIM_Base_Start(&htim4);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

    bool vco_adjust_allowed = false;

    while (1) {
        uint32_t now = HAL_GetTick();
        if(pps_out_up && now-last_pps_out >= PPS_PULSE_WIDTH)
        {
            HAL_GPIO_WritePin(PPS_OUTPUT_GPIO_Port, PPS_OUTPUT_Pin, 0);
            pps_out_up = false;
        }
        if (!vco_adjust_allowed && (now >= (warmup_time_seconds*1000)))
        {   // Start adjusting the VCO after some time
            frequency_allow_adjustment(true);
            vco_adjust_allowed = true;
        }
        if((now - last_frame_receive_time) > GPS_FRAME_WAIT_DELAY)
        {   // We've not been receiving a frame from GPS for too long, try and restart UART
            gps_reconfigure_gps_uart(gps_baudrate);
            last_frame_receive_time = now;
        }
        
        gps_read();
        menu_run();
    }
}
