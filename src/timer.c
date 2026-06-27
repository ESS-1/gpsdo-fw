#include "main.h"
#include "timer.h"

void timer_reset(uint32_t* last_execution_time)
{
    // Set the last execution time to the current tick
    *last_execution_time = HAL_GetTick();
}

bool timer_is_elapsed(uint32_t* last_execution_time, uint32_t interval_ms)
{
    uint32_t current_time = HAL_GetTick();

    // Check if the difference between current and last tick is less than the interval.
    // Unsigned arithmetic safely handles system tick roll-over.
    if (current_time - (*last_execution_time) < interval_ms) {
        return false; // Interval has not elapsed yet
    }

    // Interval has elapsed; update the timestamp for the next cycle
    *last_execution_time = current_time;
    return true;
}
