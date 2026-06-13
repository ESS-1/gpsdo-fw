#ifndef _UI_78145532759C_H_
#define _UI_78145532759C_H_

#include <stddef.h>
#include <stdint.h>

void ui_format_ppb_5char(int32_t ppb_signed, char* buffer, size_t bufferSize);
const char* ui_get_month_name_3char(uint8_t month_num);

#endif
