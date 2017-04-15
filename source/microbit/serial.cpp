/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George, 2017 Mark Shannon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

extern "C" {

#include "serial.h"
#include "py/mpstate.h"
#include "py/mphal.h"

#define MICROBIT_SERIAL_DEFAULT_BAUD_RATE 115200

static serial_t uart { 0 };
int interrupt_char = -1;

#define UART_RX_BUF_SIZE (64) // Use a large buffer so we can paste example code.

typedef struct _uart_buffer_t {
    uint8_t data[UART_RX_BUF_SIZE];
    volatile uint16_t head, tail;
} uart_buffer_t;

static void uart_buffer_init(uart_buffer_t *buf) {
    buf->head = buf->tail = 0;
}

/** This IS NOT safe to use outside of interrupt */
static inline void write_buf(uart_buffer_t *buf, uint8_t val) {
    uint16_t next_head = (buf->head + 1) % UART_RX_BUF_SIZE;
    if (next_head != buf->tail) {
        // only store data if room in buf
        buf->data[buf->head] = val;
        buf->head = next_head;
    }
}

/** This IS safe to use outside of interrupt */
static inline int read_buf(uart_buffer_t *buf) {
    __disable_irq();
    uint32_t tail = buf->tail;
    if (buf->head == tail) {
        __enable_irq();
        return -1;
    }
    uint32_t c = buf->data[tail];
    buf->tail = (tail + 1) % UART_RX_BUF_SIZE;
    __enable_irq();
    return c;
}

static inline int is_buf_empty(uart_buffer_t *buf) {
    return buf->tail == buf->head;
}

static uart_buffer_t microbit_rx_buffer;

void microbit_serial_set_interrupt_char(int c) {
    if (c != -1) {
        mp_obj_exception_clear_traceback(MP_STATE_PORT(keyboard_interrupt_obj));
    }
    interrupt_char = c;
}

static void rx_interrupt_handler(uint32_t id, SerialIrq event) {
    (void)id;
    if (event == TxIrq)
        return;
    int c = serial_getc(&uart);
    if (c == interrupt_char) {
        MP_STATE_VM(mp_pending_exception) = MP_STATE_PORT(keyboard_interrupt_obj);
    } else {
        write_buf(&microbit_rx_buffer, c);
    }
}

void microbit_serial_init(void) {
    interrupt_char = -1;
    uart_buffer_init(&microbit_rx_buffer);
    serial_init(&uart, USBTX, USBRX);
    serial_baud(&uart, MICROBIT_SERIAL_DEFAULT_BAUD_RATE);
    //Clear uart
    while (serial_readable(&uart)) {
        serial_getc(&uart);
    }
    serial_irq_handler(&uart, rx_interrupt_handler, 1);
    serial_irq_set(&uart, RxIrq, true);
}

int microbit_serial_readable(void) {
    return !is_buf_empty(&microbit_rx_buffer);
}

void microbit_serial_putc(int val) {
    serial_putc(&uart, val);
}

int microbit_serial_getc(void) {
    while (is_buf_empty(&microbit_rx_buffer)) {
        __WFI();
    }
    return read_buf(&microbit_rx_buffer);
}

}
