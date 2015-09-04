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

typedef struct _microbit_io_obj_t {
    mp_obj_base_t base;
} microbit_io_obj_t;

STATIC const mp_map_elem_t microbit_io_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_P0), (mp_obj_t)&microbit_p0_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P1), (mp_obj_t)&microbit_p1_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P2), (mp_obj_t)&microbit_p2_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P3), (mp_obj_t)&microbit_p3_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P4), (mp_obj_t)&microbit_p4_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P5), (mp_obj_t)&microbit_p5_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P6), (mp_obj_t)&microbit_p6_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P7), (mp_obj_t)&microbit_p7_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P8), (mp_obj_t)&microbit_p8_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P9), (mp_obj_t)&microbit_p9_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P10), (mp_obj_t)&microbit_p10_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P11), (mp_obj_t)&microbit_p11_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P12), (mp_obj_t)&microbit_p12_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P13), (mp_obj_t)&microbit_p13_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P14), (mp_obj_t)&microbit_p14_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P15), (mp_obj_t)&microbit_p15_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P16), (mp_obj_t)&microbit_p16_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P19), (mp_obj_t)&microbit_p19_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_P20), (mp_obj_t)&microbit_p20_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_io_locals_dict, microbit_io_locals_dict_table);

const mp_obj_type_t microbit_io_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitIO,
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
    .bases_tuple = MP_OBJ_NULL,
    /* .locals_dict = */ (mp_obj_t)&microbit_io_locals_dict,
};

const microbit_io_obj_t microbit_io_obj = {
    {&microbit_io_type},
};

}
