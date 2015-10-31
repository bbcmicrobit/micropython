
/*
 * This file is part of the Micro Python project, http://micropython.org/
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

#include <stdio.h>
#include "MicroBit.h"

extern "C" {

#include "mphal.h"
#include "microbitimage.h"
#include "microbitdisplay.h"

#define GET_PIXEL(x, y) microbit_display_get_pixel(&microbit_display_obj, x, y)
#define SET_PIXEL(x, y, v) microbit_display_set_pixel(&microbit_display_obj, x, y, v)

void antigravity(uint8_t interval = 200 /* ms */) {
    // move all of the LED's upwards (we can move them in other directions in the future)

    for (uint8_t iteration = 0; iteration < 5; iteration++) {
        mp_hal_delay_ms(interval);
        bool wait = false;
        for (uint8_t row = 1; row < 5 - iteration; row++) {
            for (uint8_t col = 0; col < 5; col++) {
                // move this bit down if possible
                uint8_t val = GET_PIXEL(col, row);
                if (val) {
                    // this is why the row for loop starts at one
                    if (!GET_PIXEL(col, row - 1)) {
                        SET_PIXEL(col, row, 0);
                        SET_PIXEL(col, row - 1, val);
                        wait = true;
                    }
                } // we don't care if the LED is off
            }
        }

        if (!wait) {
            continue;
        }
    }
}

STATIC mp_obj_t antigravity__init__(void) {
    antigravity();

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(antigravity___init___obj, antigravity__init__);

STATIC const mp_map_elem_t antigravity_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_antigravity) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&antigravity___init___obj },
};

STATIC MP_DEFINE_CONST_DICT(antigravity_module_globals, antigravity_module_globals_table);

const mp_obj_module_t antigravity_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_antigravity,
    .globals = (mp_obj_dict_t*)&antigravity_module_globals,
};

}
