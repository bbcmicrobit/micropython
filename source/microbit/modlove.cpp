
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

#include <stdio.h>

extern "C" {

#include "py/mphal.h"
#include "microbit/modmicrobit.h"
#include "microbit/microbit_image.h"

static const mp_float_t bright[7] = {
    0.0, 1.0/9, 2.0/9, 4.0/9, 6.0/9, 7.0/9, 1.0,
};

void love(int interval = 25 /* ms */) {
    microbit_image_obj_t *hearts[MP_ARRAY_SIZE(bright)];
    for (uint i = 0; i < MP_ARRAY_SIZE(bright); i++) {
         hearts[i] = microbit_image_dim(HEART_IMAGE, bright[i]);
    }

    for (int iteration = 0; iteration < 8; iteration++) {
        // pause between double beats of the heart
        if (iteration && (iteration & 1) == 0) {
            mp_hal_delay_ms(20 * interval);
        }
        // pulse heart to max brightness
        for(uint step = 0; step < MP_ARRAY_SIZE(bright); ++step) {
            microbit_display_show(&microbit_display_obj, hearts[step]);
            mp_hal_delay_ms(interval);
        }
        // pulse heart to min brightness
        for(int step = MP_ARRAY_SIZE(bright) - 1; step >= 0; --step) {
            microbit_display_show(&microbit_display_obj, hearts[step]);
            mp_hal_delay_ms(interval);
        }
    }
    microbit_display_clear();
}

STATIC mp_obj_t love_badaboom(void) {
    // make
    love();
    // ! war
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(love___init___obj, love_badaboom);
MP_DEFINE_CONST_FUN_OBJ_0(love_badaboom_obj, love_badaboom);

STATIC const mp_map_elem_t love_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_love) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&love___init___obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_badaboom), (mp_obj_t)&love_badaboom_obj },
};

STATIC MP_DEFINE_CONST_DICT(love_module_globals, love_module_globals_table);

const mp_obj_module_t love_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&love_module_globals,
};

}
