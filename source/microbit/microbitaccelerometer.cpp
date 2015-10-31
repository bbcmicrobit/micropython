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

typedef struct _microbit_accelerometer_obj_t {
    mp_obj_base_t base;
    MicroBitAccelerometer *accelerometer;
} microbit_accelerometer_obj_t;

volatile bool accelerometer_up_to_date = false;
volatile bool accelerometer_updating = false;

static void update(microbit_accelerometer_obj_t *self) {
    /* The only time it is possible for accelerometer_updating to be true here
     * is if this is called in an interrupt when it is already updating in
     * the main execution thread. This is extremely unlikely, so we just
     * accept that a slightly out-of-date result will be returned
     */
    if (!accelerometer_up_to_date && !accelerometer_updating) {
        accelerometer_up_to_date = true;
        accelerometer_updating = true;
        self->accelerometer->idleTick();
        accelerometer_updating = false;
    }
}


mp_obj_t microbit_accelerometer_get_x(mp_obj_t self_in) {
    microbit_accelerometer_obj_t *self = (microbit_accelerometer_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(self->accelerometer->getX());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_accelerometer_get_x_obj, microbit_accelerometer_get_x);

mp_obj_t microbit_accelerometer_get_y(mp_obj_t self_in) {
    microbit_accelerometer_obj_t *self = (microbit_accelerometer_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(self->accelerometer->getY());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_accelerometer_get_y_obj, microbit_accelerometer_get_y);

mp_obj_t microbit_accelerometer_get_z(mp_obj_t self_in) {
    microbit_accelerometer_obj_t *self = (microbit_accelerometer_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(self->accelerometer->getZ());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_accelerometer_get_z_obj, microbit_accelerometer_get_z);

mp_obj_t microbit_accelerometer_get_values(mp_obj_t self_in) {
    microbit_accelerometer_obj_t *self = (microbit_accelerometer_obj_t*)self_in;
    mp_obj_tuple_t *tuple = (mp_obj_tuple_t *)mp_obj_new_tuple(3, NULL);
    update(self);
    tuple->items[0] = mp_obj_new_int(self->accelerometer->getX());
    tuple->items[1] = mp_obj_new_int(self->accelerometer->getY());
    tuple->items[2] = mp_obj_new_int(self->accelerometer->getZ());
    return tuple;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_accelerometer_get_values_obj, microbit_accelerometer_get_values);

STATIC const mp_map_elem_t microbit_accelerometer_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_x), (mp_obj_t)&microbit_accelerometer_get_x_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_y), (mp_obj_t)&microbit_accelerometer_get_y_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_z), (mp_obj_t)&microbit_accelerometer_get_z_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_values), (mp_obj_t)&microbit_accelerometer_get_values_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_accelerometer_locals_dict, microbit_accelerometer_locals_dict_table);

const mp_obj_type_t microbit_accelerometer_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitAccelerometer,
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
    /* .locals_dict = */ (mp_obj_t)&microbit_accelerometer_locals_dict,
};

const microbit_accelerometer_obj_t microbit_accelerometer_obj = {
    {&microbit_accelerometer_type},
    .accelerometer = &uBit.accelerometer
};

}
