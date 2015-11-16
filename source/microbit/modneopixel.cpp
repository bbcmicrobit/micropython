
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

#include "py/mpstate.h"
#include "lib/neopixel.h"
#include "microbitobj.h"

STATIC mp_obj_t mod_neopixel_init(mp_obj_t pin_in, mp_obj_t num_leds_in) {
    PinName pin = microbit_obj_get_pin(pin_in)->name;
    MP_STATE_PORT(neopixel_strip) = m_new_obj(neopixel_strip_t);
    neopixel_init(MP_STATE_PORT(neopixel_strip), pin, mp_obj_get_int(num_leds_in));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mod_neopixel_init_obj, mod_neopixel_init);

STATIC mp_obj_t mod_neopixel_clear(void) {
    neopixel_clear(MP_STATE_PORT(neopixel_strip));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(mod_neopixel_clear_obj, mod_neopixel_clear);

STATIC mp_obj_t mod_neopixel_show(void) {
    neopixel_show(MP_STATE_PORT(neopixel_strip));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(mod_neopixel_show_obj, mod_neopixel_show);

STATIC mp_obj_t mod_neopixel_set_colour(mp_uint_t n_args, const mp_obj_t *args) {
    (void)n_args;
    neopixel_set_color(MP_STATE_PORT(neopixel_strip), mp_obj_get_int(args[0]), mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_neopixel_set_colour_obj, 4, 4, mod_neopixel_set_colour);

STATIC const mp_map_elem_t neopixel_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_neopixel) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&mod_neopixel_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_clear), (mp_obj_t)&mod_neopixel_clear_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_show), (mp_obj_t)&mod_neopixel_show_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_colour), (mp_obj_t)&mod_neopixel_set_colour_obj },
};

STATIC MP_DEFINE_CONST_DICT(neopixel_module_globals, neopixel_module_globals_table);

const mp_obj_module_t neopixel_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_neopixel,
    .globals = (mp_obj_dict_t*)&neopixel_module_globals,
};

}
