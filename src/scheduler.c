/**
 * @file scheduler.c
 * @brief Scheduler implementation
 */

#include "scheduler.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

/* Thread-local storage for worker ID */
static __thread int current_worker_id = -1;

void* lwt_worker_function(void* arg) {
    int* id_ptr = (int*)arg;
    int id = *id_ptr;
    struct lwt_scheduler* scheduler = NULL;

    lwt_scheduler_set_worker_id(id);

    /* Find scheduler - it will be set when we run the first thread */
    /* This is a bit of a hack, but works for our purposes */
    struct lwt_thread* thread = NULL;
    while (1) {
        if (NULL == scheduler) {
            sleep(1);
            continue;
        }
        pthread_mutex_lock(&scheduler->mutex);
        while (scheduler->running_flag && scheduler->ready_queue.head == NULL) {
            pthread_cond_wait(&scheduler->cond, &scheduler->mutex);
        }

        if (!scheduler->running_flag) {
            pthread_mutex_unlock(&scheduler->mutex);
            break;
        }
        
        thread = lwt_queue_pop_locked(&scheduler->ready_queue);
        if (thread) {
            thread->state = LWT_STATE_RUNNING;
            scheduler->running[id] = thread;
            pthread_mutex_unlock(&scheduler->mutex);
            lwt_thread_set_current(thread);
            swapcontext(&scheduler->main_contexts[id], &thread->context);
        } else {
            pthread_mutex_unlock(&scheduler->mutex);
        }
    }
    return NULL;
}

int lwt_scheduler_init(struct lwt_scheduler* scheduler, int num_workers) {
    if (NULL == scheduler || num_workers <= 0 || num_workers > LWT_MAX_WORKERS) {
        errno = EINVAL;
        return -1;
    }

    memset(scheduler, 0, sizeof(struct lwt_scheduler));
    scheduler->num_workers = num_workers;
    scheduler->running_flag = 0;
    scheduler->next_thread_id = 1;

    if (lwt_queue_init(&scheduler->ready_queue) != 0) {
        return -1;
    }

    if (pthread_mutex_init(&scheduler->mutex, NULL) != 0) {
        lwt_queue_destroy(&scheduler->ready_queue);
        return -1;
    }

    if (pthread_cond_init(&scheduler->cond, NULL) != 0) {
        pthread_mutex_destroy(&scheduler->mutex);
        lwt_queue_destroy(&scheduler->ready_queue);
        return -1;
    }

    for (int i = 0; i < num_workers; i++) {
        scheduler->worker_ids[i] = i;
    }
    return 0;
}

void lwt_scheduler_cleanup(struct lwt_scheduler* scheduler) {
    if (!scheduler) {
        return;
    }
    
    /* Clean up synchronization primitives */
    pthread_mutex_destroy(&scheduler->mutex);
    pthread_cond_destroy(&scheduler->cond);
    
    /* Clean up queue */
    lwt_queue_destroy(&scheduler->ready_queue);
}

int lwt_scheduler_add_thread(struct lwt_scheduler* scheduler, struct lwt_thread* thread) {
    if (!scheduler || !thread) {
        errno = EINVAL;
        return -1;
    }
    
    /* Add thread to ready queue */
    thread->state = LWT_STATE_READY;
    if (lwt_queue_push(&scheduler->ready_queue, thread) != 0) {
        return -1;
    }
    
    /* Signal workers that a new thread is ready */
    pthread_mutex_lock(&scheduler->mutex);
    pthread_cond_signal(&scheduler->cond);
    pthread_mutex_unlock(&scheduler->mutex);
    
    return 0;
}

int lwt_scheduler_get_worker_id(void) {
    return current_worker_id;
}

void lwt_scheduler_set_worker_id(int id) {
    current_worker_id = id;
}