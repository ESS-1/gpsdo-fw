#include "main.h"
#include "menu.h"
#include "ui.h"
#include <stdbool.h>

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <math.h>

//#include "frequency.h"
//#include "eeprom.h"
//#include "gps.h"
//#include "stm32f1xx_hal_gpio.h"
//#include "int.h"
//#include "trend8_t.h"
#include "cdcio.h"
#include "icons.h"

#include "fonts.h"
#include "st7735.h"
#include "st7735_config.h"


#define UI_BG_COLOR (ST7735_BLACK)



//typedef enum { SCREEN_MAIN, SCREEN_DATE, SCREEN_DATE_TIME, SCREEN_TREND, SCREEN_PPB, SCREEN_PWM, SCREEN_GPS, SCREEN_UPTIME, SCREEN_FRAMES, SCREEN_BRIGHTNESS, SCREEN_PPS, SCREEN_SAVE_CONFIG, SCREEN_VERSION, SCREEN_MAX } menu_screen;
//typedef enum { SCREEN_TREND_MAIN, SCREEN_TREND_AUTO_V, SCREEN_TREND_AUTO_H, SCREEN_TREND_V_SCALE, SCREEN_TREND_H_SCALE, SCREEN_TREND_EXIT, SCREEN_TREND_MAX } menu_trend_screen;
//typedef enum { SCREEN_GPS_TIME, SCREEN_GPS_LATITUDE, SCREEN_GPS_LONGITUDE, SCREEN_GPS_LATITUDE_DEC, SCREEN_GPS_LONGITUDE_DEC, SCREEN_GPS_LOCATOR, SCREEN_GPS_ALTITUDE, SCREEN_GPS_GEOID, SCREEN_GPS_SATELITES, SCREEN_GPS_HDOP, SCREEN_GPS_BAUDRATE, SCREEN_GPS_ERRORS, SCREEN_GPS_TIME_OFFSET, SCREEN_GPS_DATE_FORMAT, SCREEN_GPS_MODEL, SCREEN_GPS_LAST_FRAME, SCREEN_GPS_EXIT, SCREEN_GPS_MAX } menu_gps_screen;
//typedef enum { SCREEN_PPB_MEAN, SCREEN_PPB_INST, SCREEN_PPB_FREQUENCY, SCREEN_PPB_ERROR, SCREEN_PPB_CORRECTION, SCREEN_PPB_PWM, SCREEN_PPB_OCXO_MODEL, SCREEN_PPB_WARMUP_TIME, SCREEN_PPB_ALGO, SCREEN_PPB_CORRECTION_FACTOR, SCREEN_PPB_MILLIS, SCREEN_PPB_AUTO_SAVE_PWM, SCREEN_PPB_AUTO_SYNC_PPS, SCREEN_PPB_LOCK_THRESHOLD, SCREEN_PPB_EXIT, SCREEN_PPB_MAX } menu_ppb_screen;
//typedef enum { SCREEN_PPS_SHIFT, SCREEN_PPS_SHIFT_MS, SCREEN_PPS_SYNC_COUNT, SCREEN_PPS_SYNC_MODE, SCREEN_PPS_SYNC_DELAY, SCREEN_PPS_SYNC_THRESHOLD, SCREEN_PPS_FORCE_SYNC, SCREEN_PPS_EXIT, SCREEN_PPS_MAX } menu_pps_screen;
//
//// Possible baudrate values
//typedef enum { BAUDRATE_9600, BAUDRATE_19200, BAUDRATE_38400, BAUDRATE_57600, BAUDRATE_115200, BAUDRATE_230400, BAUDRATE_460800, BAUDRATE_921600, BAUDRATE_MAX} baudrate;
//
//static menu_screen current_menu_screen = SCREEN_MAIN;
//static menu_trend_screen current_menu_trend_screen = SCREEN_TREND_MAIN;
//static menu_gps_screen current_menu_gps_screen = SCREEN_GPS_TIME;
//static menu_ppb_screen current_menu_ppb_screen = SCREEN_PPB_MEAN;
//static menu_pps_screen current_menu_pps_screen = SCREEN_PPS_SHIFT;
//static uint8_t      menu_level          = 0;
//
//static bool         auto_save_pwm_done  = false;
//static bool         auto_sync_pps_done  = false;
//
//#define TREND_SCREEN_SIZE   40
//#define TREND_MAX_H_SCALE   64
//#define TREND_MAX_SIZE      (112*TREND_MAX_H_SCALE + TREND_SCREEN_SIZE) // Max trend duration: 2:00:08
//#define TREND_MAX_SHIFT     (TREND_MAX_SIZE - TREND_SCREEN_SIZE)
//static trend8_t     ppb_trend_values[TREND_MAX_SIZE];
//static uint32_t     ppb_trend_position = 0;
//static uint32_t     ppb_trend_size = 0;
//
//uint32_t    trend_shift = 0; 
//uint8_t     trend_arrow = TREND_LEFT_CODE;
//
//baudrate    gps_baudrate_enum = BAUDRATE_9600;
//
//#define         DATE_TIME_DURATION  5000 // Change date/time screen display every 5 seconds
//static uint32_t last_hour_date_screen_update = 0;
//
//correction_algo_type displayed_correction_algorithm;
//
//uint32_t menu_get_baudrate_value(baudrate baudrate_enum)
//{
//    uint32_t result;
//    switch (baudrate_enum)
//    {
//        default:
//        case BAUDRATE_9600:
//            result = 9600;
//            break;
//        case BAUDRATE_19200:
//            result = 19200;
//            break;
//        case BAUDRATE_38400:
//            result = 38400;
//            break;
//        case BAUDRATE_57600:
//            result = 57600;
//            break;
//        case BAUDRATE_115200:
//            result = 115200;
//            break;
//        case BAUDRATE_230400:
//            result = 230400;
//            break;
//        case BAUDRATE_460800:
//            result = 460800;
//            break;
//        case BAUDRATE_921600:
//            result = 921600;
//            break;
//    }
//    return result;
//}
//
//baudrate menu_get_baudrate_enum(uint32_t baudrate_value)
//{
//    baudrate result;
//    if(baudrate_value <= 9600)
//    {
//        result = BAUDRATE_9600;
//    }
//    else if(baudrate_value <= 19200)
//    {
//        result = BAUDRATE_19200;
//    }
//    else if(baudrate_value <= 38400)
//    {
//        result = BAUDRATE_38400;
//    }
//    else if(baudrate_value <= 57600)
//    {
//        result = BAUDRATE_57600;
//    }
//    else if(baudrate_value <= 115200)
//    {
//        result = BAUDRATE_115200;
//    }
//    else if(baudrate_value <= 230400)
//    {
//        result = BAUDRATE_230400;
//    }
//    else if(baudrate_value <= 460800)
//    {
//        result = BAUDRATE_460800;
//    }
//    else // if(baudrate_value <= 921600)
//    {
//        result = BAUDRATE_921600;
//    }
//    return result;
//}
//
//void menu_set_gps_baudrate(uint32_t baudrate)
//{
//    if(baudrate != ee_storage.gps_baudrate)
//    {   // Baudrate changed
//        ee_storage.gps_baudrate = baudrate;
//        ee_is_changed = true;
//        gps_baudrate_enum = menu_get_baudrate_enum(baudrate);
//        gps_change_module_baudrate(ee_storage.gps_baudrate);
//        gps_reconfigure_gps_uart(ee_storage.gps_baudrate);
//    }
//}
//
//void menu_set_correction_algorithm(correction_algo_type algo)
//{
//    displayed_correction_algorithm = algo;
//}
//
//static void menu_force_redraw() { refresh_screen = true; }
//
//void init_trend_values()
//{
//    for(int i = 0 ; i < TREND_MAX_SIZE ; i++)
//    {
//        ppb_trend_values[i] = TREND_ENCODED_UNSET_VALUE;
//    }
//}
//
//static uint32_t get_trend_data(uint32_t offset)
//{
//    if (offset > TREND_MAX_SIZE)
//    {   // We are trying to access a value outside the trend history range
//        // This can happen when ee_storage.trend_h_scale >= 2 and the trend is being scrolled
//        return TREND_UNSET_VALUE;
//    }
//
//    int32_t read_index = (int32_t)ppb_trend_position - (int32_t)offset;
//    if (read_index < 0)
//    {   // Wrap around
//        read_index = TREND_MAX_SIZE + read_index;
//    }
//
//    return decode_trend8_t(ppb_trend_values[read_index]);
//}
//
//static uint32_t get_trend_value(uint32_t position, uint32_t shift, uint32_t h_scale)
//{
//    if(h_scale == 1)
//    {   // No h scaling
//        return get_trend_data(shift + TREND_SCREEN_SIZE - position);
//    }
//    else
//    {   // Compute mean value over h-scale size
//        uint32_t result = 0;
//        uint32_t new_value;
//        for(uint32_t i = 0 ; i < h_scale ; i ++)
//        {   // '(ppb_trend_position % h_scale)' : Use the same start position for each point in time within the given scale group
//            new_value = get_trend_data((ppb_trend_position%h_scale) + shift + (TREND_SCREEN_SIZE*h_scale) - (position*h_scale) - i);
//            if(new_value == TREND_UNSET_VALUE)
//            {   // Don't compute mean value if one value is unset
//                return TREND_UNSET_VALUE;
//            }
//            result += new_value;
//        }
//        return result/h_scale;
//    }
//}
//
//static uint32_t get_trend_peak_value(uint32_t shift)
//{
//    uint32_t peak_value = 0;
//    uint32_t cur_value;
//    for(int32_t pos = 0; pos < TREND_SCREEN_SIZE ; pos++)
//    {
//        cur_value = get_trend_value(pos,shift,ee_storage.trend_h_scale);
//        if((cur_value != TREND_UNSET_VALUE) && (cur_value > peak_value))
//        {
//            peak_value = cur_value;
//        }
//    }
//    return peak_value;
//}
//
//static void add_trend_value(uint32_t value)
//{
//    ppb_trend_values[ppb_trend_position] = encode_trend8_t(value);
//    ppb_trend_position++;
//    if(ppb_trend_position>=TREND_MAX_SIZE)
//    {
//        ppb_trend_position = 0;
//    }
//
//    if (ppb_trend_size < TREND_MAX_SIZE)
//    {
//        ppb_trend_size++;
//    }
//}
//
//static uint32_t menu_round_v_scale(uint32_t scale)
//{
//    uint32_t rounded_scale;
//    if(scale < 70)
//    {   // 70 is the lower possible scale (0.1 ppb = 1px)
//        rounded_scale = 70;
//    }
//    else if(scale > 2000)
//    {   // For large values round scale to 10 ppb
//        rounded_scale = round(((double)scale)/1000)*1000;
//    }
//    else if(scale > 200)
//    {   // For medium values round scale to 1 ppb
//        rounded_scale = round(((double)scale)/100)*100;
//    }
//    else
//    {   // For smaller values, round scale to 0.1 ppb
//        rounded_scale = round(((double)scale)/10)*10;
//    }
//    return rounded_scale;
//}
//
//static uint32_t menu_round_h_scale(uint32_t scale)
//{
//    uint32_t rounded_scale = 0;
//    if(scale >= TREND_MAX_H_SCALE)
//    {
//        rounded_scale = TREND_MAX_H_SCALE;
//    }
//    else if(scale <= 1)
//    {
//        rounded_scale = 1;
//    }
//    else
//    {   // Ceil to power of 2
//        rounded_scale = scale - 1;
//
//        rounded_scale |= rounded_scale >> 1;
//        rounded_scale |= rounded_scale >> 2;
//        rounded_scale |= rounded_scale >> 4;
//
//        ++rounded_scale;
//    }
//    return rounded_scale;
//}
//
//static void menu_draw_trend(uint32_t shift)
//{   // Horizontal autoscale
//    if(ee_storage.trend_auto_h)
//    {   // Need to zoom horizontally
//        <?ee_storage.?>trend_h_scale = menu_round_h_scale((ppb_trend_size+TREND_SCREEN_SIZE-1) / TREND_SCREEN_SIZE);
//    }
//    // Vertical auto-scale
//    if(ee_storage.trend_auto_v)
//    {   // Determine scale, to fit the screen
//        <?ee_storage.?>trend_v_scale = menu_round_v_scale(get_trend_peak_value(shift));
//    }
//    for(int col_screen = 0 ; col_screen < 8 ; col_screen++)
//    {
//        uint8_t cust_char[8] = {0};
//        for(int col_char = 0; col_char < 5 ; col_char++)
//        {
//            uint32_t cur_ppb = get_trend_value(col_screen * 5 + col_char,shift,ee_storage.trend_h_scale);
//            // Ignore unset values
//            if(cur_ppb != TREND_UNSET_VALUE)
//            {   
//                uint8_t cur_val = cur_ppb >= ee_storage.trend_v_scale ? 7 : cur_ppb * 7 / ee_storage.trend_v_scale;
//                cust_char[7-cur_val]  |= (__extension__ 0b10000 >> col_char);
//            }
//        }
//        LCD_CreateChar(col_screen,cust_char);
//        LCD_PutCustom(col_screen,1,col_screen);
//    }
//}

