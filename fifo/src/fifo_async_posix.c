/*
 * This file is part of the fifo Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for linux.
 * Created on: 2015-04-28
 */

#if defined(__linux__)
#include <fifo.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include <string.h>
#include <sched.h>
#include <semaphore.h>

static pthread_mutex_t output_mutex_lock;
static sem_t output_notice_sem;

/* asynchronous output pthread thread */
static pthread_t async_output_thread;

/* thread running flag */
bool thread_running = false;
/* Initialize OK flag */
static bool init_ok = false;

/**
 * output lock
 */
void fifo_platform_output_lock(void) {
    pthread_mutex_lock(&output_mutex_lock);
}

/**
 * output unlock
 */
void fifo_platform_output_unlock(void) {
    pthread_mutex_unlock(&output_mutex_lock);
}

void fifo_async_put_notice(void) {
    sem_post(&output_notice_sem);
}

void fifo_async_get_notice(void) {
    sem_wait(&output_notice_sem);
}

/**
 * asynchronous output mode initialize
 *
 * @return result
 */
FifoErrCode fifo_async_init(void) {
    FifoErrCode result = FIFO_NO_ERR;

    if (init_ok) {
        return result;
    }

    pthread_attr_t thread_attr;
    struct sched_param thread_sched_param;

    sem_init(&output_notice_sem, 0, 0);
    pthread_mutex_init(&output_mutex_lock, NULL);

    thread_running = true;

    pthread_attr_init(&thread_attr);
    pthread_attr_setstacksize(&thread_attr, (1*1024));
    pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
    thread_sched_param.sched_priority = (sched_get_priority_max(SCHED_RR) - 1);
    pthread_attr_setschedparam(&thread_attr, &thread_sched_param);

    extern void async_output_task(void *arg);
    pthread_create(&async_output_thread, &thread_attr, (void *)async_output_task, NULL);
    pthread_attr_destroy(&thread_attr);

    init_ok = true;

    return result;
}

/**
 * asynchronous output mode deinitialize
 *
 */
void fifo_async_deinit(void) {
    if (!init_ok) {
        return ;
    }

    thread_running = false;

    fifo_async_put_notice();

    pthread_join(async_output_thread, NULL);
    
    sem_destroy(&output_notice_sem);
    pthread_mutex_destroy(&output_mutex_lock);

    init_ok = false;
}

#endif
