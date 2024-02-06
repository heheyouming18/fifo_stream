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

#include <fifo.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include <string.h>
#include <sched.h>
#include <semaphore.h>

/* thread default stack size */
#if PTHREAD_STACK_MIN > 4*1024
#define FIFO_ASYNC_OUTPUT_PTHREAD_STACK_SIZE     PTHREAD_STACK_MIN
#else
#define FIFO_ASYNC_OUTPUT_PTHREAD_STACK_SIZE     (1*1024)
#endif
/* thread default priority */
#define FIFO_ASYNC_OUTPUT_PTHREAD_PRIORITY       (sched_get_priority_max(SCHED_RR) - 1)
/* output thread poll get log buffer size  */
#define FIFO_ASYNC_POLL_GET_LOG_BUF_SIZE         (OUTPUT_BUF_SIZE - 4)

static pthread_mutex_t output_lock;

/* asynchronous output log notice */
static sem_t output_notice;
/* asynchronous output pthread thread */
static pthread_t async_output_thread;

/* thread running flag */
static bool thread_running = false;
/* Initialize OK flag */
static bool init_ok = false;

// /**
//  * output log port interface
//  *
//  * @param log output of log
//  * @param size log size
//  */
// void fifo_pop(const char *log, size_t size) {
//     /* output to terminal */
//     printf("%.*s", (int)size, log);
// }

/**
 * fifo port initialize
 *
 * @return result
 */
FifoErrCode fifo_platform_init(void) {
    FifoErrCode result = FIFO_NO_ERR;
    pthread_mutex_init(&output_lock, NULL);
    return result;
}

/**
 * fifo port deinitialize
 *
 */
void fifo_platform_deinit(void) {
    pthread_mutex_destroy(&output_lock);
}

/**
 * output lock
 */
void fifo_platform_output_lock(void) {
    pthread_mutex_lock(&output_lock);
}

/**
 * output unlock
 */
void fifo_platform_output_unlock(void) {
    pthread_mutex_unlock(&output_lock);
}

void fifo_async_put_notice(void) {
    sem_post(&output_notice);
}

static void *async_output(void *arg) {
    size_t get_log_size = 0;
    static char poll_get_buf[FIFO_ASYNC_POLL_GET_LOG_BUF_SIZE];

    while(thread_running) {
        /* waiting log */
        sem_wait(&output_notice);
        /* polling gets and outputs the log */
        while(true) {
            extern size_t fifo_async_get_log(char *log, size_t size);
            get_log_size = fifo_async_get_log(poll_get_buf, FIFO_ASYNC_POLL_GET_LOG_BUF_SIZE);

            if (get_log_size) {
                extern FifoCallbacks usr_cbs;
                if(usr_cbs.fp_fifo_pop != NULL)
                    usr_cbs.fp_fifo_pop(poll_get_buf, get_log_size);
            } else {
                break;
            }
        }
    }
    return NULL;
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

    sem_init(&output_notice, 0, 0);

    thread_running = true;

    pthread_attr_init(&thread_attr);
    pthread_attr_setstacksize(&thread_attr, FIFO_ASYNC_OUTPUT_PTHREAD_STACK_SIZE);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
    thread_sched_param.sched_priority = FIFO_ASYNC_OUTPUT_PTHREAD_PRIORITY;
    pthread_attr_setschedparam(&thread_attr, &thread_sched_param);
    pthread_create(&async_output_thread, &thread_attr, async_output, NULL);
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
    
    sem_destroy(&output_notice);

    init_ok = false;
}