//static void menu_format_ppb(int32_t ppb, char *buffer, size_t bufferSize)
//{
//    if (ppb == PPB_UNSET_VALUE) {
//        snprintf(buffer, bufferSize, "N/A");
//    } else {
//        // Divide the PPB value by 100 and keep the remainder.
//        int integerPart = abs(ppb / 100);
//        int decimalPart = abs(ppb % 100);
//
//        // Make sure negative values <0 are displayed correctly.
//        snprintf(buffer, bufferSize, "%s%d.%02d", ppb < 0 ? "-" : "", integerPart, decimalPart);
//    }
//}

//static void menu_format_ppb_compact(int32_t ppb_signed, char* buffer, size_t bufferSize)
//{
//    int32_t ppb = abs(ppb_signed);
//
//    if (ppb == PPB_UNSET_VALUE) {
//        snprintf(buffer, bufferSize, "   ?");
//    } else if (ppb > 999999) {
//        snprintf(buffer, bufferSize, ">10k");
//    } else if (ppb > 9999) {
//        snprintf(buffer, bufferSize, "%4ld", (ppb / 100));
//    } else if (ppb > 999) {
//        snprintf(buffer, bufferSize, "%ld.%01ld", ppb / 100, ((ppb % 100)/10));
//    } else {
//        snprintf(buffer, bufferSize, "%ld.%02ld", ppb / 100, ppb % 100);
//    }
//}

