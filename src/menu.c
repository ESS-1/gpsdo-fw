
// Char codes for trend view
// #define TREND_LEFT_CODE         0x7F
// #define TREND_RIGHT_CODE        0x7E

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
//
//#define PPB_STRING_SIZE     5
//#define SCREEN_BUFFER_SIZE  14
//
//static void menu_draw()
//{
//    char    screen_buffer[SCREEN_BUFFER_SIZE];
//    char    ppb_string[PPB_STRING_SIZE];
//
//    switch (current_menu_screen) {
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
//            }
//        }
//        break;
//    }
//}
//
//void menu_run()
//{
//    if(new_encoder_value != last_encoder_value)
//    {
//        if(menu_level == 2 && current_menu_screen == SCREEN_TREND)
//        {   // Sub-sub menu for TREND screen
//            switch(current_menu_trend_screen)
//            {
//                case SCREEN_TREND_MAIN:
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
//    }
//}
