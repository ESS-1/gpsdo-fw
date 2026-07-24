#ifndef _UI_HELPERS_78145532759C_H_
#define _UI_HELPERS_78145532759C_H_

#include <stddef.h>
#include <stdint.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

void ui_format_ppb_5char(int32_t ppb_signed, char* buffer, size_t bufferSize);
void ui_format_ppb_9char(int32_t ppb_signed, char* buffer, size_t bufferSize);
const char* ui_get_month_name_3char(uint8_t month_num);

void ui_change_setting_i8(int8_t* value, int32_t step, int8_t min, int8_t max, bool wrap_around);
void ui_change_setting_u8(uint8_t* value, int32_t step, uint8_t min, uint8_t max, bool wrap_around);
void ui_change_setting_u16(uint16_t* value, int32_t step, uint16_t min, uint16_t max, bool wrap_around);
void ui_change_setting_u32(uint32_t* value, int32_t step, uint32_t min, uint32_t max, bool wrap_around);

int32_t ui_limit_i32(int32_t value, int32_t min, int32_t max);

int32_t ui_get_adaptive_step(uint32_t value, int32_t encoder_step);

#endif
