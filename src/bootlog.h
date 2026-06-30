#ifndef _BOOTLOG_58B39A7297C6_H_
#define _BOOTLOG_58B39A7297C6_H_

#include <stdbool.h>

void bootlog_init();
void bootlog_add(const char* msg);
void bootlog_set_status(bool status);
void bootlog_error(const char* msg);

#endif
