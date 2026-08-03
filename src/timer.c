#include "main.h"
#include "timer.h"

void timer_reset(uint32_t* last_execution_time)
{
    // Set the last execution time to the current tick
    *last_execution_time = HAL_GetTick();
}

bool timer_is_elapsed(uint32_t* last_execution_time, uint32_t interval_ms, bool strict_mode)
{
    uint32_t current_time = HAL_GetTick();

    // Check if the difference between current and last tick is less than the interval.
    // Unsigned arithmetic safely handles system tick roll-over.
    if ((current_time - (*last_execution_time)) < interval_ms) {
        return false; // Interval has not elapsed yet
    }

    // Interval has elapsed; update the timestamp based on the mode
    if (strict_mode) {
        // Strict mode: increment by exact interval
        // If execution was heavily delayed, subsequent calls will return true instantly
        *last_execution_time += interval_ms;
    } else {
        // Relative mode: sync to current time (drift is allowed)
        *last_execution_time = current_time;
    }

    return true;
}
