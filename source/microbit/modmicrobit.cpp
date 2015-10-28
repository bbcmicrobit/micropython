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
#include "mphal.h"
#include "modmicrobit.h"

STATIC mp_obj_t microbit_reset_(void) {
    uBit.reset();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(microbit_reset_obj, microbit_reset_);

STATIC mp_obj_t microbit_sleep(mp_obj_t ms_in) {
    mp_int_t ms = mp_obj_get_int(ms_in);
    mp_hal_sleep(ms);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_sleep_obj, microbit_sleep);

STATIC mp_obj_t microbit_random(mp_obj_t max_in) {
    return mp_obj_new_int(uBit.random(mp_obj_get_int(max_in)));
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_random_obj, microbit_random);

STATIC mp_obj_t microbit_running_time(void) {
    return MP_OBJ_NEW_SMALL_INT(uBit.systemTime());
}
MP_DEFINE_CONST_FUN_OBJ_0(microbit_running_time_obj, microbit_running_time);

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

    { MP_OBJ_NEW_QSTR(MP_QSTR_Image), (mp_obj_t)&microbit_image_type },

    { MP_OBJ_NEW_QSTR(MP_QSTR_display), (mp_obj_t)&microbit_display_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_button_a), (mp_obj_t)&microbit_button_a_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_button_b), (mp_obj_t)&microbit_button_b_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_accelerometer), (mp_obj_t)&microbit_accelerometer_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_compass), (mp_obj_t)&microbit_compass_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_i2c), (mp_obj_t)&microbit_i2c_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_uart), (mp_obj_t)&microbit_uart_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_music), (mp_obj_t)&microbit_music_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_reset), (mp_obj_t)&microbit_reset_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_sleep), (mp_obj_t)&microbit_sleep_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_random), (mp_obj_t)&microbit_random_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_running_time), (mp_obj_t)&microbit_running_time_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_panic), (mp_obj_t)&microbit_panic_obj },
    
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin0), (mp_obj_t)&microbit_p0_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin1), (mp_obj_t)&microbit_p1_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin2), (mp_obj_t)&microbit_p2_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin3), (mp_obj_t)&microbit_p3_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin4), (mp_obj_t)&microbit_p4_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin5), (mp_obj_t)&microbit_p5_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin6), (mp_obj_t)&microbit_p6_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin7), (mp_obj_t)&microbit_p7_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin8), (mp_obj_t)&microbit_p8_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin9), (mp_obj_t)&microbit_p9_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin10), (mp_obj_t)&microbit_p10_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin11), (mp_obj_t)&microbit_p11_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin12), (mp_obj_t)&microbit_p12_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin13), (mp_obj_t)&microbit_p13_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin14), (mp_obj_t)&microbit_p14_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin15), (mp_obj_t)&microbit_p15_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin16), (mp_obj_t)&microbit_p16_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin19), (mp_obj_t)&microbit_p19_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pin20), (mp_obj_t)&microbit_p20_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_module_globals, microbit_module_globals_table);

const mp_obj_module_t microbit_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_microbit,
    .globals = (mp_obj_dict_t*)&microbit_module_globals,
};

}
