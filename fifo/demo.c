/*
 * This file is part of the fifo Library.
 *
 * Copyright (c) 2015-2017, Armink, <armink.ztl@gmail.com>
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
 * Function: linux demo.
 * Created on: 2015-07-30
 */

#define LOG_TAG    "main"

#include <fifo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void fifo_pop(const char *log, size_t size) {
    /* output to terminal */
    printf("%.*s", (int)size, log);
}

static void test_fifo_push(void);

int main(void) {
    /* close printf buffer */
    setbuf(stdout, NULL);

    FifoCallbacks fifo_cb;
    fifo_cb.fp_fifo_pop = fifo_pop;

    /* initialize fifo */
    fifo_init(&fifo_cb);
    /* start fifo */
    fifo_start();

    /* test logger output */
    test_fifo_push();

    return EXIT_SUCCESS;
}

/**
 * fifo demo
 */
void test_fifo_push(void) {
    while(1) {
        fifo_push("Hello thread safe fifo!");
        sleep(5);
    }
}
