#ifndef _FREQUENCY_76AB08914AC0_H_
#define _FREQUENCY_76AB08914AC0_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TARGET_FREQ 72000000
#define CIRCULAR_BUFFER_LEN 128

#define PPB_UNSET_VALUE     0x7FFFFFFF

// PPB lock threshold (*100)
#define DEFAULT_PPB_LOCK_THRESHOLD  50
#define MAX_PPB_LOCK_THRESHOLD      1000

typedef enum {
    FREQ_STABILITY_UNSTABLE = 0,
    FREQ_STABILITY_MARGINAL,
    FREQ_STABILITY_STABLE,
} FrequencyStability;

extern volatile int32_t            frequency_ppb_x100; // ppb * 100
extern volatile FrequencyStability frequency_stability;

typedef struct circbuf_t {
    size_t  write;
    int32_t buf[CIRCULAR_BUFFER_LEN];
} circbuf_t;

extern volatile circbuf_t circular_buffer;

void    circbuf_add(volatile circbuf_t* circbuf, int32_t val);
int32_t circbuf_sum(volatile circbuf_t* circbuf);

void    frequency_start_backlight();
void    frequency_start_tracking();
int32_t frequency_get_error();
void    frequency_allow_adjustment(bool allow);
bool    frequency_adjustment_allowed();

// Returns ppb * 100
int32_t frequency_get_inst_ppb_x100();

void frequency_update_ppb_and_stability();

#endif
