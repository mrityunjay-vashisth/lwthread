/**
 * @file queue.h
 * @brief Internal thread queue implementation
 * @internal This header is not part of the public API
 * This library provides a lightweight threading system inspired by Go's
 * goroutines, allowing for efficient concurrency with minimal overhead.
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#ifndef LWTHREAD_QUEUE_INTERNAL_H
#define LWTHREAD_QUEUE_INTERNAL_H

#include <pthread.h>

/**
 * Thread queue structure
 */
typedef struct lwt_thread_queue {
    struct lwt_thread* head;   /* First thread in the queue */
    struct lwt_thread* tail;   /* Last thread in the queue */
    pthread_mutex_t mutex;     /* Queue lock */
    int count;                 /* Number of threads in the queue */
} lwt_thread_queue_t;

/**
 * Initialize a thread queue
 * 
 * @param queue Queue to initialize
 * @return 0 on success, -1 on failure
 */
int lwt_queue_init(lwt_thread_queue_t* queue);

/**
 * Destroy a thread queue
 * 
 * @param queue Queue to destroy
 */
void lwt_queue_destroy(lwt_thread_queue_t* queue);

/**
 * Push a thread onto the queue
 * 
 * @param queue Queue to push to
 * @param thread Thread to push
 * @return 0 on success, -1 on failure
 */
int lwt_queue_push(lwt_thread_queue_t* queue, struct lwt_thread* thread);

/**
 * Push a thread onto the queue with the lock already held
 * 
 * @param queue Queue to push to
 * @param thread Thread to push
 */
void lwt_queue_push_locked(lwt_thread_queue_t* queue, struct lwt_thread* thread);

/**
 * Pop a thread from the queue
 * 
 * @param queue Queue to pop from
 * @return Thread or NULL if queue is empty
 */
struct lwt_thread* lwt_queue_pop(lwt_thread_queue_t* queue);

/**
 * Pop a thread from the queue with the lock already held
 * 
 * @param queue Queue to pop from
 * @return Thread or NULL if queue is empty
 */
struct lwt_thread* lwt_queue_pop_locked(lwt_thread_queue_t* queue);

/**
 * Check if queue is empty
 * 
 * @param queue Queue to check
 * @return 1 if empty, 0 if not empty
 */
int lwt_queue_empty(lwt_thread_queue_t* queue);

/**
 * Get queue size
 * 
 * @param queue Queue to check
 * @return Number of items in the queue
 */
int lwt_queue_size(lwt_thread_queue_t* queue);

#endif /* LWTHREAD_QUEUE_INTERNAL_H */