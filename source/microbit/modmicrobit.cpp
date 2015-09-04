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

#include "MicroBit.h"

extern "C" {

#include "py/obj.h"
#include "modmicrobit.h"

STATIC mp_obj_t microbit_sleep(mp_obj_t ms_in) {
    uBit.sleep(mp_obj_get_int(ms_in));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_sleep_obj, microbit_sleep);

STATIC mp_obj_t microbit_random(mp_obj_t max_in) {
    return mp_obj_new_int(uBit.random(mp_obj_get_int(max_in)));
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_random_obj, microbit_random);

STATIC mp_obj_t microbit_system_time(void) {
    return MP_OBJ_NEW_SMALL_INT(uBit.systemTime());
}
MP_DEFINE_CONST_FUN_OBJ_0(microbit_system_time_obj, microbit_system_time);

STATIC mp_obj_t microbit_panic(mp_uint_t n_args, const mp_obj_t *args) {
    if (n_args == 0) {
        uBit.panic();
    } else {
        uBit.panic(mp_obj_get_int(args[0]));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_panic_obj, 0, 1, microbit_panic);

STATIC const mp_map_elem_t microbit_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_microbit) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_io), (mp_obj_t)&microbit_io_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_display), (mp_obj_t)&microbit_display_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_button_a), (mp_obj_t)&microbit_button_a_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_button_b), (mp_obj_t)&microbit_button_b_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_accelerometer), (mp_obj_t)&microbit_accelerometer_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_compass), (mp_obj_t)&microbit_compass_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_sleep), (mp_obj_t)&microbit_sleep_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_random), (mp_obj_t)&microbit_random_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_system_time), (mp_obj_t)&microbit_system_time_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_panic), (mp_obj_t)&microbit_panic_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_module_globals, microbit_module_globals_table);

const mp_obj_module_t microbit_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_microbit,
    .globals = (mp_obj_dict_t*)&microbit_module_globals,
};

}
