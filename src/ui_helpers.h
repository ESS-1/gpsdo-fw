#ifndef _UI_78145532759C_H_
#define _UI_78145532759C_H_

#include <stddef.h>
#include <stdint.h>

void ui_format_ppb_5char(int32_t ppb_signed, char* buffer, size_t bufferSize);
const char* ui_get_month_name_3char(uint8_t month_num);
void ui_change_setting_i8(int8_t* value, int32_t step, int8_t min, int8_t max);
void ui_change_setting_u16(uint16_t* value, int32_t step, uint16_t max);

#endif
