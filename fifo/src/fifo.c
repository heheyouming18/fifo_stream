/*
 * This file is part of the fifo Library.
 *
 * Copyright (c) 2015-2018, Armink, <armink.ztl@gmail.com>
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
 * Function: Initialize function and other general function.
 * Created on: 2015-04-28
 */

#define LOG_TAG      "fifo"

#include <fifo.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/* buffer size for every line's log */
#define FIFO_ONE_MSG_MAX_SIZE                       1024*8

/* fifo */
typedef struct {
    bool init_ok;
    bool output_enabled;
    bool output_lock_enabled;
    bool output_is_locked_before_enable;
    bool output_is_locked_before_disable;
}fifo, *Fifo_t;

/* log ring buffer write index */
static size_t write_index = 0;
/* log ring buffer read index */
static size_t read_index = 0;
/* log ring buffer full flag */
static bool buf_is_full = false;
/* log ring buffer empty flag */
static bool buf_is_empty = true;

/* fifo object */
static fifo s_fifo;
static char log_buf[FIFO_ONE_MSG_MAX_SIZE] = { 0 };
FifoCallbacks usr_cbs;

static void fifo_set_output_enabled(bool enabled);
static void fifo_output_lock_enabled(bool enabled);
extern void fifo_async_put_notice(void);
/**
 * fifo initialize.
 *
 * @return result
 */
FifoErrCode fifo_init(FifoCallbacks *cb) {

    usr_cbs.fp_fifo_pop = cb->fp_fifo_pop; // add callback for output

    FifoErrCode result = FIFO_NO_ERR;

    if (s_fifo.init_ok == true) {
        return result;
    }
    extern FifoErrCode fifo_async_init(void);
    result = fifo_async_init();
    if (result != FIFO_NO_ERR) {
        return result;
    }

    /* enable the output lock */
    fifo_output_lock_enabled(true);
    /* output locked status initialize */
    s_fifo.output_is_locked_before_enable = false;
    s_fifo.output_is_locked_before_disable = false;
    s_fifo.init_ok = true;

    return result;
}

/**
 * fifo deinitialize.
 *
 */
void fifo_deinit(void) {

    if (!s_fifo.init_ok) {
        return ;
    }
    extern FifoErrCode fifo_async_deinit(void);
    fifo_async_deinit();

    s_fifo.init_ok = false;
}


/**
 * fifo start after initialize.
 */
void fifo_start(void) {
    if (!s_fifo.init_ok) {
        return ;
    }
    
    /* enable output */
    fifo_set_output_enabled(true);
    /* show version */
    printf("fifo V%s is initialize success.", FIFO_SW_VERSION);
}

/**
 * fifo stop after initialize.
 */
void fifo_stop(void) {
    if (!s_fifo.init_ok) {
        return ;
    }

    /* disable output */
    fifo_set_output_enabled(false);

    /* show version */
    printf("fifo V%s is deinitialize success.", FIFO_SW_VERSION);
}


/**
 * set output enable or disable
 *
 * @param enabled TRUE: enable FALSE: disable
 */
static void fifo_set_output_enabled(bool enabled) {
    s_fifo.output_enabled = enabled;
}

extern void fifo_platform_output_lock(void);
extern void fifo_platform_output_unlock(void);
/**
 * lock output 
 */
void fifo_output_lock(void) {
    if (s_fifo.output_lock_enabled) {
        fifo_platform_output_lock();
        s_fifo.output_is_locked_before_disable = true;
    } else {
        s_fifo.output_is_locked_before_enable = true;
    }
}

/**
 * unlock output
 */
void fifo_output_unlock(void) {
    if (s_fifo.output_lock_enabled) {
        fifo_platform_output_unlock();
        s_fifo.output_is_locked_before_disable = false;
    } else {
        s_fifo.output_is_locked_before_enable = false;
    }
}

/**
 * asynchronous output ring buffer used size
 *
 * @return used size
 */
static size_t fifo_async_get_buf_used(void) {
    if (write_index > read_index) {
        return write_index - read_index;
    } else {
        if (!buf_is_full && !buf_is_empty) {
            return OUTPUT_BUF_SIZE - (read_index - write_index);
        } else if (buf_is_full) {
            return OUTPUT_BUF_SIZE;
        } else {
            return 0;
        }
    }
}

/**
 * get log from asynchronous output ring buffer
 *
 * @param log get log buffer
 * @param size log size
 *
 * @return get log size, the log size is less than ring buffer used size
 */
