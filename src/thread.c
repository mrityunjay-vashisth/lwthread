/**
 * @file thread.c
 * @brief Thread implementation
 */

#include "thread.h"
#include "scheduler.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Default stack size: 64KB */
#define LWT_DEFAULT_STACK_SIZE (64 * 1024)

/* Thread-local storage for current thread */
static __thread struct lwt_thread* current_thread = NULL;

static void lwt_thread_start(void) {
    struct lwt_thread* thread = current_thread;
    if (NULL == thread) {
        return;
    }
    /* Execute the thread function */
    thread->func(thread->arg);
    pthread_mutex_lock(&thread->scheduler->mutex);
    thread->state = LWT_STATE_FINISHED;

    if (thread->waiting) {
        thread->waiting->state = LWT_STATE_READY;
        lwt_queue_push_locked(&thread->scheduler->ready_queue, thread->waiting);
        thread->waiting = NULL;
        pthread_cond_signal(&thread->scheduler->cond);
    }
    pthread_mutex_unlock(&thread->scheduler->mutex);
    lwt_yield();
}

int lwt_thread_init(struct lwt_thread* thread, lwt_func_t func, void* arg,
                    struct lwt_scheduler* scheduler, size_t stack_size) {
    if (NULL == thread || NULL == func || NULL == scheduler) {
        errno = EINVAL;
        return -1;
    }

    if (0 == stack_size) {
        stack_size = LWT_DEFAULT_STACK_SIZE;
    }

    memset(thread, 0, sizeof(struct lwt_thread));
    thread->func = func;
    thread->arg = arg;
    thread->scheduler = scheduler;
    thread->state = LWT_STATE_NEW;
    thread->stack_size = stack_size;
    thread->stack = malloc(stack_size);
    if (NULL == thread->stack) {
        return -1;
    }

    if (getcontext(&thread->context) == -1) {
        free(thread->stack);
        thread->stack = NULL;
        return -1;
    }

    /*
     * Configure thread execution context and stack
     * 
     * - Assigns the pre-allocated stack memory to this thread's context
     * - Sets the stack size to the specified allocation size
     * - Sets uc_link to NULL as we're implementing custom context switching
     *   rather than using the default chain-of-execution mechanism
     * 
     * This setup is required before calling makecontext() to initialize
     * the thread with its entry function.
     */
    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = stack_size;
    thread->context.uc_link = NULL;

    makecontext(&thread->context, lwt_thread_start, 0);
    pthread_mutex_lock(&scheduler->mutex);
    thread->id = scheduler->next_thread_id++;
    pthread_mutex_unlock(&scheduler->mutex);
    return 0;
}

void lwt_thread_cleanup(struct lwt_thread* thread) {
    if (NULL == thread) {
        return;
    }
    
    if (thread->stack) {
        free(thread->stack);
        thread->stack = NULL;
    }
}

struct lwt_thread* lwt_thread_self(void) {
    return current_thread;
}

void lwt_thread_set_current(struct lwt_thread* thread) {
    current_thread = thread;
}