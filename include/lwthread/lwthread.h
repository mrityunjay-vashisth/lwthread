/**
 * @file lwthread.h
 * @brief Lightweight threading library public API
 *
 * This library provides a lightweight threading system inspired by Go's
 * goroutines, allowing for efficient concurrency with minimal overhead.
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#ifndef LWTHREAD_H
#define LWTHREAD_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Opaque type definitions */
typedef struct lwt_thread lwt_thread_t;
typedef struct lwt_scheduler lwt_scheduler_t;

/**
 * Function type for thread entry points
 */
typedef void (*lwt_func_t)(void* arg);

/**
 * Creates a new scheduler with the specified number of worker threads
 * 
 * @param num_threads Number of OS threads to create (similar to GOMAXPROCS)
 * @return Pointer to scheduler or NULL on error
 */
lwt_scheduler_t* lwt_scheduler_create(int num_threads);

/**
 * Destroys a scheduler and all its resources
 * 
 * @param scheduler Scheduler to destroy
 */
void lwt_scheduler_destroy(lwt_scheduler_t* scheduler);

/**
 * Starts the scheduler and begins executing threads
 * 
 * @param scheduler Scheduler to start
 */
void lwt_scheduler_start(lwt_scheduler_t* scheduler);

/**
 * Stops the scheduler
 * 
 * @param scheduler Scheduler to stop
 */
void lwt_scheduler_stop(lwt_scheduler_t* scheduler);

/**
 * Creates a new lightweight thread
 * 
 * @param scheduler Scheduler that will manage this thread
 * @param func Function to execute
 * @param arg Argument to pass to the function
 * @return Pointer to thread or NULL on error
 */
lwt_thread_t* lwt_create(lwt_scheduler_t* scheduler, lwt_func_t func, void* arg);

/**
 * Yields execution from current thread to another
 */
void lwt_yield(void);

/**
 * Waits for a thread to complete
 * 
 * @param thread Thread to wait for
 */
void lwt_join(lwt_thread_t* thread);

/**
 * Get the current thread
 * 
 * @return Pointer to current thread or NULL if not in a thread
 */
lwt_thread_t* lwt_current(void);

/**
 * Sleep for the specified duration
 * 
 * @param ms Milliseconds to sleep
 */
void lwt_sleep(unsigned int ms);

#ifdef __cplusplus
}
#endif

#endif /* LWTHREAD_H */