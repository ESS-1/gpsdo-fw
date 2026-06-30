#ifndef _TIMER_5AB4F8B44E12_H_
#define _TIMER_5AB4F8B44E12_H_

#include <stdint.h>
#include <stdbool.h>

// Resets the timer by setting its timestamp to the current tick.
// Call this function to ensure that the timer will block execution
// for at least the specified interval from the moment of this call.
void timer_reset(uint32_t* last_execution_time);

// Checks if the specified interval has elapsed.
// If the interval has elapsed, this function automatically updates the
// referenced timestamp to the current tick.
// Returns true if the interval has elapsed, false otherwise.
bool timer_is_elapsed(uint32_t* last_execution_time, uint32_t interval_ms);

#endif
