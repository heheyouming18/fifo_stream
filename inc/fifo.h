/*
 * This file is part of the fifo Library.
 *
 * Copyright (c) 2015-2019, Armink, <armink.ztl@gmail.com>
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
 * Function: It is an head file for this library. You can see all be called functions.
 * Created on: 2015-04-28
 */

#ifndef __FIFO_H__
#define __FIFO_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* fifo software version number */
#define FIFO_SW_VERSION                      "2.2.99"
/* buffer size for asynchronous output mode */
#define OUTPUT_BUF_SIZE           (1024 * 8)

/* fifo error code */
typedef enum {
    FIFO_NO_ERR,
} FifoErrCode;

typedef struct {
    /* Callback to pop out fifo data, user should impliment this function */
    void (*fp_fifo_pop)(const char *log, size_t size); 
} FifoCallbacks;

/* fifo.c */
FifoErrCode fifo_init(FifoCallbacks *callbacks);
void fifo_deinit(void);
void fifo_start(void);
void fifo_stop(void);

void fifo_push(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __FIFO_H__ */
