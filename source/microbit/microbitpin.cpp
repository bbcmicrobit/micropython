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
#include "microbitobj.h"

extern "C" {

#include "py/runtime.h"
#include "modmicrobit.h"
#include "lib/pwm.h"

typedef struct _microbit_pin_obj_t {
    mp_obj_base_t base;
    MicroBitPin *pin;
} microbit_pin_obj_t;

mp_obj_t microbit_pin_write_digital(mp_obj_t self_in, mp_obj_t value_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    int val = mp_obj_get_int(value_in);
    if (val >> 1) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "value must be 0 or 1"));
    }
    self->pin->setDigitalValue(val);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_pin_write_digital_obj, microbit_pin_write_digital);

mp_obj_t microbit_pin_read_digital(mp_obj_t self_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    int val = self->pin->getDigitalValue();
    assert(val != MICROBIT_IO_OP_NA);
    return mp_obj_new_int(val);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_pin_read_digital_obj, microbit_pin_read_digital);

mp_obj_t microbit_pin_write_analog(mp_obj_t self_in, mp_obj_t value_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    int set_value;
    if (mp_obj_is_float(value_in)) {
        mp_float_t val = mp_obj_get_float(value_in);
        set_value = val+0.5;
    } else {
        set_value = mp_obj_get_int(value_in);
    }
    if (set_value < 0 || set_value > MICROBIT_PIN_MAX_OUTPUT) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "value must be between 0 and 1023"));
    }
    pwm_set_duty_cycle(self->pin->name, set_value);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_pin_write_analog_obj, microbit_pin_write_analog);

mp_obj_t microbit_pin_read_analog(mp_obj_t self_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    int val = self->pin->getAnalogValue();
    assert(val != MICROBIT_IO_OP_NA);
    return mp_obj_new_int(val);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_pin_read_analog_obj, microbit_pin_read_analog);

mp_obj_t microbit_pin_set_analog_period(mp_obj_t self_in, mp_obj_t period_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    int err = pwm_set_period_us(mp_obj_get_int(period_in)*1000);
    if (err) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid period"));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_pin_set_analog_period_obj, microbit_pin_set_analog_period);

mp_obj_t microbit_pin_set_analog_period_microseconds(mp_obj_t self_in, mp_obj_t period_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    int err = pwm_set_period_us(mp_obj_get_int(period_in));
    if (err) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid period"));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_pin_set_analog_period_microseconds_obj, microbit_pin_set_analog_period_microseconds);

mp_obj_t microbit_pin_get_analog_period_microseconds(mp_obj_t self_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    int32_t period = pwm_get_period_us();
    return mp_obj_new_int(period);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_pin_get_analog_period_microseconds_obj, microbit_pin_get_analog_period_microseconds);

mp_obj_t microbit_pin_is_touched(mp_obj_t self_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    return mp_obj_new_bool(self->pin->isTouched());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_pin_is_touched_obj, microbit_pin_is_touched);

STATIC const mp_map_elem_t microbit_dig_pin_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_digital), (mp_obj_t)&microbit_pin_write_digital_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_digital), (mp_obj_t)&microbit_pin_read_digital_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_analog), (mp_obj_t)&microbit_pin_write_analog_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_analog_period), (mp_obj_t)&microbit_pin_set_analog_period_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_analog_period_microseconds), (mp_obj_t)&microbit_pin_set_analog_period_microseconds_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_analog_period_microseconds), (mp_obj_t)&microbit_pin_get_analog_period_microseconds_obj },
};

STATIC const mp_map_elem_t microbit_ann_pin_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_digital), (mp_obj_t)&microbit_pin_write_digital_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_digital), (mp_obj_t)&microbit_pin_read_digital_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_analog), (mp_obj_t)&microbit_pin_write_analog_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_analog), (mp_obj_t)&microbit_pin_read_analog_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_analog_period), (mp_obj_t)&microbit_pin_set_analog_period_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_analog_period_microseconds), (mp_obj_t)&microbit_pin_set_analog_period_microseconds_obj },
};

