#ifndef _UI_MSGBOX_EA9F35B40337_H_
#define _UI_MSGBOX_EA9F35B40337_H_

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

// The 'message' array MUST NOT be allocated on the stack (as a local non-static variable).
// Since screen transitions are deferred and processed asynchronously in ui_run(), the calling
// function will return and its stack frame will be destroyed before the message box is initialized.
// Always declare the message array as 'static' (e.g., 'static const char* const msg[]') or global.
bool ui_msgbox(const char* const message[], UI_MsgBoxType type, UI_MsgBoxButton selected_button, UI_MsgBoxHandler handler);

#endif
