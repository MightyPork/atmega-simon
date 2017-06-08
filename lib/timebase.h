//
// Created by MightyPork on 2017/06/08.
//

#ifndef TIMEBASE_H
#define TIMEBASE_H

/**
 * To use the Timebase functionality,
 * set up SysTick to 1 kHz and call
 * timebase_ms_cb() in the IRQ.
 *
 * If you plan to use pendable future tasks,
 * also make sure you call run_pending_tasks()
 * in your main loop.
 *
 * This is not needed for non-pendable tasks.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/** Task PID. */
typedef uint8_t task_pid_t;

/** Time value in ms */
typedef uint16_t ms_time_t;

extern volatile ms_time_t time_ms;

#define TIMEBASE_PERIODIC_COUNT 5
#define TIMEBASE_FUTURE_COUNT 5


// PID value that can be used to indicate no task
#define PID_NONE 0

/** Loop until timeout - use in place of while() or for(). break and continue work too! */
#define until_timeout(to_ms) for(uint32_t _utmeo = ms_now(); ms_elapsed(_utmeo) < (to_ms);)

/** Retry a call until a timeout. Variable 'suc' is set to the return value. Must be defined. */
#define retry_TO(to_ms, call)  \
    until_timeout(to_ms) { \
        suc = call; \
        if (suc) break; \
    }

/** Must be called every 1 ms */
void timebase_ms_cb(void);


// --- Periodic -----------------------------------------------

/**
 * @brief Add a periodic task with an arg.
 * @param callback : task callback
 * @param arg      : callback argument
 * @param interval : task interval (ms)
 * @param enqueue  : put on the task queue when due
 * @return task PID
 */
task_pid_t add_periodic_task(void (*callback)(void *), void *arg, ms_time_t interval, bool enqueue);


/** Destroy a periodic task. */
bool remove_periodic_task(task_pid_t pid);

/** Enable or disable a periodic task. Returns true on success. */
bool enable_periodic_task(task_pid_t pid, bool cmd);

/** Check if a periodic task exists and is enabled. */
bool is_periodic_task_enabled(task_pid_t pid);

/** Reset timer for a task */
bool reset_periodic_task(task_pid_t pid);

/** Set inteval */
bool set_periodic_task_interval(task_pid_t pid, ms_time_t interval);


// --- Future -------------------------------------------------


/**
 * @brief Schedule a future task, with uint32_t argument.
 * @param callback : task callback
 * @param arg      : callback argument
 * @param delay    : task delay (ms)
 * @param enqueue  : put on the task queue when due
 * @return task PID
 */
task_pid_t schedule_task(void (*callback_arg)(void *), void *arg, ms_time_t delay, bool enqueue);


/** Abort a scheduled task. */
bool abort_scheduled_task(task_pid_t pid);


// --- Waiting functions --------------------------------------

/** Get milliseconds elapsed since start timestamp */
static inline ms_time_t ms_elapsed(ms_time_t start)
{
	return time_ms - start;
}

/** Delay N ms */
static inline void delay_ms(ms_time_t ms)
{
	ms_time_t start = time_ms;
	while ((time_ms - start) < ms); // overrun solved by unsigned arithmetic
}

/** Get current timestamp. */
static inline ms_time_t ms_now(void)
{
	return time_ms;
}

/** Seconds delay */
static inline void delay_s(uint16_t s)
{
	while (s-- != 0) {
		delay_ms(1000);
	}
}

/**
 * @brief Check if time since `start` elapsed.
 *
 * If so, sets the *start variable to the current time.
 *
 * Example:
 *
 *   ms_time_t s = ms_now();
 *
 *   while(1) {
 *     if (ms_loop_elapsed(&s, 100)) {
 *       // this is called every 100 ms
 *     }
 *     // ... rest of the loop ...
 *   }
 *
 * @param start     start time variable
 * @param duration  delay length
 * @return delay elapsed; start was updated.
 */
bool ms_loop_elapsed(ms_time_t *start, ms_time_t duration);

#endif //TIMEBASE_H
