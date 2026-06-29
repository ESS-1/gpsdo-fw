#ifndef _INT_H_
#define _INT_H_

#include <stdbool.h>
#include <stdint.h>

extern volatile bool     allow_adjustment;
extern volatile uint32_t frequency;
extern volatile uint32_t num_samples;
extern volatile uint32_t device_uptime;
extern volatile uint32_t last_pps_out;
extern volatile bool     pps_out_up;
extern volatile int32_t  ppb_frequency;
extern volatile int32_t  ppb_frequency_error;
extern volatile int32_t  ppb_correction;
extern volatile int32_t  ppb_millis;
extern volatile int32_t  pps_error;
extern volatile int32_t  pps_millis;
extern volatile uint32_t pps_sync_count;
extern volatile bool     sync_pps_out;
extern volatile bool     update_trend;
extern volatile bool     gps_lock_status;

// For correction algorithms
// OCXO models
typedef enum { OCXO_MODEL_ISOTEMP, OCXO_MODEL_OX256B, OCXO_MODEL_UNKNOWN } ocxo_model_type;

// Correction algorithms
typedef enum { CORRECTION_ALGO_DANKAR, CORRECTION_ALGO_FREDZO, CORRECTION_ALGO_ERIC_H, CORRECTION_ALGO_ERIC_H_PLUS } correction_algo_type;

void update_brightness();

uint32_t get_default_correction_factor(correction_algo_type algo);

uint32_t increment_correction_factor_value(correction_algo_type algo, uint32_t value, int increment);

uint32_t get_default_warmup_time(ocxo_model_type model);

#endif