STATIC const mp_map_elem_t microbit_touch_pin_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_digital), (mp_obj_t)&microbit_pin_write_digital_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_digital), (mp_obj_t)&microbit_pin_read_digital_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_analog), (mp_obj_t)&microbit_pin_write_analog_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_analog), (mp_obj_t)&microbit_pin_read_analog_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_analog_period), (mp_obj_t)&microbit_pin_set_analog_period_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_analog_period_microseconds), (mp_obj_t)&microbit_pin_set_analog_period_microseconds_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_touched), (mp_obj_t)&microbit_pin_is_touched_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_dig_pin_locals_dict, microbit_dig_pin_locals_dict_table);
STATIC MP_DEFINE_CONST_DICT(microbit_ann_pin_locals_dict, microbit_ann_pin_locals_dict_table);
STATIC MP_DEFINE_CONST_DICT(microbit_touch_pin_locals_dict, microbit_touch_pin_locals_dict_table);


const mp_obj_type_t microbit_dig_pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitDigitalPin,
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
    .locals_dict = (mp_obj_dict_t*)&microbit_dig_pin_locals_dict,
};

const mp_obj_type_t microbit_ad_pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitAnalogDigitalPin,
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
    .locals_dict = (mp_obj_dict_t*)&microbit_ann_pin_locals_dict,
};

const mp_obj_type_t microbit_touch_pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitTouchPin,
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
    .locals_dict = (mp_obj_dict_t*)&microbit_touch_pin_locals_dict,
};

const microbit_pin_obj_t microbit_p0_obj = {{&microbit_touch_pin_type}, &uBit.io.P0};
const microbit_pin_obj_t microbit_p1_obj = {{&microbit_touch_pin_type}, &uBit.io.P1};
const microbit_pin_obj_t microbit_p2_obj = {{&microbit_touch_pin_type}, &uBit.io.P2};
const microbit_pin_obj_t microbit_p3_obj = {{&microbit_ad_pin_type}, &uBit.io.P3};
const microbit_pin_obj_t microbit_p4_obj = {{&microbit_ad_pin_type}, &uBit.io.P4};
const microbit_pin_obj_t microbit_p5_obj = {{&microbit_dig_pin_type}, &uBit.io.P5};
const microbit_pin_obj_t microbit_p6_obj = {{&microbit_dig_pin_type}, &uBit.io.P6};
const microbit_pin_obj_t microbit_p7_obj = {{&microbit_dig_pin_type}, &uBit.io.P7};
const microbit_pin_obj_t microbit_p8_obj = {{&microbit_dig_pin_type}, &uBit.io.P8};
const microbit_pin_obj_t microbit_p9_obj = {{&microbit_dig_pin_type}, &uBit.io.P9};
const microbit_pin_obj_t microbit_p10_obj = {{&microbit_ad_pin_type}, &uBit.io.P10};
const microbit_pin_obj_t microbit_p11_obj = {{&microbit_dig_pin_type}, &uBit.io.P11};
const microbit_pin_obj_t microbit_p12_obj = {{&microbit_dig_pin_type}, &uBit.io.P12};
const microbit_pin_obj_t microbit_p13_obj = {{&microbit_dig_pin_type}, &uBit.io.P13};
const microbit_pin_obj_t microbit_p14_obj = {{&microbit_dig_pin_type}, &uBit.io.P14};
const microbit_pin_obj_t microbit_p15_obj = {{&microbit_dig_pin_type}, &uBit.io.P15};
const microbit_pin_obj_t microbit_p16_obj = {{&microbit_dig_pin_type}, &uBit.io.P16};
const microbit_pin_obj_t microbit_p19_obj = {{&microbit_dig_pin_type}, &uBit.io.P19};
const microbit_pin_obj_t microbit_p20_obj = {{&microbit_dig_pin_type}, &uBit.io.P20};

MicroBitPin *microbit_obj_get_pin(mp_obj_t o) {
    mp_obj_type_t *type = mp_obj_get_type(o);
    if (type == &microbit_touch_pin_type || type == &microbit_ad_pin_type || type == &microbit_dig_pin_type) {
        microbit_pin_obj_t *pin = (microbit_pin_obj_t*)o;
        return pin->pin;
    } else {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "expecting a pin"));
    }
}

PinName microbit_obj_get_pin_name(mp_obj_t o) {
    return microbit_obj_get_pin(o)->name;
}

}
