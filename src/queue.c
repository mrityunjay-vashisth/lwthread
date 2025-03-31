/**
 * @file queue.c
 * @brief Thread queue implementation
 */

#include "queue.h"
#include "thread.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int lwt_queue_init(lwt_thread_queue_t* queue) {
    if (NULL == queue) {
        errno = EINVAL;
        return -1;
    }

    memset(queue, 0, sizeof(lwt_thread_queue_t));
    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        return -1;
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    return 0;
}

void lwt_queue_destroy(lwt_thread_queue_t* queue) {
    if (NULL == queue) {
        return;
    }
    pthread_mutex_destroy(&queue->mutex);
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
}

int lwt_queue_push(lwt_thread_queue_t* queue, struct lwt_thread* thread) {
    if (NULL == queue || NULL == thread) {
        errno = EINVAL;
        return -1;
    }
    pthread_mutex_lock(&queue->mutex);
    lwt_queue_push_locked(queue, thread);
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

void lwt_queue_push_locked(lwt_thread_queue_t* queue, struct lwt_thread* thread) {
    thread->next = NULL;
    if (queue->tail == NULL) {
        queue->head = queue->tail = thread;
    } else {
        queue->tail->next = thread;
        queue->tail = thread;
    }
    queue->count++;
}

struct lwt_thread* lwt_queue_pop(lwt_thread_queue_t* queue) {
    if (NULL == queue) {
        errno = EINVAL;
        return NULL;
    }
    pthread_mutex_lock(&queue->mutex);
    struct lwt_thread* thread = lwt_queue_pop_locked(queue);
    pthread_mutex_unlock(&queue->mutex);
    return thread;
}

struct lwt_thread* lwt_queue_pop_locked(lwt_thread_queue_t* queue) {
    if (NULL == queue->head) {
        return NULL;
    }

    struct lwt_thread* thread = queue->head;
    queue->head = thread->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    thread->next = NULL;
    queue->count--;
    return thread;
}

int lwt_queue_empty(lwt_thread_queue_t* queue) {
    if (NULL == queue) {
        errno = EINVAL;
        return 1;
    }
    pthread_mutex_lock(&queue->mutex);
    int empty = (queue->head == NULL);
    pthread_mutex_unlock(&queue->mutex);
    return empty;
}

int lwt_queue_size(lwt_thread_queue_t* queue) {
    if (NULL == queue) {
        errno = EINVAL;
        return 0;
    }
    pthread_mutex_lock(&queue->mutex);
    int count = queue->count;
    pthread_mutex_unlock(&queue->mutex);
    return count;
}