//#define PPB_STRING_SIZE     5
//#define SCREEN_BUFFER_SIZE  14
//
//static void menu_draw()
//{
//    char    screen_buffer[SCREEN_BUFFER_SIZE];
//    char    ppb_string[PPB_STRING_SIZE];
//    int32_t ppb;
//
//    switch (current_menu_screen) {
//    default:
//    case SCREEN_MAIN:
//    case SCREEN_DATE:
//    case SCREEN_DATE_TIME:
//        // Main screen with satellites, ppb and UTC time
//        menu_format_ppb_compact(frequency_get_ppb(), ppb_string, PPB_STRING_SIZE);
//        snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%02d %s", num_sats, ppb_string);
//        LCD_Puts(1, 0, screen_buffer);
//        if(current_menu_screen == SCREEN_MAIN)
//        {
//            LCD_Puts(0, 1, gps_time);
//        }
//        else if(current_menu_screen == SCREEN_DATE)
//        {
//            LCD_Puts(0, 1, gps_date);
//        }
//        else // SCREEN_DATE_TIME
//        {
//            uint32_t now = HAL_GetTick();
//            uint32_t duration = now - last_hour_date_screen_update;
//            if(duration <= DATE_TIME_DURATION)
//            {
//                LCD_Puts(0, 1, gps_time);
//            }
//            else
//            {
//                LCD_Puts(0, 1, gps_date);
//            }
//            if(duration >= 2*DATE_TIME_DURATION)
//            {
//                last_hour_date_screen_update = now;
//            }
//        }
//        break;
//    case SCREEN_TREND:
//        // Trend screen 
//        if(menu_level == 0)
//        {
//            menu_format_ppb_compact(frequency_get_ppb(), ppb_string, PPB_STRING_SIZE);
//            snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%02d %s", num_sats, ppb_string);
//            LCD_Puts(1, 0, screen_buffer);
//            menu_draw_trend(0);
//        }
//        else
//        {
//            switch (current_menu_trend_screen)
//            {
//                default:
//                case SCREEN_TREND_MAIN:
//                    if(menu_level == 1)
//                    {
//                        menu_format_ppb_compact(frequency_get_ppb(), ppb_string, PPB_STRING_SIZE);
//                        snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%02d/%s", num_sats, ppb_string);
//                        LCD_Puts(1, 0, screen_buffer);
//                        menu_draw_trend(0);
//                    }
//                    else
//                    {   // Show value at the left of the screen
//                        uint32_t trend_ppb = get_trend_value(TREND_SCREEN_SIZE - 1, trend_shift, ee_storage.trend_h_scale);
//                        menu_format_ppb_compact(trend_ppb != TREND_UNSET_VALUE ? trend_ppb : PPB_UNSET_VALUE, ppb_string, PPB_STRING_SIZE);
//                        snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%03ld%c%s", trend_shift, trend_arrow, ppb_string);
//                        LCD_Puts(0, 0, screen_buffer);
//                        menu_draw_trend(trend_shift);
//                    }
//                    break;
//                case SCREEN_TREND_AUTO_V:
//                    LCD_Puts(1, 0, menu_level == 1 ? "Auto-V:":"Auto-V?");
//                    LCD_Puts(0, 1, ee_storage.trend_auto_v ? "      ON" : "     OFF");
//                    break;
//                case SCREEN_TREND_AUTO_H:
//                    LCD_Puts(1, 0, menu_level == 1 ? "Auto-H:":"Auto-H?");
//                    LCD_Puts(0, 1, ee_storage.trend_auto_h ? "      ON" : "     OFF");
//                    break;
//                case SCREEN_TREND_V_SCALE:
//                    LCD_Puts(1, 0, menu_level == 1 ? "V-Scal:":"V-Scal?");
//                    LCD_Puts(0, 1, "        ");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld.%02ld", ee_storage.trend_v_scale / 100, ee_storage.trend_v_scale % 100);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_TREND_H_SCALE:
//                    LCD_Puts(1, 0, menu_level == 1 ? "H-Scal:":"H-Scal?");
//                    LCD_Puts(0, 1, "        ");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", ee_storage.trend_h_scale);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_TREND_EXIT:
//                    LCD_Puts(1, 0, "Exit?");
//                    LCD_Puts(0, 1, "        ");
//                    break;
//            }
//        }
//        break;
//    case SCREEN_PPB:
//        // Screen with ppb
//        if(menu_level == 0)
//        {
//            ppb = frequency_get_ppb();
//            LCD_Puts(1, 0, "PPB:   ");
//            LCD_Puts(0, 1, "        ");
//            menu_format_ppb(ppb, screen_buffer, SCREEN_BUFFER_SIZE);
//            LCD_Puts(0, 1, screen_buffer);
//        }
//        else
//        {
//            // Clear line 2
//            LCD_Puts(0, 1, "        ");
//            switch (current_menu_ppb_screen)
//            {
//                default:
//                case SCREEN_PPB_MEAN:
//                    ppb = frequency_get_ppb();
//                    LCD_Puts(1, 0, "Mean:");
//                    menu_format_ppb(ppb, screen_buffer, SCREEN_BUFFER_SIZE);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPB_INST:
//                    {
//                    LCD_Puts(1, 0, "Inst:");
//                    int32_t ppb_inst = frequency_get_inst_ppb();
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld.%02d", ppb_inst / 100, abs(ppb_inst) % 100);
//                    LCD_Puts(0, 1, screen_buffer);
//                    }
//                    break;
//                case SCREEN_PPB_FREQUENCY:
//                    LCD_Puts(1, 0, "Freq:");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", ppb_frequency);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPB_ERROR:
//                    LCD_Puts(1, 0, "Error:");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", ppb_error);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPB_CORRECTION:
//                    LCD_Puts(1, 0, "Corr.:");
//                    if(frequency_adjustment_allowed())
//                    {
//                        snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", ppb_correction);
//                        LCD_Puts(0, 1, screen_buffer);
//                    }
//                    else
//                    {
//                        LCD_Puts(0, 1, "Warm-up");
//                    }
//                    break;
//                case SCREEN_PPB_PWM:
//                    LCD_Puts(1, 0, "PWM:");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", TIM1->CCR2);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPB_OCXO_MODEL:
//                    LCD_Puts(1, 0, menu_level == 1 ? "OCXO:":"OCXO?");
//                    switch(ee_storage.ocxo_model)
//                    {
//                        case OCXO_MODEL_ISOTEMP:
//                            LCD_Puts(0, 1, "ISOTEMP");
//                            break;
//                        case OCXO_MODEL_OX256B:
//                            LCD_Puts(0, 1, "OX256B");
//                            break;
//                        default:
//                        case OCXO_MODEL_UNKNOWN:
//                            LCD_Puts(0, 1, "Unknown");
//                            break;
//                    }
//                    break;
//                case SCREEN_PPB_WARMUP_TIME:
//                    LCD_Puts(1, 0, menu_level == 1 ? "Warmup:":"Warmup?");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", ee_storage.warmup_time_seconds);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPB_ALGO:
//                    LCD_Puts(1, 0, menu_level == 1 ? "Algo.:":"Algo?");
//                    switch(displayed_correction_algorithm)
//                    {
//                        case CORRECTION_ALGO_DANKAR:
//                            LCD_Puts(0, 1, "Dankar");
//                            break;
//                        case CORRECTION_ALGO_ERIC_H:
//                            LCD_Puts(0, 1, "Eric H");
//                            break;
//                        case CORRECTION_ALGO_ERIC_H_PLUS:
//                            LCD_Puts(0, 1, "Eric H+");
//                            break;
//                        default:
//                        case CORRECTION_ALGO_FREDZO:
//                            LCD_Puts(0, 1, "Fredzo");
//                            break;
//                    }
//                    break;
//                case SCREEN_PPB_CORRECTION_FACTOR:
//                    LCD_Puts(1, 0, menu_level == 1 ? "Corr.F:":"Corr.F?");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", ee_storage.correction_factor);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPB_MILLIS:
//                    LCD_Puts(1, 0, "Millis:");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", ppb_millis);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPB_AUTO_SAVE_PWM:
//                    LCD_Puts(1, 0, menu_level == 1 ? "PWM S.:":"PWM S.?");
//                    LCD_Puts(0, 1, ee_settings.pwm_auto_save ? "      ON" : "     OFF");
//                    break;
//                case SCREEN_PPB_AUTO_SYNC_PPS:
//                    LCD_Puts(1, 0, menu_level == 1 ? "PPS S.:":"PPS S.?");
//                    LCD_Puts(0, 1, ee_settings.pps_ppm_auto_sync ? "      ON" : "     OFF");
//                    break;
//                case SCREEN_PPB_LOCK_THRESHOLD:
//                    LCD_Puts(1, 0, menu_level == 1 ? "PPB Lk:":"PPB Lk?");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld.%02ld", ee_storage.ppb_lock_threshold / 100, ee_storage.ppb_lock_threshold % 100);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPB_EXIT:
//                    LCD_Puts(1, 0, "Exit?");
//                    LCD_Puts(0, 1, "        ");
//                    break;
//            }
//        }
//        break;
//    case SCREEN_PWM:
//        // Screen with current PPM
//        if(menu_level == 0)
//        {
//            LCD_Puts(1, 0, "PWM:   ");
//            LCD_Puts(0, 1, "        ");
//            snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", TIM1->CCR2);
//            LCD_Puts(0, 1, screen_buffer);
//        }
//        else
//        {
//            LCD_Puts(0, 0, " PRESS  ");
//            LCD_Puts(0, 1, " TO SET ");
//        }
//        break;
//    case SCREEN_GPS:
//        if(menu_level == 0)
//        {
//            snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "GPS:%02d\5", num_sats);
//            LCD_Puts(1, 0, screen_buffer);
//            LCD_Puts(0, 1, gps_time);
//        }
//        else
//        {
//            // Clear line 2
//            LCD_Puts(0, 1, "        ");
//            switch (current_menu_gps_screen)
//            {
//                default:
//                case SCREEN_GPS_TIME:
//                    LCD_Puts(1, 0, "Time:");
//                    LCD_Puts(0, 1, gps_time);
//                    break;
//                case SCREEN_GPS_LATITUDE:
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "Lat.: %s", gps_n_s);
//                    LCD_Puts(1, 0, screen_buffer);
//                    LCD_Puts(0, 1, gps_latitude);
//                    break;
//                case SCREEN_GPS_LONGITUDE:
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "Long.:%s", gps_e_w);
//                    LCD_Puts(1, 0, screen_buffer);
//                    LCD_Puts(0, 1, gps_longitude);
//                    break;
//                case SCREEN_GPS_LATITUDE_DEC:
//                    {
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "Lat.D:");
//                    LCD_Puts(1, 0, screen_buffer);
//                    const char *fmt = "%d.%d";
//                    double gps_latitude_double_abs = gps_latitude_double;
//                    if (gps_latitude_double < 0.0)
//                    {
//                        gps_latitude_double_abs *= -1.0;
//                        fmt = "-%d.%d";
//                    }
//                    double coord_int = floor(gps_latitude_double_abs);
//                    double coord_frac = (gps_latitude_double_abs - coord_int)*1000000;
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, fmt, ((int)coord_int), ((int)coord_frac));
//                    LCD_Puts(0, 1, screen_buffer);
//                    }
//                break;
//                case SCREEN_GPS_LONGITUDE_DEC:
//                    {
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "Long.D:");
//                    LCD_Puts(1, 0, screen_buffer);
//                    const char *fmt = "%d.%d";
//                    double gps_longitude_double_abs = gps_longitude_double;
//                    if (gps_longitude_double < 0.0)
//                    {
//                        gps_longitude_double_abs *= -1.0;
//                        fmt = "-%d.%d";
//                    }
//                    double coord_int = floor(gps_longitude_double_abs);
//                    double coord_frac = (gps_longitude_double_abs - coord_int)*1000000;
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, fmt, ((int)coord_int), ((int)coord_frac));
//                    LCD_Puts(0, 1, screen_buffer);
//                    }
//                    break;
//                case SCREEN_GPS_LOCATOR:
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "Lcator:");
//                    LCD_Puts(1, 0, screen_buffer);
//                    LCD_Puts(0, 1, gps_locator);
//                    break;
//                case SCREEN_GPS_ALTITUDE:
//                    {
//                        double alt_int = floor(gps_msl_altitude);
//                        double alt_frac = (gps_msl_altitude - alt_int)*10;
//                        LCD_Puts(1, 0, "Alt.:");
//                        snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%d.%d", ((int)alt_int), ((int)alt_frac));
//                        LCD_Puts(0, 1, screen_buffer);
//                    }
//                    break;
//                case SCREEN_GPS_GEOID:
//                    {
//                        double geoid_int = floor(gps_geoid_separation);
//                        double geoid_frac = (gps_geoid_separation - geoid_int)*10;
//                        LCD_Puts(1, 0, "Geoid:");
//                        snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%d.%d", ((int)geoid_int), ((int)geoid_frac));
//                        LCD_Puts(0, 1, screen_buffer);
//                    }
//                    break;
//                case SCREEN_GPS_SATELITES:
//                    LCD_Puts(1, 0, "Sat. #:");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%02d", num_sats);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_GPS_HDOP:
//                    LCD_Puts(1, 0, "HDOP:");
//                    LCD_Puts(0, 1, gps_hdop);
//                    break;
//                case SCREEN_GPS_BAUDRATE:
//                    LCD_Puts(1, 0, menu_level == 1 ? "GPS BR:":"GPS BR?");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", ee_storage.gps_baudrate);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_GPS_ERRORS:
//                    LCD_Puts(1, 0, "GPS Err");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld/%ld/%ld", gps_invalid_frames, gps_fifo_overflow_gps, gps_fifo_overflow_comm);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_GPS_TIME_OFFSET:
//                    LCD_Puts(1, 0, menu_level == 1 ? "TZ ofs:":"TZ ofs?");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%2d", (int)gps_time_offset);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_GPS_MODEL:
//                    LCD_Puts(1, 0, menu_level == 1 ? "Model:":"Model?");
//                    switch(ee_storage.gps_model)
//                    {
//                        case GPS_MODEL_ATGM336H:
//                            LCD_Puts(0, 1, "ATGM336H");
//                            break;
//                        case GPS_MODEL_NEO6M:
//                            LCD_Puts(0, 1, "NEO-6M");
//                            break;
//                        case GPS_MODEL_NEOM9N:
//                            LCD_Puts(0, 1, "NEO-M9N");
//                            break;
//                        default:
//                        case GPS_MODEL_UNKNOWN:
//                            LCD_Puts(0, 1, menu_level == 1 ? "Unknown":"Auto");
//                            break;
//                    }
//                    break;
//                case SCREEN_GPS_LAST_FRAME:
//                    LCD_Puts(1, 0, "Frame:");
//                    LCD_Puts(0, 1, gps_last_frame);
//                    if(gps_last_frame_changed)
//                    {
//                        menu_force_redraw();
//                        gps_last_frame_changed = false;
//                    }
//                    break;
//                case SCREEN_GPS_EXIT:
//                    LCD_Puts(1, 0, "Exit?");
//                    LCD_Puts(0, 1, "        ");
//                    break;
//            }
//        }
//        break;
//    case SCREEN_UPTIME:
//        LCD_Puts(1, 0, "UPTIME:");
//        LCD_Puts(0, 1, "        ");
//        snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", device_uptime);
//        LCD_Puts(0, 1, screen_buffer);
//        break;
//    case SCREEN_FRAMES:
//        LCD_Puts(1, 0, "GGA FR:");
//        LCD_Puts(0, 1, "        ");
//        snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", gga_frames);
//        LCD_Puts(0, 1, screen_buffer);
//        break;
//    case SCREEN_BRIGHTNESS:
//        LCD_Puts(1, 0, menu_level == 0 ? "BRIGHT:":"BRIGHT?");
//        LCD_Puts(0, 1, "        ");
//        snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%d", ee_storage.brightness);
//        LCD_Puts(0, 1, screen_buffer);
//        break;
//    case SCREEN_PPS:
//        // Screen with pps
//        // Clear line 2
//        LCD_Puts(0, 1, "        ");
//        if(menu_level == 0)
//        {
//            snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "PPS:%3ld", pps_sync_count);
//            LCD_Puts(1, 0, screen_buffer);
//            snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", pps_error);
//            LCD_Puts(0, 1, screen_buffer);
//        }
//        else
//        {
//            switch (current_menu_pps_screen)
//            {
//                default:
//                case SCREEN_PPS_SHIFT:
//                    LCD_Puts(1, 0, "Shift:");
//                    // Check we have enough space for minus sign
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", (pps_error < -9999999) ? abs(pps_error) : pps_error);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPS_SHIFT_MS:
//                    LCD_Puts(1, 0, "Sft ms:");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld.%04d", pps_millis / 10000, abs(pps_millis) % 10000);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPS_SYNC_COUNT:
//                    LCD_Puts(1, 0, "SynCnt:");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", pps_sync_count);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPS_SYNC_MODE:
//                    LCD_Puts(1, 0, menu_level == 1 ? "Sync.:":"Sync.?");
//                    LCD_Puts(0, 1, ee_storage.pps_sync_on ? "      ON" : "     OFF");
//                    break;
//                case SCREEN_PPS_SYNC_DELAY:
//                    LCD_Puts(1, 0, menu_level == 1 ? "Delay:":"Delay?");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", ee_storage.pps_sync_delay);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPS_SYNC_THRESHOLD:
//                    LCD_Puts(1, 0, menu_level == 1 ? "Thrsld:":"Thrsld?");
//                    snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", ee_storage.pps_sync_threshold);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPS_FORCE_SYNC:
//                    if(menu_level == 1)
//                    {
//                        LCD_Puts(1, 0,  " Force ");
//                        LCD_Puts(0, 1, "  sync ?");
//                    }
//                    else
//                    {
//                        LCD_Puts(1, 0,  " Forced");
//                        LCD_Puts(0, 1, "  sync !");
//                        sync_pps_out = true;
//                        menu_level = 1;
//                    }
//                    break;
//                case SCREEN_PPS_EXIT:
//                    LCD_Puts(1, 0, "Exit?");
//                    LCD_Puts(0, 1, "        ");
//                    break;
//            }
//        }
//        break;
//    case SCREEN_SAVE_CONFIG:
//        //  Save configuration screen
//        if(menu_level == 0)
//        {
//            LCD_Puts(1, 0,  "Save   ");
//            LCD_Puts(0, 1, "Settings");
//        }
//        else
//        {
//            LCD_Puts(0, 0, " PRESS  ");
//            LCD_Puts(0, 1, "TO SAVE ");
//        }
//        break;
//    case SCREEN_VERSION:
//        if(menu_level == 0)
//        {
//            LCD_Puts(1, 0, "Vers.:");
//            LCD_Puts(0, 1, FIRMWARE_VERSION);
//        }
//        else
//        {
//            // Print MCU info
//            snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "MCU %dK", *(uint16_t*)FLASHSIZE_BASE);
//            LCD_Puts(0, 0, screen_buffer);
//            snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%08lX", DBGMCU->IDCODE);
//            LCD_Puts(0, 1, screen_buffer);
//        }
//        break;
//    }
//}
//
//void menu_run()
//{
//    // Detect rotary encoder value change
//    uint32_t new_encoder_value = TIM4->CNT / 2;
//
//#ifdef ROTARY_INVERT
//    new_encoder_value = -new_encoder_value;
//#endif
//
//    if(new_encoder_value != last_encoder_value)
//    {
//        menu_screen previous_menu_screen = current_menu_screen;
//        int encoder_increment = (new_encoder_value < last_encoder_value)? -1 : +1;
//        // Handle overflow cases
//        if(new_encoder_value == 32767 && last_encoder_value == 0)
//        {
//            encoder_increment = -1;
//        }
//        else if (new_encoder_value == 0 && last_encoder_value == 32767)
//        {
//            encoder_increment = +1;
//        }
//        if(menu_level == 0)
//        {   // Main menu => change menu screen
//            current_menu_screen =  (current_menu_screen + encoder_increment) % SCREEN_MAX;
//
//            if(current_menu_screen >= SCREEN_MAX) current_menu_screen = SCREEN_MAX-1; // Roll over for first sceen - 1
//
//            // Reset counter for date/time screen
//            last_hour_date_screen_update = HAL_GetTick();
//            LCD_Clear();
//            menu_force_redraw();
//        }
//        else if(menu_level == 1)
//        {   // Sub menu
//            switch(current_menu_screen)
//            {
//                case SCREEN_TREND:
//                    {
//                        // Trend view => change trend menu
//                        current_menu_trend_screen =  (current_menu_trend_screen + encoder_increment) % SCREEN_TREND_MAX;
//                        if(current_menu_trend_screen >= SCREEN_TREND_MAX) current_menu_trend_screen = SCREEN_TREND_MAX-1; // Roll over for first sceen - 1
//                        LCD_Clear();
//                        menu_force_redraw();
//                    }
//                    break;
//                case SCREEN_PWM:
//                case SCREEN_SAVE_CONFIG:
//                case SCREEN_VERSION:
//                    // Go back to main menu
//                    LCD_Clear();
//                    menu_force_redraw();
//                    menu_level = 0;
//                    break;
//                case SCREEN_PPB:
//                    {
//                        // PPB view => change ppb menu
//                        current_menu_ppb_screen =  (current_menu_ppb_screen + encoder_increment) % SCREEN_PPB_MAX;
//                        if(current_menu_ppb_screen >= SCREEN_PPB_MAX) current_menu_ppb_screen = SCREEN_PPB_MAX-1; // Roll over for first sceen - 1
//                        LCD_Clear();
//                        menu_force_redraw();
//                    }
//                    break;
//                case SCREEN_GPS:
//                    {
//                        // GPS view => change gps menu
//                        current_menu_gps_screen =  (current_menu_gps_screen + encoder_increment) % SCREEN_GPS_MAX;
//                        if(current_menu_gps_screen >= SCREEN_GPS_MAX) current_menu_gps_screen = SCREEN_GPS_MAX-1; // Roll over for first sceen - 1
//                        LCD_Clear();
//                        menu_force_redraw();
//                    }
//                    break;
//                case SCREEN_BRIGHTNESS:
//                    // Update brightness
//                    ee_storage.brightness += encoder_increment*5;
//                    if(ee_storage.brightness < 5) ee_storage.brightness = 5;
//                    if(ee_storage.brightness > 100) ee_storage.brightness = 100;
//                    ee_is_changed = true;
//                    update_brightness();
//                    LCD_Clear();
//                    menu_force_redraw();
//                    break;
//                case SCREEN_PPS:
//                    {
//                        // PPB view => change ppb menu
//                        current_menu_pps_screen =  (current_menu_pps_screen + encoder_increment) % SCREEN_PPS_MAX;
//                        if(current_menu_pps_screen >= SCREEN_PPS_MAX) current_menu_pps_screen = SCREEN_PPS_MAX-1; // Roll over for first sceen - 1
//                        LCD_Clear();
//                        menu_force_redraw();
//                    }
//                    break;
//                default:
//                    break;
//            }
//        }
//        else if(menu_level == 2 && current_menu_screen == SCREEN_TREND)
//        {   // Sub-sub menu for TREND screen
//            switch(current_menu_trend_screen)
//            {
//                case SCREEN_TREND_MAIN:
//                    {
//                    // Update position
//                    int32_t new_trend_shift = trend_shift + (encoder_increment * ee_storage.trend_h_scale);
//                    trend_arrow = encoder_increment < 0 ? TREND_LEFT_CODE : TREND_RIGHT_CODE;
//                    if(new_trend_shift < 0)
//                    {
//                        trend_shift = 0;
//                        trend_arrow = TREND_LEFT_CODE;
//                    }
//                    else if(new_trend_shift >= TREND_MAX_SHIFT)
//                    {
//                        trend_shift = TREND_MAX_SHIFT;
//                        trend_arrow = TREND_RIGHT_CODE;
//                    }
//                    else
//                    {
//                        trend_shift = new_trend_shift;
//                    }
//                    LCD_Clear();
//                    menu_force_redraw();
//                    break;
//                    }
//                case SCREEN_TREND_AUTO_V:
//                    // Update mode
//                    ee_storage.trend_auto_v = !ee_storage.trend_auto_v;
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    break;
//                case SCREEN_TREND_AUTO_H:
//                    // Update mode
//                    ee_storage.trend_auto_h = !ee_storage.trend_auto_h;
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    break;
//                case SCREEN_TREND_V_SCALE:
//                    {
//                    // Update v scale
//                    uint32_t multiplier;
//                    if(ee_storage.trend_v_scale > 2000 || ((ee_storage.trend_v_scale == 2000) && (encoder_increment > 0)))
//                    {
//                        multiplier = 1000;
//                    }
//                    else if(ee_storage.trend_v_scale > 200 || ((ee_storage.trend_v_scale == 200) && (encoder_increment > 0)))
//                    {
//                        multiplier = 100;
//                    }
//                    else
//                    {
//                        multiplier = 10;
//                    }
//                    ee_storage.trend_v_scale += (multiplier*encoder_increment);
//                    ee_storage.trend_v_scale = menu_round_v_scale(ee_storage.trend_v_scale);
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    break;
//                    }
//                case SCREEN_TREND_H_SCALE:
//                    // Update v scale
//                    ee_storage.trend_h_scale = encoder_increment > 0 ? ee_storage.trend_h_scale * 2 : ee_storage.trend_h_scale/2;
//                    ee_storage.trend_h_scale = menu_round_h_scale(ee_storage.trend_h_scale);
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    break;
//                default:
//                    break;
//            }
//        }
//        else if(menu_level == 2 && current_menu_screen == SCREEN_PPB)
//        {   // Sub-sub menu for PPB screen
//            switch(current_menu_ppb_screen)
//            {
//                case SCREEN_PPB_OCXO_MODEL:
//                    { // Update model
//                    ee_storage.ocxo_model =  (ee_storage.ocxo_model + encoder_increment) % (OCXO_MODEL_UNKNOWN+1);
//                    if(ee_storage.ocxo_model > OCXO_MODEL_UNKNOWN) ee_storage.ocxo_model = OCXO_MODEL_UNKNOWN;
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    }
//                    break;
//                case SCREEN_PPB_WARMUP_TIME:
//                    { // Update ppb lock threshold
//                    int new_warmup_time = ee_storage.warmup_time_seconds + (encoder_increment);
//                    if(new_warmup_time < 0)
//                    {
//                        new_warmup_time = 0;
//                    }
//                    else if(new_warmup_time > 1000)
//                    {
//                        new_warmup_time = 1000;
//                    }
//                    ee_storage.warmup_time_seconds = new_warmup_time;
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    }
//                    break;
//                case SCREEN_PPB_ALGO:
//                    { // Update algorithm
//                    displayed_correction_algorithm =  (displayed_correction_algorithm + encoder_increment) % (CORRECTION_ALGO_ERIC_H_PLUS+1);
//                    if(displayed_correction_algorithm > CORRECTION_ALGO_ERIC_H_PLUS) displayed_correction_algorithm = CORRECTION_ALGO_ERIC_H_PLUS;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    }
//                    break;
//                case SCREEN_PPB_CORRECTION_FACTOR:
//                    { // Update correction factor
//                    ee_storage.correction_factor = increment_correction_factor_value(ee_storage.correction_algorithm,correction_factor,encoder_increment);
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    }
//                    break;
//                case SCREEN_PPB_AUTO_SAVE_PWM:
//                    // Update mode
//                    ee_settings.pwm_auto_save = !ee_settings.pwm_auto_save;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    break;
//                case SCREEN_PPB_AUTO_SYNC_PPS:
//                    // Update mode
//                    ee_settings.pps_ppm_auto_sync = !ee_settings.pps_ppm_auto_sync;
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    break;
//                case SCREEN_PPB_LOCK_THRESHOLD:
//                    { // Update ppb lock threshold
//                    int new_threshold = ee_storage.ppb_lock_threshold + (5*encoder_increment);
//                    if(new_threshold < 0)
//                    {
//                        new_threshold = 0;
//                    }
//                    else if(new_threshold > MAX_PPB_LOCK_THRESHOLD)
//                    {
//                        new_threshold = MAX_PPB_LOCK_THRESHOLD;
//                    }
//                    ee.storage.ppb_lock_threshold = new_threshold;
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    }
//                    break;
//                default:
//                    break;
//            }
//        }
//        else if(menu_level == 2 && current_menu_screen == SCREEN_GPS)
//        {   // Sub-sub menu for GPS screen
//            switch(current_menu_gps_screen)
//            {
//                case SCREEN_GPS_BAUDRATE:
//                    { // Update GPS module baudrate
//                        baudrate max_baudrate = BAUDRATE_MAX;
//                        switch (ee_storage.gps_model)
//                        {
//                            case GPS_MODEL_ATGM336H:
//                                max_baudrate = BAUDRATE_115200 + 1;
//                                break;
//                            default:
//                                break;
//                        }
//                        gps_baudrate_enum = (gps_baudrate_enum + encoder_increment) % max_baudrate;
//                        if(gps_baudrate_enum >= max_baudrate) gps_baudrate_enum = max_baudrate-1; // Roll over for first screen - 1
//                        ee_storage.gps_baudrate = menu_get_baudrate_value(gps_baudrate_enum);
//                        ee_is_changed = true;
//                         LCD_Clear();
//                        menu_force_redraw();
//                    }
//                    break;
//                case SCREEN_GPS_TIME_OFFSET:
//                    {   // Update time offset
//                        gps_time_offset += encoder_increment;
//                        if (gps_time_offset > GPS_MAX_TIME_OFFSET) {
//                            gps_time_offset = GPS_MIN_TIME_OFFSET;
//                        } 
//                        else if(gps_time_offset < GPS_MIN_TIME_OFFSET)
//                        {
//                            gps_time_offset = GPS_MAX_TIME_OFFSET;
//                        }
//
//                        LCD_Clear();
//                        menu_force_redraw();
//                    }
//                    break;
//                case SCREEN_GPS_MODEL:
//                    { // Update model
//                    ee_storage.gps_model =  (ee_storage.gps_model + encoder_increment) % (GPS_MODEL_UNKNOWN+1);
//                    if(ee_storage.gps_model > GPS_MODEL_UNKNOWN) ee_storage.gps_model = GPS_MODEL_UNKNOWN; // Roll over for first sceen - 1
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    }
//                    break;
//                default:
//                    break;
//            }
//        }
//        else if(menu_level == 2 && current_menu_screen == SCREEN_PPS)
//        {   // Sub-sub menu for PPS screen
//            switch(current_menu_pps_screen)
//            {
//                case SCREEN_PPS_SYNC_MODE:
//                    // Update mode
//                    ee_storage.pps_sync_on = !ee_storage.pps_sync_on;
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    break;
//                case SCREEN_PPS_SYNC_DELAY:
//                    // Update delay
//                    ee_storage.pps_sync_delay += encoder_increment;
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    break;
//                case SCREEN_PPS_SYNC_THRESHOLD:
//                    // Update threshold
//                    ee_storage.pps_sync_threshold += encoder_increment;
//                    ee_is_changed = true;
//                    LCD_Clear();
//                    menu_force_redraw();
//                    break;
//                case SCREEN_PPS_FORCE_SYNC:
//                    // PPB view => change ppb menu
//                    current_menu_pps_screen =  (current_menu_pps_screen + encoder_increment) % SCREEN_PPS_MAX;
//                    if(current_menu_pps_screen >= SCREEN_PPS_MAX) current_menu_pps_screen = SCREEN_PPS_MAX-1; // Roll over for first sceen - 1
//                    LCD_Clear();
//                    menu_force_redraw();
//                    break;
//                default:
//                    break;
//            }
//        }
//        last_encoder_value = new_encoder_value;
//    }
//
//    if (rotary_get_click()) {
//        if (menu_level == 0) {
//            switch(current_menu_screen)
//            {
//                case SCREEN_TREND:
//                case SCREEN_PPB:
//                case SCREEN_GPS:
//                case SCREEN_PWM:
//                case SCREEN_BRIGHTNESS:
//                case SCREEN_PPS:
//                case SCREEN_SAVE_CONFIG:
//                case SCREEN_VERSION:
//                    menu_level = 1;
//                    LCD_Clear();
//                    break;
//                default:
//                    break;
//            }
//        } else  if (menu_level == 1){
//            switch(current_menu_screen)
//            {
//                case SCREEN_TREND:
//                    switch(current_menu_trend_screen)
//                    {
//                        case SCREEN_TREND_AUTO_H:
//                        case SCREEN_TREND_AUTO_V:
//                        case SCREEN_TREND_MAIN:
//                            menu_level = 2;
//                            break;
//                        case SCREEN_TREND_V_SCALE:
//                            // Prevent editing v scale if auto-v is on
//                            menu_level = ee_storage.trend_auto_v ? 1 : 2;
//                            break;
//                        case SCREEN_TREND_H_SCALE:
//                            // Prevent editing h scale if auto-h is on
//                            menu_level = ee_storage.trend_auto_h ? 1 : 2;
//                            break;
//                        case SCREEN_TREND_EXIT:
//                        default:
//                            // Go back to main screen to prevent returning to exit screen
//                            current_menu_trend_screen = SCREEN_TREND_MAIN;
//                            menu_level = 0;
//                            break;
//                    }
//                    break;
//                case SCREEN_PWM:
//                    ee_storage.pwm = TIM1->CCR2;
//                    ee_is_changed = true;
//                    menu_level = 0;
//                    break;
//                case SCREEN_BRIGHTNESS:
//                    menu_level = 0;
//                    break;
//                case SCREEN_PPB:
//                    switch(current_menu_ppb_screen)
//                    {
//                        case SCREEN_PPB_OCXO_MODEL:
//                        case SCREEN_PPB_WARMUP_TIME:
//                        case SCREEN_PPB_ALGO:
//                        case SCREEN_PPB_CORRECTION_FACTOR:
//                        case SCREEN_PPB_AUTO_SAVE_PWM:
//                        case SCREEN_PPB_AUTO_SYNC_PPS:
//                        case SCREEN_PPB_LOCK_THRESHOLD:
//                            menu_level = 2;
//                            break;
//                        case SCREEN_PPB_EXIT:
//                            // Go back to main screen to prevent returning to exit screen
//                            current_menu_ppb_screen = SCREEN_PPB_MEAN;
//                            menu_level = 0;
//                            break;
//                        default:
//                            menu_level = 0;
//                            break;
//                    }
//                    break;
//                case SCREEN_GPS:
//                    switch(current_menu_gps_screen)
//                    {
//                        case SCREEN_GPS_MODEL:
//                        case SCREEN_GPS_DATE_FORMAT:
//                        case SCREEN_GPS_TIME_OFFSET:
//                        case SCREEN_GPS_BAUDRATE:
//                            menu_level = 2;
//                            break;
//                        case SCREEN_GPS_EXIT:
//                            // Go back to main screen to prevent returning to exit screen
//                            current_menu_gps_screen = SCREEN_GPS_TIME;
//                            menu_level = 0;
//                            break;
//                        default:
//                            menu_level = 0;
//                            break;
//                    }
//                    break;
//                case SCREEN_PPS:
//                    switch(current_menu_pps_screen)
//                    {
//                        case SCREEN_PPS_SYNC_MODE:
//                        case SCREEN_PPS_SYNC_DELAY:
//                        case SCREEN_PPS_SYNC_THRESHOLD:
//                        case SCREEN_PPS_FORCE_SYNC:
//                            menu_level = 2;
//                            break;
//                        case SCREEN_PPS_EXIT:
//                            // Go back to main screen to prevent returning to exit screen
//                            current_menu_pps_screen = SCREEN_PPS_SHIFT;
//                            menu_level = 0;
//                            break;
//                        default:
//                            menu_level = 0;
//                            break;
//                    }
//                    break;
//                case SCREEN_SAVE_CONFIG:
//                    save_config();
//                    menu_level = 0;
//                    break;
//                default:
//                    menu_level = 0;
//                    break;
//            }
//            LCD_Clear();
//        } else  if (menu_level == 2 && current_menu_screen == SCREEN_TREND){
//            switch(current_menu_trend_screen)
//            {
//                case SCREEN_TREND_AUTO_V:
//                    break;
//                case SCREEN_TREND_AUTO_H:
//                    break;
//                case SCREEN_TREND_V_SCALE:
//                    break;
//                case SCREEN_TREND_H_SCALE:
//                    break;
//                default:
//                    break;
//            }
//            menu_level = 1;
//            LCD_Clear();
//        } else  if (menu_level == 2 && current_menu_screen == SCREEN_PPB){
//            switch(current_menu_ppb_screen)
//            {
//                case SCREEN_PPB_OCXO_MODEL:
//                    // Alsa change warmup time accordingly
//                    ee_storage.warmup_time_seconds = get_default_warmup_time(ee_storage.ocxo_model);
//                    ee_is_changed = true;
//                    break;
//                case SCREEN_PPB_WARMUP_TIME:
//                    break;
//                case SCREEN_PPB_ALGO:
//                    if(ee_storage.correction_algorithm != displayed_correction_algorithm)
//                    {   // Make sure correction algo and correction are consistant before activating new algo
//                        // Reset correction factor to default value when algo is changed
//                        ee_storage.correction_factor = get_default_correction_factor(displayed_correction_algorithm);
//                        ee_storage.correction_algorithm = displayed_correction_algorithm;
//                        ee_is_changed = true;
//                    }
//                    break;
//                case SCREEN_PPB_CORRECTION_FACTOR:
//                    break;
//                case SCREEN_PPB_AUTO_SAVE_PWM:
//                    break;
//                case SCREEN_PPB_AUTO_SYNC_PPS:
//                    break;
//                case SCREEN_PPB_LOCK_THRESHOLD:
//                    break;
//                default:
//                    break;
//            }
//            menu_level = 1;
//            LCD_Clear();
//        } else  if (menu_level == 2 && current_menu_screen == SCREEN_GPS){
//            switch(current_menu_gps_screen)
//            {
//                case SCREEN_GPS_BAUDRATE:
//                        // Reconfigure GPS module
//                        if(gps_change_module_baudrate(ee_storage.gps_baudrate)>=0)
//                        {   // Reconfigure UART
//                            gps_reconfigure_gps_uart(ee_storage.gps_baudrate);
//                        }
//                    break;
//                case SCREEN_GPS_TIME_OFFSET:
//                    if(ee_storage.gps_time_offset != ((uint32_t)(gps_time_offset-MIN_TIME_OFFSET)))
//                    {   // Save changes
//                        ee_storage.gps_time_offset = gps_time_offset-MIN_TIME_OFFSET;
//                        ui_on_config_changed();
//                    }
//                    break;
//                case SCREEN_GPS_MODEL:
//                    break;
//                default:
//                    break;
//            }
//            menu_level = 1;
//            LCD_Clear();
//        } else  if (menu_level == 2 && current_menu_screen == SCREEN_PPS){
//            switch(current_menu_pps_screen)
//            {
//                case SCREEN_PPS_SYNC_MODE:
//                    break;
//                case SCREEN_PPS_SYNC_DELAY:
//                        ui_on_config_changed();
//                    break;
//                case SCREEN_PPS_SYNC_THRESHOLD:
//                    break;
//                default:
//                    break;
//            }
//            menu_level = 1;
//            LCD_Clear();
//        }
//        else
//        {
//            menu_level = 0;
//            LCD_Clear();
//        }
//        menu_force_redraw();
//    }
//
//    if (refresh_screen) {
//        refresh_screen = false;
//
//        // Display state icon
//        if(current_menu_screen == SCREEN_TREND && (current_state_icon < 8))
//        {   // Don't use custom icon in trend screen since all 8 custom chars are used for graphic display
//            uint8_t icon;
//            LCD_PutCustom(0,0,icon);
//        }
//        else
//        {
//            LCD_PutCustom(0,0,current_state_icon);
//        }
//        
//        // Update PPB trend if needed
//        if(update_trend)
//        {
//            add_trend_value(abs(frequency_get_ppb()));
//            update_trend = false;
//        }
//
//        menu_draw();
//
//        // Check if we need resync or PWM save
//        if(frequency_is_stable(0))
//        {   // Frequency is stabilized
//            // Save PWM if requested
//            bool did_pwm = false;
//            bool did_pps = false;
//            if(ee_settings.pwm_auto_save && !auto_save_pwm_done)
//            {
//                ee_storage.pwm = TIM1->CCR2;
//                ee_is_changed = true;
//                // Only auto-save once per session
//                auto_save_pwm_done = true;
//                did_pwm = true;
//            }
//            if(ee_settings.pps_ppm_auto_sync && !auto_sync_pps_done)
//            {
//                sync_pps_out = true;
//                // Only auto-sync once per session
//                auto_sync_pps_done = true;
//                did_pps = true;
//            }
//            if(did_pps && did_pwm)
//            {
//                LCD_Puts(0, 0, "PPS&PWM ");
//                LCD_Puts(0, 1, " DONE!  ");
//            }
//            else if(did_pps)
//            {
//                LCD_Puts(0, 0, "  PPS   ");
//                LCD_Puts(0, 1, "SYNCED! ");
//            }
//            else if(did_pwm)
//            {
//                LCD_Puts(0, 0, "  PWM   ");
//                LCD_Puts(0, 1, "  SET!  ");
//            }
//        }
//        bool new_ppb_lock_status = frequency_is_stable(ee_storage.ppb_lock_threshold);
//        if(ppb_lock_status != new_ppb_lock_status )
//        {   // Update PPB lock status
//            ppb_lock_status = new_ppb_lock_status;
//            HAL_GPIO_WritePin(PPB_LOCK_OUTPUT_GPIO_Port, PPB_LOCK_OUTPUT_Pin, !ppb_lock_status); // Active low
//        }
//    }
//}
