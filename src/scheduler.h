/**
 * @file scheduler.h
 * @brief Internal scheduler implementation details
 * @internal This header is not part of the public API
 * This library provides a lightweight threading system inspired by Go's
 * goroutines, allowing for efficient concurrency with minimal overhead.
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#ifndef LWTHREAD_SCHEDULER_INTERNAL_H
#define LWTHREAD_SCHEDULER_INTERNAL_H

#include "queue.h"
#include "thread.h"
#include <pthread.h>
#include <ucontext.h>

/**
 * Maximum number of worker threads
 */
#define LWT_MAX_WORKERS 64

/**
 * Scheduler structure
 */
struct lwt_scheduler {
    lwt_thread_queue_t ready_queue;                 /* Queue of ready threads */
    pthread_t workers[LWT_MAX_WORKERS];             /* OS worker threads */
    int worker_ids[LWT_MAX_WORKERS];                /* Worker thread IDs */
    int num_workers;                                /* Number of worker threads */
    struct lwt_thread* running[LWT_MAX_WORKERS];    /* Currently running threads */
    ucontext_t main_contexts[LWT_MAX_WORKERS];      /* Main contexts for workers */
    pthread_mutex_t mutex;                          /* Mutex for scheduler state */
    pthread_cond_t cond;                            /* Condition for signaling workers */
    int running_flag;                               /* Whether scheduler is running */
    int next_thread_id;                             /* For generating unique thread IDs */
};

/**
 * Worker thread function
 * @param arg Worker thread argument (pointer to worker ID)
 * @return Always returns NULL
 */
void* lwt_worker_function(void* arg);

/**
 * Initialize the scheduler
 * 
 * @param scheduler Scheduler to initialize
 * @param num_workers Number of worker threads
 * @return 0 on success, -1 on failure
 */
int lwt_scheduler_init(struct lwt_scheduler* scheduler, int num_workers);

/**
 * Clean up scheduler resources
 * 
 * @param scheduler Scheduler to clean up
 */
void lwt_scheduler_cleanup(struct lwt_scheduler* scheduler);

/**
 * Add a thread to the scheduler's ready queue
 * 
 * @param scheduler Scheduler to add to
 * @param thread Thread to add
 * @return 0 on success, -1 on failure
 */
int lwt_scheduler_add_thread(struct lwt_scheduler* scheduler, struct lwt_thread* thread);

/**
 * Get the worker ID for the current thread
 * 
 * @return Worker ID or -1 if not a worker
 */
int lwt_scheduler_get_worker_id(void);

/**
 * Set the worker ID for the current thread
 * 
 * @param id Worker ID
 */
void lwt_scheduler_set_worker_id(int id);

#endif /* LWTHREAD_SCHEDULER_INTERNAL_H */