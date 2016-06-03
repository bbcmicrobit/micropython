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

#include "py/runtime.h"
#include "modmicrobit.h"

typedef struct _microbit_i2c_obj_t {
    mp_obj_base_t base;
    MicroBitI2C *i2c;
    int frequency; /* This could be a short using KHz if 2 bytes are needed */
} microbit_i2c_obj_t;

STATIC mp_obj_t microbit_i2c_set_frequency(mp_obj_t self_in, mp_obj_t frequency_in) {
    microbit_i2c_obj_t *self = (microbit_i2c_obj_t*)self_in;
    int frequency = mp_obj_get_int(frequency_in);

    /* Workaround: This is built with knowledge from
     * mbed-classic/targets/hal/TARGET_NORDIC/TARGET_MCU_NRF51822/i2c_api.c
     * which clamps the frequency to one of the following values.  The mbed
     * API doesn't support return values and DAL doesn't support getting the
     * frequency (it's protected with no getter).  We prefer to raise a
     * ValueError when the frequency isn't one of the supported frequencies
     * instead of clamping it silently, and to support getting the frequency.
     *
     * Note: If the default frequency changes, please update the initialized
     * value at the bottom of this file.
     */
    switch (frequency) {
    case 100000: /* 100 KHz */
    case 250000: /* 250 KHz */
    case 400000: /* 400 KHz */
        break;
    default:
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Invalid frequency"));
    }

    self->frequency = frequency;
    self->i2c->frequency(frequency);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_i2c_set_frequency_obj, microbit_i2c_set_frequency);

STATIC mp_obj_t microbit_i2c_get_frequency(mp_obj_t self_in) {
    microbit_i2c_obj_t *self = (microbit_i2c_obj_t*)self_in;

    return mp_obj_new_int(self->frequency);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_i2c_get_frequency_obj, microbit_i2c_get_frequency);

STATIC mp_obj_t microbit_i2c_read(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_addr,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_n,        MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_repeat,   MP_ARG_BOOL, {.u_bool = false} },
    };

    // parse args
    microbit_i2c_obj_t *self = (microbit_i2c_obj_t*)pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // do the I2C read
    vstr_t vstr;
    vstr_init_len(&vstr, args[1].u_int);
    int err = self->i2c->read(args[0].u_int << 1, vstr.buf, vstr.len, args[2].u_bool);
    if (err != MICROBIT_OK) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_OSError, "I2C read failed with error code %d", err));
    }
    // return bytes object with read data
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
MP_DEFINE_CONST_FUN_OBJ_KW(microbit_i2c_read_obj, 1, microbit_i2c_read);

STATIC mp_obj_t microbit_i2c_write(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_addr,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_buf,      MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_repeat,   MP_ARG_BOOL, {.u_bool = false} },
    };

    // parse args
    microbit_i2c_obj_t *self = (microbit_i2c_obj_t*)pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // do the I2C write
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[1].u_obj, &bufinfo, MP_BUFFER_READ);
    int err = self->i2c->write(args[0].u_int << 1, (char*)bufinfo.buf, bufinfo.len, args[2].u_bool);
    if (err != MICROBIT_OK) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_OSError, "I2C write failed with error code %d", err));
    }
    return mp_const_none;

}
MP_DEFINE_CONST_FUN_OBJ_KW(microbit_i2c_write_obj, 1, microbit_i2c_write);

STATIC const mp_map_elem_t microbit_i2c_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_frequency), (mp_obj_t)&microbit_i2c_set_frequency_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_frequency), (mp_obj_t)&microbit_i2c_get_frequency_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&microbit_i2c_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&microbit_i2c_write_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_i2c_locals_dict, microbit_i2c_locals_dict_table);

const mp_obj_type_t microbit_i2c_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitI2C,
    .print = NULL,
    .make_new = NULL,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = NULL,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .stream_p = NULL,
    .bases_tuple = NULL,
    .locals_dict = (mp_obj_dict_t*)&microbit_i2c_locals_dict,
};

microbit_i2c_obj_t microbit_i2c_obj = {
    {&microbit_i2c_type},
    .i2c = &uBit.i2c,
    .frequency = 100000, /* 100 KHz */
};

}
