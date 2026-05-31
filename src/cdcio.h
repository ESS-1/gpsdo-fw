#ifndef _CDCIO_H_
#define _CDCIO_H_

#include <stdint.h>

// USB CDC status
typedef enum { CDC_STATUS_OK = 0, CDC_STATUS_NO_CONN, CDC_STATUS_OVERFLOW } cdc_status;
extern volatile cdc_status cdcio_status;

void cdcio_transmit(const uint8_t* buf, uint16_t len);

#endif
