#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TARGET_FREQ_HZ      70000000
#define CIRCULAR_BUFFER_LEN 128

volatile uint32_t num_samples   = 0;
volatile uint32_t device_uptime = 0;

typedef struct circbuf_t {
    size_t  write;
    int32_t buf[CIRCULAR_BUFFER_LEN];
} circbuf_t;

volatile circbuf_t circular_buffer = { 0 };

// Quick and dirty circular buffer
void circbuf_add(volatile circbuf_t* circbuf, int32_t val)
{
    circbuf->buf[circbuf->write] = val;
    circbuf->write               = (circbuf->write + 1) % CIRCULAR_BUFFER_LEN;

    if (num_samples < CIRCULAR_BUFFER_LEN)
        num_samples++;
}

int32_t circbuf_sum(volatile circbuf_t* circbuf)
{
    int32_t sum = 0;
    for (size_t i = 0; i < CIRCULAR_BUFFER_LEN; i++) {
        sum += circbuf->buf[i];
    }
    return sum;
}

int32_t frequency_get_ppb()
{
    if (num_samples == 0) {
        return 0x7FFFFFFF;
    }

    // Get ratio of cumulative error / expected number of cycles. Multiply by 1e9 for PPB and by
    // 100 to get additional digits without using floats.
    // This will be a running average over 128 seconds of the error in PPB*100
    return (int64_t)circbuf_sum(&circular_buffer) * 1000000000 * 100 / ((int64_t)70000000 * num_samples);
}

#define CORRECTION_FACTOR 300

int32_t alg(int32_t current_error)
{
    const float  ema_a      = 0.33f;
    static float ema_err_hz = 0.0f;

    int32_t current_ppb = frequency_get_ppb();
    int32_t adjustment  = 0;

    if (current_ppb != 0x7FFFFFFF) {
        ema_err_hz = ema_a * current_error + ((1 - ema_a) * ema_err_hz);

        if (abs(current_ppb) > 0) {
            const int factor   = CORRECTION_FACTOR;
            int       interval = 1;

            // Calculate adjustment.
            adjustment = -current_ppb / factor;
            if (adjustment == 0) {
                // Adjustment is less than 1 per interval.
                adjustment = current_ppb > 0 ? -1 : 1;
                interval   = factor / (abs(current_ppb) % factor);
            }

            if (ema_err_hz == 0 || ((ema_err_hz > 0) != (current_ppb > 0))) {
                adjustment = 0;
            }

            // Apply adjustment.
            if (device_uptime % interval == 0) {
                return adjustment;
            } else {
                return 0;
            }
        }
    }
    return adjustment;
}

int main()
{
    double   continuous_phase = 0.0;
    int32_t  pwm              = 0;
    uint32_t prev_capture     = 0;

    // Thermal drift parameters
    double initial_offset   = -25.0; // Frequency is off by 25Hz at start
    double warmup_drift_max = -30.0; // It will drift another 30Hz during warmup
    double warmup_tau       =   400; // Warmup time constant (400 seconds)

    printf("Time_s,Avg_PPB,Err_PPB,Err_Hz,Err_Ticks,Adj,PWM\n");

    for (int t = 0; t <= 3600; t++) { // Simulate 1 hour

        // Calculate current physical frequency
        // f = f_target + f_static_offset + f_thermal(t) + f_pwm
        double thermal_component = warmup_drift_max * exp(-(double)t / warmup_tau);
        double pwm_effect        = (double)pwm / 312;

        double current_real_f = TARGET_FREQ_HZ + initial_offset + thermal_component + pwm_effect;

        // We add the exact frequency to the total phase every second
        continuous_phase += current_real_f;

        // Hardware Capture (Simulating MCU Timer register)
        // This naturally truncates the fractional part, which stays for the next second
        uint32_t current_capture = (uint32_t)continuous_phase;

        // Calculate Delta between two PPS pulses
        int32_t captured_ticks = (int32_t)(current_capture - prev_capture);
        int32_t err_ticks      = (t == 0) ? 0 : (captured_ticks - TARGET_FREQ_HZ);
        prev_capture           = current_capture;

        ++device_uptime;
        circbuf_add(&circular_buffer, err_ticks);
        float avg_ppb = frequency_get_ppb()/100.0f;

        // Run the algorithm (except for the very first step)
        int32_t adj = alg(err_ticks);
        pwm += adj;

        double err_hz  = current_real_f - TARGET_FREQ_HZ;
        double err_ppb = err_hz / (TARGET_FREQ_HZ / 1000000000.0f);

        // Output results
        printf("%d,%.2f,%.2f,%.4f,%d,%d,%d\n", t, avg_ppb, err_ppb, err_hz, err_ticks, adj, pwm);
    }

    return 0;
}
