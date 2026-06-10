
#include "trend8_t.h"

//// Possible baudrate values
//typedef enum { BAUDRATE_9600, BAUDRATE_19200, BAUDRATE_38400, BAUDRATE_57600, BAUDRATE_115200, BAUDRATE_230400, BAUDRATE_460800, BAUDRATE_921600, BAUDRATE_MAX} baudrate;
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
//        if(current_menu_screen == SCREEN_MAIN)
//        {
//            LCD_Puts(0, 1, gps_time);
//        }
//        else if(current_menu_screen == SCREEN_DATE)
//        {
//            LCD_Puts(0, 1, gps_date);
//        }
//        break;
//    case SCREEN_TREND:
//        // Trend screen 
//        if(menu_level == 0)
//        {
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
//            }
//        }
//        break;
//    case SCREEN_PPB:
//        // Screen with ppb
//        if(menu_level == 0)
//        {
//            ppb = frequency_ppb_x100;
//            LCD_Puts(1, 0, "PPB:   ");
//            menu_format_ppb(ppb, screen_buffer, SCREEN_BUFFER_SIZE);
//            LCD_Puts(0, 1, screen_buffer);
//        }
//        else
//        {
//            switch (current_menu_ppb_screen)
//            {
//                default:
//                case SCREEN_PPB_MEAN:
//                    ppb = frequency_ppb_x100;
//                    LCD_Puts(1, 0, "Mean:");
//                    menu_format_ppb(ppb, screen_buffer, SCREEN_BUFFER_SIZE);
//                    LCD_Puts(0, 1, screen_buffer);
//                    break;
//                case SCREEN_PPB_INST:
//                    {
//                    LCD_Puts(1, 0, "Inst:");
//                    int32_t ppb_inst = frequency_get_inst_ppb_x100();
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
//            }
//        }
//        break;
//    case SCREEN_GPS:
//        if(menu_level == 0)
//        {
//            LCD_Puts(0, 1, gps_time);
//        }
//        else
//        {
//            switch (current_menu_gps_screen)
//            {
//                default:
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
//            }
//        }
//        break;
//    case SCREEN_UPTIME:
//        LCD_Puts(1, 0, "UPTIME:");
//        snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", device_uptime);
//        LCD_Puts(0, 1, screen_buffer);
//        break;
//    case SCREEN_FRAMES:
//        LCD_Puts(1, 0, "GGA FR:");
//        snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%ld", gga_frames);
//        LCD_Puts(0, 1, screen_buffer);
//        break;
//    case SCREEN_BRIGHTNESS:
//        LCD_Puts(1, 0, menu_level == 0 ? "BRIGHT:":"BRIGHT?");
//        snprintf(screen_buffer, SCREEN_BUFFER_SIZE, "%d", ee_storage.brightness);
//        LCD_Puts(0, 1, screen_buffer);
//        break;
//    case SCREEN_PPS:
//        // Screen with pps
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
//            }
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
//    if(new_encoder_value != last_encoder_value)
//    {
//        if(menu_level == 0)
//        else if(menu_level == 1)
//        {   // Sub menu
//            switch(current_menu_screen)
//            {
//                case SCREEN_BRIGHTNESS:
//                    // Update brightness
//                    ee_storage.brightness += encoder_increment*5;
//                    if(ee_storage.brightness < 5) ee_storage.brightness = 5;
//                    if(ee_storage.brightness > 100) ee_storage.brightness = 100;
//                    ee_is_changed = true;
//                    update_brightness();
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
//                    break;
//                    }
//                case SCREEN_TREND_AUTO_V:
//                    // Update mode
//                    ee_storage.trend_auto_v = !ee_storage.trend_auto_v;
//                    ee_is_changed = true;
//                    break;
//                case SCREEN_TREND_AUTO_H:
//                    // Update mode
//                    ee_storage.trend_auto_h = !ee_storage.trend_auto_h;
//                    ee_is_changed = true;
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
//                    break;
//                    }
//                case SCREEN_TREND_H_SCALE:
//                    // Update v scale
//                    ee_storage.trend_h_scale = encoder_increment > 0 ? ee_storage.trend_h_scale * 2 : ee_storage.trend_h_scale/2;
//                    ee_storage.trend_h_scale = menu_round_h_scale(ee_storage.trend_h_scale);
//                    ee_is_changed = true;
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
//                    }
//                    break;
//                case SCREEN_PPB_ALGO:
//                    { // Update algorithm
//                    displayed_correction_algorithm =  (displayed_correction_algorithm + encoder_increment) % (CORRECTION_ALGO_ERIC_H_PLUS+1);
//                    if(displayed_correction_algorithm > CORRECTION_ALGO_ERIC_H_PLUS) displayed_correction_algorithm = CORRECTION_ALGO_ERIC_H_PLUS;
//                    }
//                    break;
//                case SCREEN_PPB_CORRECTION_FACTOR:
//                    { // Update correction factor
//                    ee_storage.correction_factor = increment_correction_factor_value(ee_storage.correction_algorithm,correction_factor,encoder_increment);
//                    ee_is_changed = true;
//                    }
//                    break;
//                case SCREEN_PPB_AUTO_SAVE_PWM:
//                    // Update mode
//                    ee_settings.pwm_auto_save = !ee_settings.pwm_auto_save;
//                    break;
//                case SCREEN_PPB_AUTO_SYNC_PPS:
//                    // Update mode
//                    ee_settings.pps_ppm_auto_sync = !ee_settings.pps_ppm_auto_sync;
//                    ee_is_changed = true;
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
//                    ee_storage.ppb_lock_threshold = new_threshold;
//                    ee_is_changed = true;
//                    }
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
//                        }
//                        gps_baudrate_enum = (gps_baudrate_enum + encoder_increment) % max_baudrate;
//                        if(gps_baudrate_enum >= max_baudrate) gps_baudrate_enum = max_baudrate-1; // Roll over for first screen - 1
//                        ee_storage.gps_baudrate = menu_get_baudrate_value(gps_baudrate_enum);
//                        ee_is_changed = true;
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
//                    }
//                    break;
//                case SCREEN_GPS_MODEL:
//                    { // Update model
//                    ee_storage.gps_model =  (ee_storage.gps_model + encoder_increment) % (GPS_MODEL_UNKNOWN+1);
//                    if(ee_storage.gps_model > GPS_MODEL_UNKNOWN) ee_storage.gps_model = GPS_MODEL_UNKNOWN; // Roll over for first sceen - 1
//                    ee_is_changed = true;
//                    }
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
//                    break;
//                case SCREEN_PPS_SYNC_DELAY:
//                    // Update delay
//                    ee_storage.pps_sync_delay += encoder_increment;
//                    ee_is_changed = true;
//                    break;
//                case SCREEN_PPS_SYNC_THRESHOLD:
//                    // Update threshold
//                    ee_storage.pps_sync_threshold += encoder_increment;
//                    ee_is_changed = true;
//                    break;
//            }
//        }
//    }
//
//    if (rotary_get_click()) {
//        if (menu_level == 0) {
//        } else  if (menu_level == 2 && current_menu_screen == SCREEN_PPB){
//            switch(current_menu_ppb_screen)
//            {
//                case SCREEN_PPB_OCXO_MODEL:
//                    // Alsa change warmup time accordingly
//                    ee_storage.warmup_time_seconds = get_default_warmup_time(ee_storage.ocxo_model);
//                    ee_is_changed = true;
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
//            }
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
//                        ee_is_changed = true;
//                    }
//                    break;
//            }
//        }
//    }
//
//    if (refresh_screen) {
//
//        // Update PPB trend if needed
//        if(update_trend)
//        {
//            add_trend_value(abs(frequency_ppb_x100));
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
//    }
//}
