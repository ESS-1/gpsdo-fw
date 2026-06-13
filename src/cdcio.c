#include "cdcio.h"
#include "usbd_cdc_if.h"
#include <stdbool.h>

#define CDCIO_OVERFLOW_DISPLAY_TIME_MS 5000U
#define CDCIO_TX_WAIT_BYTES_PER_MS     15U
#define CDCIO_TX_WAIT_MIN_MS           5U

volatile cdc_status cdcio_status;

static bool     cdcio_is_overflow    = false;
static uint32_t cdcio_overflow_start = 0;

static uint32_t cdcio_tx_wait_time_ms = 0;

static void cdcio_update_status(uint8_t result)
{
    switch (result)
    {
    case USBD_OK:
        if (!cdcio_is_overflow || (HAL_GetTick() - cdcio_overflow_start) >= CDCIO_OVERFLOW_DISPLAY_TIME_MS) {
            cdcio_is_overflow    = false;
            cdcio_overflow_start = 0;
            cdcio_status         = CDC_STATUS_OK;
        }
        break;

    case USBD_BUSY:
        if (!cdcio_is_overflow) {
            cdcio_is_overflow    = true;
            cdcio_overflow_start = HAL_GetTick();
        }
        cdcio_status = CDC_STATUS_OVERFLOW;
        break;

    case USBD_FAIL:
    default:
        cdcio_is_overflow    = false;
        cdcio_overflow_start = 0;
        cdcio_status         = CDC_STATUS_NO_CONN;
        break;
    }
}

static uint32_t cdcio_get_next_tx_timeout(uint16_t len)
{
    uint32_t timeout_ms = len / CDCIO_TX_WAIT_BYTES_PER_MS;

    if (timeout_ms < CDCIO_TX_WAIT_MIN_MS) {
        timeout_ms = CDCIO_TX_WAIT_MIN_MS;
    }

    return timeout_ms;
}

void cdcio_transmit(const uint8_t* buf, uint16_t len)
{
    uint8_t result = CDC_WaitTxReady_FS(cdcio_tx_wait_time_ms);
    if (result == USBD_OK) {
        result = CDC_TransmitBuffered_FS(buf, len);
    }

    if (result == USBD_OK) {
        // If data was added to the TX buffer, set a wait timeout for the next operation
        cdcio_tx_wait_time_ms = cdcio_get_next_tx_timeout(len);
    } else {
        // If transmission failed, set a zero timeout for the next transmission attempt
        cdcio_tx_wait_time_ms = 0;
    }

    cdcio_update_status(result);
}
