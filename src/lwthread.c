/**
 * @file lwthread.c
 * @brief Main implementation file for the lwthread library
 */

#include "lwthread/lwthread.h"
#include "thread.h"
#include "scheduler.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define CLOCK_REALTIME 0

/* Create a new scheduler */
lwt_scheduler_t* lwt_scheduler_create(int num_threads) {
    if (num_threads <= 0 || num_threads > LWT_MAX_WORKERS) {
        errno = EINVAL;
        return NULL;
    }
    
    /* Allocate scheduler */
    lwt_scheduler_t* scheduler = malloc(sizeof(lwt_scheduler_t));
    if (!scheduler) {
        return NULL;
    }
    
    /* Initialize scheduler */
    if (lwt_scheduler_init(scheduler, num_threads) != 0) {
        free(scheduler);
        return NULL;
    }
    return scheduler;
}

/* Destroy a scheduler */
void lwt_scheduler_destroy(lwt_scheduler_t* scheduler) {
    if (!scheduler) {
        return;
    }
    
    lwt_scheduler_stop(scheduler);
    lwt_scheduler_cleanup(scheduler);
    free(scheduler);
}

/* Start the scheduler */
void lwt_scheduler_start(lwt_scheduler_t* scheduler) {
    if (!scheduler || scheduler->running_flag) {
        return;
    }
    
    pthread_mutex_lock(&scheduler->mutex);
    scheduler->running_flag = 1;
    pthread_mutex_unlock(&scheduler->mutex);
    
    for (int i = 0; i < scheduler->num_workers; i++) {
        pthread_create(&scheduler->workers[i], NULL, 
                       lwt_worker_function, &scheduler->worker_ids[i]);
    }
}

/* Stop the scheduler */
void lwt_scheduler_stop(lwt_scheduler_t* scheduler) {
    if (!scheduler || !scheduler->running_flag) {
        return;
    }
    
    /* Clear running flag */
    pthread_mutex_lock(&scheduler->mutex);
    scheduler->running_flag = 0;
    pthread_cond_broadcast(&scheduler->cond);
    pthread_mutex_unlock(&scheduler->mutex);
    
    /* Wait for workers to finish */
    for (int i = 0; i < scheduler->num_workers; i++) {
        pthread_join(scheduler->workers[i], NULL);
    }
}

/* Create a new lightweight thread */
lwt_thread_t* lwt_create(lwt_scheduler_t* scheduler, lwt_func_t func, void* arg) {
    if (!scheduler || !func) {
        errno = EINVAL;
        return NULL;
    }
    
    /* Allocate thread */
    lwt_thread_t* thread = malloc(sizeof(lwt_thread_t));
    if (!thread) {
        return NULL;
    }
    
    /* Initialize thread */
    if (lwt_thread_init(thread, func, arg, scheduler, 0) != 0) {
        free(thread);
        return NULL;
    }
    
    /* Add to scheduler */
    if (lwt_scheduler_add_thread(scheduler, thread) != 0) {
        lwt_thread_cleanup(thread);
        free(thread);
        return NULL;
    }
    
    return thread;
}

/* Yield execution from current thread */
void lwt_yield(void) {
    /* Get current thread */
    lwt_thread_t* thread = lwt_thread_self();
    if (!thread) {
        return;  /* Not in a lightweight thread */
    }
    
    /* Get scheduler */
    lwt_scheduler_t* scheduler = thread->scheduler;
    
    /* Get worker ID */
    int worker_id = lwt_scheduler_get_worker_id();
    if (worker_id < 0 || worker_id >= scheduler->num_workers) {
        return;
    }
    
    pthread_mutex_lock(&scheduler->mutex);
    
    /* If thread is not finished, add back to ready queue */
    if (thread->state != LWT_STATE_FINISHED) {
        thread->state = LWT_STATE_READY;
        lwt_queue_push_locked(&scheduler->ready_queue, thread);
    }
    
    /* Clear running thread */
    scheduler->running[worker_id] = NULL;
    
    /* Signal that there's a thread ready to run */
    pthread_cond_signal(&scheduler->cond);
    
    pthread_mutex_unlock(&scheduler->mutex);
    
    /* Switch back to scheduler */
    swapcontext(&thread->context, &scheduler->main_contexts[worker_id]);
}

/* Wait for a thread to complete */
void lwt_join(lwt_thread_t* thread) {
    if (!thread) {
        return;
    }
    
    /* Get current thread */
    lwt_thread_t* self = lwt_thread_self();
    if (!self) {
        return;  /* Not in a lightweight thread */
    }
    
    /* Get scheduler */
    lwt_scheduler_t* scheduler = self->scheduler;
    
    /* Get worker ID */
    int worker_id = lwt_scheduler_get_worker_id();
    if (worker_id < 0 || worker_id >= scheduler->num_workers) {
        return;
    }
    
    pthread_mutex_lock(&scheduler->mutex);
    
    /* If thread is already finished, we're done */
    if (thread->state == LWT_STATE_FINISHED) {
        pthread_mutex_unlock(&scheduler->mutex);
        return;
    }
    
    /* Otherwise, block until thread finishes */
    self->state = LWT_STATE_BLOCKED;
    thread->waiting = self;
    
    /* Clear running thread */
    scheduler->running[worker_id] = NULL;
    
    pthread_mutex_unlock(&scheduler->mutex);
    
    /* Switch back to scheduler */
    swapcontext(&self->context, &scheduler->main_contexts[worker_id]);
}

/* Get the current thread */
lwt_thread_t* lwt_current(void) {
    return lwt_thread_self();
}

/* Sleep for the specified duration */
void lwt_sleep(unsigned int ms) {
    /* Get current thread */
    lwt_thread_t* thread = lwt_thread_self();
    if (!thread) {
        /* Not in a lightweight thread, use regular sleep */
        struct timespec ts;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000000;
        nanosleep(&ts, NULL);
        return;
    }
    
    /* Get scheduler */
    lwt_scheduler_t* scheduler = thread->scheduler;
    
    /* Get worker ID */
    int worker_id = lwt_scheduler_get_worker_id();
    if (worker_id < 0 || worker_id >= scheduler->num_workers) {
        return;
    }
    
    /* Calculate wake time */
    struct timespec wake_time;
    clock_gettime(CLOCK_REALTIME, &wake_time);
    wake_time.tv_sec += ms / 1000;
    wake_time.tv_nsec += (ms % 1000) * 1000000;
    if (wake_time.tv_nsec >= 1000000000) {
        wake_time.tv_sec += 1;
        wake_time.tv_nsec -= 1000000000;
    }
    
    pthread_mutex_lock(&scheduler->mutex);
    
    /* Mark thread as blocked */
    thread->state = LWT_STATE_BLOCKED;
    
    /* Clear running thread */
    scheduler->running[worker_id] = NULL;
    
    pthread_mutex_unlock(&scheduler->mutex);
    
    /* Sleep for the requested time */
    struct timespec remaining;
    nanosleep(&wake_time, &remaining);
    
    /* Add thread back to ready queue */
    pthread_mutex_lock(&scheduler->mutex);
    thread->state = LWT_STATE_READY;
    lwt_queue_push_locked(&scheduler->ready_queue, thread);
    pthread_cond_signal(&scheduler->cond);
    pthread_mutex_unlock(&scheduler->mutex);
    
    /* Switch back to scheduler */
    swapcontext(&thread->context, &scheduler->main_contexts[worker_id]);
}