size_t fifo_async_get_log(char *log, size_t size) {
    size_t used = 0;
    /* lock output */
    fifo_output_lock();
    used = fifo_async_get_buf_used();
    /* no log */
    if (!used || !size) {
        size = 0;
        goto __exit;
    }
    /* less log */
    if (used <= size) {
        size = used;
        buf_is_empty = true;
    }

    if (read_index + size < OUTPUT_BUF_SIZE) {
        memcpy(log, log_buf + read_index, size);
        read_index += size;
    } else {
        memcpy(log, log_buf + read_index, OUTPUT_BUF_SIZE - read_index);
        memcpy(log + OUTPUT_BUF_SIZE - read_index, log_buf,
                size - (OUTPUT_BUF_SIZE - read_index));
        read_index += size - OUTPUT_BUF_SIZE;
    }

    buf_is_full = false;

__exit:
    /* lock output */
    fifo_output_unlock();
    return size;
}

/**
 * asynchronous output ring buffer remain space
 *
 * @return remain space
 */
static size_t async_get_buf_space(void) {
    return OUTPUT_BUF_SIZE - fifo_async_get_buf_used();
}

/**
 * put log to asynchronous output ring buffer
 *
 * @param log put log buffer
 * @param size log size
 *
 * @return put log size, the log which beyond ring buffer space will be dropped
 */
static size_t async_put_log(const char *log, size_t size) {
    size_t space = 0;

    space = async_get_buf_space();
    /* no space */
    if (!space) {
        size = 0;
        goto __exit;
    }
    /* drop some log */
    if (space <= size) {
        size = space;
        buf_is_full = true;
    }

    if (write_index + size < OUTPUT_BUF_SIZE) {
        memcpy(log_buf + write_index, log, size);
        write_index += size;
    } else {
        memcpy(log_buf + write_index, log, OUTPUT_BUF_SIZE - write_index);
        memcpy(log_buf, log + OUTPUT_BUF_SIZE - write_index,
                size - (OUTPUT_BUF_SIZE - write_index));
        write_index += size - OUTPUT_BUF_SIZE;
    }

    buf_is_empty = false;

__exit:

    return size;
}

void async_output_task(void *arg) {
    size_t get_log_size = 0;
    static char poll_get_buf[OUTPUT_BUF_SIZE - 4];

    extern bool thread_running;
    while(thread_running) {
        /* waiting log */
        void fifo_async_get_notice(void);
        fifo_async_get_notice(); // block until get notice
        /* polling gets and outputs the log */
        while(true) {
            extern size_t fifo_async_get_log(char *log, size_t size);
            get_log_size = fifo_async_get_log(poll_get_buf, OUTPUT_BUF_SIZE - 4);

            if (get_log_size) {
                extern FifoCallbacks usr_cbs;
                if(usr_cbs.fp_fifo_pop != NULL)
                    usr_cbs.fp_fifo_pop(poll_get_buf, get_log_size);
            } else {
                break;
            }
        }
    }
    // return NULL;
}

/**
 * output RAW format log
 *
 * @param format output format
 * @param ... args
 */
void fifo_push(const char *format, ...) {
    va_list args;
    size_t log_len = 0;
    int fmt_result;

    /* check output enabled */
    if (!s_fifo.output_enabled) {
        return;
    }

    /* args point to the first variable parameter */
    va_start(args, format);

    /* lock output */
    fifo_output_lock();

    /* package log data to buffer */
    fmt_result = vsnprintf(log_buf, FIFO_ONE_MSG_MAX_SIZE, format, args);

    /* output converted log */
    if ((fmt_result > -1) && (fmt_result <= FIFO_ONE_MSG_MAX_SIZE)) {
        log_len = fmt_result;
    } else {
        log_len = FIFO_ONE_MSG_MAX_SIZE;
    }
    /* put log to buffer */
    size_t put_size;
    put_size = async_put_log(log_buf, log_len);
    /* notify output log thread */
    if (put_size > 0) {
        /* this function must be implement by user when FIFO_ASYNC_OUTPUT_USING_PTHREAD is not defined */
        fifo_async_put_notice();
    }

    /* unlock output */
    fifo_output_unlock();

    va_end(args);
}

/**
 * enable or disable logger output lock
 * @note disable this lock is not recommended except you want output system exception log
 *
 * @param enabled true: enable  false: disable
 */
static void fifo_output_lock_enabled(bool enabled) {
    s_fifo.output_lock_enabled = enabled;
    /* it will re-lock or re-unlock before output lock enable */
    if (s_fifo.output_lock_enabled) {
        if (!s_fifo.output_is_locked_before_disable && s_fifo.output_is_locked_before_enable) {
            /* the output lock is unlocked before disable, and the lock will unlocking after enable */
            fifo_output_lock();
        } else if (s_fifo.output_is_locked_before_disable && !s_fifo.output_is_locked_before_enable) {
            /* the output lock is locked before disable, and the lock will locking after enable */
            fifo_output_unlock();
        }
    }
}
