/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George
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
#ifndef __MICROPY_INCLUDED_MICROBIT_MPHALPORT_H__
#define __MICROPY_INCLUDED_MICROBIT_MPHALPORT_H__

#ifdef __cplusplus
extern "C" {
#endif

void mp_hal_init(void);

void mp_hal_set_interrupt_char(int c);
int mp_hal_stdin_rx_any(void);

// provide these since we don't assume VT100 support
void mp_hal_move_cursor_back(unsigned int pos);
void mp_hal_erase_line_from_cursor(unsigned int n_chars);

void mp_hal_display_string(const char*);

// MicroPython low-level C API for pins
#include "nrf_gpio.h"
#include "microbit/modmicrobit.h"
#define mp_hal_pin_obj_t uint8_t
#define mp_hal_get_pin_obj(o) microbit_obj_get_pin_name(o)
#define mp_hal_pin_read(p) (int)nrf_gpio_pin_read(p)

#ifdef __cplusplus
}
#endif

#endif // __MICROPY_INCLUDED_MICROBIT_MPHALPORT_H__
