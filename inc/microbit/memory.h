/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Mark Shannon
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

#ifndef __MICROPY_INCLUDED_MEMORY_H__
#define __MICROPY_INCLUDED_MEMORY_H__

#include "microbit/filesystem.h"

extern uint32_t __data_end__;
extern uint32_t __data_start__;
extern uint32_t __etext;

static inline char *rounddown(char *addr, uint32_t align) {
    return (char *)(((uint32_t)addr)&(-align));
}

static inline char *roundup(char *addr, uint32_t align) {
    return (char *)((((uint32_t)addr)+align-1)&(-align));
}

/** The end of the code area in flash ROM (text plus read-only copy of data area) */
static inline char *microbit_end_of_code() {
    return (char *)(&__etext + (&__data_end__ - &__data_start__));
}

static inline char *microbit_end_of_rom() {
    return (char *)0x40000;
}

static inline char *microbit_mp_appended_script() {
    return (char *)0x3e000;
}

static inline void *microbit_compass_calibration_page(void) {
    if (microbit_mp_appended_script()[0] == 'M') {
        return microbit_mp_appended_script() - persistent_page_size();
    } else {
        return microbit_end_of_rom() - persistent_page_size();
    }
}

#endif // __MICROPY_INCLUDED_MEMORY_H__
