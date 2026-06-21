#ifndef _UI_EA9F35B40337_H_
#define _UI_EA9F35B40337_H_

#include "ui_core.h"
#include <stdbool.h>

typedef enum {
    UI_MsgBoxButton_Ok,
    UI_MsgBoxButton_Yes,
    UI_MsgBoxButton_No,
    UI_MsgBoxButton_Back,
} UI_MsgBoxButton;

typedef enum {
    UI_MsgBoxType_Ok,
    UI_MsgBoxType_Error,
    UI_MsgBoxType_YesNo,
} UI_MsgBoxType;

typedef void (*UI_MsgBoxHandler)(UI_MsgBoxButton result);

bool ui_msgbox(const char* message[], UI_MsgBoxType type, UI_MsgBoxButton selected_button, UI_MsgBoxHandler handler);

#endif
