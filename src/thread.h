/**
 * @file thread.h
 * @brief Internal thread implementation details
 * @internal This header is not part of the public API
 * This library provides a lightweight threading system inspired by Go's
 * goroutines, allowing for efficient concurrency with minimal overhead.
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#ifndef LWTHREAD_THREAD_INTERNAL_H
#define LWTHREAD_THREAD_INTERNAL_H

#include "lwthread/lwthread.h"
#include <ucontext.h>


/**
 * Thread states
 */
typedef enum {
    LWT_STATE_NEW,      /* Thread has been created but not started */
    LWT_STATE_READY,    /* Thread is ready to run */
    LWT_STATE_RUNNING,  /* Thread is currently running */
    LWT_STATE_BLOCKED,  /* Thread is blocked (e.g., on join) */
    LWT_STATE_FINISHED  /* Thread has completed execution */
} lwt_state_t;

/* Forward declaration */
struct lwt_scheduler;

/**
 * Internal thread structure definition
 */
struct lwt_thread {
    ucontext_t context;                 /* Thread context (registers, stack, etc.) */
    void* stack;                        /* Thread stack */
    size_t stack_size;                  /* Size of the stack */
    lwt_state_t state;                  /* Current state */
    lwt_func_t func;                    /* Function to execute */
    void* arg;                          /* Argument to the function */
    struct lwt_thread* next;            /* For queue management */
    struct lwt_thread* waiting;         /* Thread waiting on this one (for join) */
    struct lwt_scheduler* scheduler;    /* Back-reference to scheduler */
    int id;                             /* Unique thread ID */
};

/**
 * Initialize thread structure
 * 
 * @param thread Thread to initialize
 * @param func Function to execute
 * @param arg Argument to the function
 * @param stack_size Size of the stack to allocate
 * @param scheduler 
 * @return 0 on success, -1 on failure
 */
int lwt_thread_init(struct lwt_thread* thread, lwt_func_t func, void* arg, 
    struct lwt_scheduler* scheduler, size_t stack_size);

/**
 * Clean up thread resources
 * 
 * @param thread Thread to clean up
 */
void lwt_thread_cleanup(struct lwt_thread* thread);

/**
 * Get the current thread
 * 
 * @return Current thread or NULL if not in a thread
 */
struct lwt_thread* lwt_thread_self(void);

/**
 * Set the current thread (for internal use)
 * 
 * @param thread Thread to set as current
 */
void lwt_thread_set_current(struct lwt_thread* thread);

#endif /* LWTHREAD_THREAD_INTERNAL_H */