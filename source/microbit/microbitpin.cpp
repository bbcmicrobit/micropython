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

#include "MicroBitPin.h"

extern "C" {

#include "nrf_gpio.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "lib/pwm.h"
#include "microbit/modmicrobit.h"

const microbit_pin_obj_t microbit_p0_obj = {{&microbit_touch_pin_type}, 0, MICROBIT_PIN_P0, MODE_UNUSED};
const microbit_pin_obj_t microbit_p1_obj = {{&microbit_touch_pin_type}, 1, MICROBIT_PIN_P1, MODE_UNUSED};
const microbit_pin_obj_t microbit_p2_obj = {{&microbit_touch_pin_type}, 2, MICROBIT_PIN_P2, MODE_UNUSED};
const microbit_pin_obj_t microbit_p3_obj = {{&microbit_ad_pin_type},   3,  MICROBIT_PIN_P3, MODE_DISPLAY};
const microbit_pin_obj_t microbit_p4_obj = {{&microbit_ad_pin_type},   4,  MICROBIT_PIN_P4, MODE_DISPLAY};
const microbit_pin_obj_t microbit_p5_obj = {{&microbit_dig_pin_type},  5,  MICROBIT_PIN_P5, MODE_BUTTON};
const microbit_pin_obj_t microbit_p6_obj = {{&microbit_dig_pin_type},  6,  MICROBIT_PIN_P6, MODE_DISPLAY};
const microbit_pin_obj_t microbit_p7_obj = {{&microbit_dig_pin_type},  7,  MICROBIT_PIN_P7, MODE_DISPLAY};
const microbit_pin_obj_t microbit_p8_obj = {{&microbit_dig_pin_type},  8,  MICROBIT_PIN_P8, MODE_UNUSED};
const microbit_pin_obj_t microbit_p9_obj = {{&microbit_dig_pin_type},  9,  MICROBIT_PIN_P9, MODE_DISPLAY};
const microbit_pin_obj_t microbit_p10_obj = {{&microbit_ad_pin_type},  10, MICROBIT_PIN_P10, MODE_DISPLAY};
const microbit_pin_obj_t microbit_p11_obj = {{&microbit_dig_pin_type}, 11, MICROBIT_PIN_P11, MODE_BUTTON};
const microbit_pin_obj_t microbit_p12_obj = {{&microbit_dig_pin_type}, 12, MICROBIT_PIN_P12, MODE_UNUSED};
const microbit_pin_obj_t microbit_p13_obj = {{&microbit_dig_pin_type}, 13, MICROBIT_PIN_P13, MODE_UNUSED};
const microbit_pin_obj_t microbit_p14_obj = {{&microbit_dig_pin_type}, 14, MICROBIT_PIN_P14, MODE_UNUSED};
const microbit_pin_obj_t microbit_p15_obj = {{&microbit_dig_pin_type}, 15, MICROBIT_PIN_P15, MODE_UNUSED};
const microbit_pin_obj_t microbit_p16_obj = {{&microbit_dig_pin_type}, 16, MICROBIT_PIN_P16, MODE_UNUSED};
const microbit_pin_obj_t microbit_p19_obj = {{&microbit_dig_pin_type}, 19, MICROBIT_PIN_P19, MODE_I2C};
const microbit_pin_obj_t microbit_p20_obj = {{&microbit_dig_pin_type}, 20, MICROBIT_PIN_P20, MODE_I2C};


static mp_obj_t microbit_pin_get_mode_func(mp_obj_t self_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    return MP_OBJ_NEW_QSTR(microbit_pin_get_mode(self)->name);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_pin_get_mode_obj, microbit_pin_get_mode_func);

mp_obj_t microbit_pin_write_digital(mp_obj_t self_in, mp_obj_t value_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    int val = mp_obj_get_int(value_in);
    if (val >> 1) {
        mp_raise_ValueError("value must be 0 or 1");
    }
    if (microbit_obj_pin_acquire(self, microbit_pin_mode_write_digital)) {
        nrf_gpio_cfg_output(self->name);
    }
    if (val)
        nrf_gpio_pin_set(self->name);
    else
        nrf_gpio_pin_clear(self->name);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_pin_write_digital_obj, microbit_pin_write_digital);

mp_obj_t microbit_pin_read_digital(mp_obj_t self_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    if (microbit_obj_pin_acquire(self, microbit_pin_mode_read_digital)) {
        nrf_gpio_cfg_input(self->name, NRF_GPIO_PIN_PULLDOWN);
    }
    return mp_obj_new_int(nrf_gpio_pin_read(self->name));
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_pin_read_digital_obj, microbit_pin_read_digital);

#define SHIFT_PULL_MASK ((1<<NRF_GPIO_PIN_PULLDOWN) | (1<<NRF_GPIO_PIN_PULLUP) | (1<<NRF_GPIO_PIN_NOPULL))


mp_obj_t microbit_pin_set_pull(mp_obj_t self_in, mp_obj_t pull_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    int pull = mp_obj_get_int(pull_in);
    if (((1 << pull) & SHIFT_PULL_MASK) == 0) {
        mp_raise_ValueError("invalid pull");
    }
    /* Pull only applies in an read digital mode */
    microbit_obj_pin_acquire(self, microbit_pin_mode_read_digital);
    nrf_gpio_cfg_input(self->name, (nrf_gpio_pin_pull_t)pull);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_pin_set_pull_obj, microbit_pin_set_pull);

#define PULL_MASK (NRF_GPIO_PIN_PULLDOWN | NRF_GPIO_PIN_NOPULL | NRF_GPIO_PIN_PULLUP)

mp_obj_t microbit_pin_get_pull(mp_obj_t self_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    const microbit_pinmode_t *mode = microbit_pin_get_mode(self);
    /* Pull only applies in an read digital mode */
    if (mode != microbit_pin_mode_read_digital) {
        pinmode_error(self);
    }
    uint32_t pull = (NRF_GPIO->PIN_CNF[self->name] >> GPIO_PIN_CNF_PULL_Pos) & PULL_MASK;
    return mp_obj_new_int(pull);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_pin_get_pull_obj, microbit_pin_get_pull);

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
        mp_raise_ValueError("value must be between 0 and 1023");
    }
    if (microbit_obj_pin_acquire(self, microbit_pin_mode_write_analog)) {
        nrf_gpio_cfg_output(self->name);
    }
    pwm_set_duty_cycle(self->name, set_value);
    if (set_value == 0)
        microbit_obj_pin_acquire(self, microbit_pin_mode_unused);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_pin_write_analog_obj, microbit_pin_write_analog);

mp_obj_t microbit_pin_read_analog(mp_obj_t self_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    microbit_obj_pin_acquire(self, microbit_pin_mode_unused);
    analogin_t obj;
    analogin_init(&obj, (PinName)self->name);
    int val = analogin_read_u16(&obj);
    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Disabled;
    return mp_obj_new_int(val);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_pin_read_analog_obj, microbit_pin_read_analog);

mp_obj_t microbit_pin_set_analog_period(mp_obj_t self_in, mp_obj_t period_in) {
    (void)self_in;
    int err = pwm_set_period_us(mp_obj_get_int(period_in)*1000);
    if (err) {
        mp_raise_ValueError("invalid period");
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_pin_set_analog_period_obj, microbit_pin_set_analog_period);

mp_obj_t microbit_pin_set_analog_period_microseconds(mp_obj_t self_in, mp_obj_t period_in) {
    (void)self_in;
    int err = pwm_set_period_us(mp_obj_get_int(period_in));
    if (err) {
        mp_raise_ValueError("invalid period");
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_pin_set_analog_period_microseconds_obj, microbit_pin_set_analog_period_microseconds);

mp_obj_t microbit_pin_get_analog_period_microseconds(mp_obj_t self_in) {
    (void)self_in;
    int32_t period = pwm_get_period_us();
    return mp_obj_new_int(period);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_pin_get_analog_period_microseconds_obj, microbit_pin_get_analog_period_microseconds);

mp_obj_t microbit_pin_is_touched(mp_obj_t self_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    const microbit_pinmode_t *mode = microbit_pin_get_mode(self);
    if (mode != microbit_pin_mode_touch && mode != microbit_pin_mode_button) {
        microbit_obj_pin_acquire(self, microbit_pin_mode_touch);
        nrf_gpio_cfg_input(self->name, NRF_GPIO_PIN_PULLUP);
    }
    /* Pin is touched if it is low after debouncing */
    return mp_obj_new_bool(!microbit_pin_high_debounced(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_pin_is_touched_obj, microbit_pin_is_touched);

#define PULL_CONSTANTS \
    { MP_OBJ_NEW_QSTR(MP_QSTR_PULL_UP), MP_OBJ_NEW_SMALL_INT(NRF_GPIO_PIN_PULLUP) }, \
    { MP_OBJ_NEW_QSTR(MP_QSTR_PULL_DOWN), MP_OBJ_NEW_SMALL_INT(NRF_GPIO_PIN_PULLDOWN) }, \
    { MP_OBJ_NEW_QSTR(MP_QSTR_NO_PULL), MP_OBJ_NEW_SMALL_INT(NRF_GPIO_PIN_NOPULL) }

STATIC const mp_map_elem_t microbit_dig_pin_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_digital), (mp_obj_t)&microbit_pin_write_digital_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_digital), (mp_obj_t)&microbit_pin_read_digital_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_analog), (mp_obj_t)&microbit_pin_write_analog_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_analog_period), (mp_obj_t)&microbit_pin_set_analog_period_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_analog_period_microseconds), (mp_obj_t)&microbit_pin_set_analog_period_microseconds_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_analog_period_microseconds), (mp_obj_t)&microbit_pin_get_analog_period_microseconds_obj },
    PULL_CONSTANTS,
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_pull),(mp_obj_t)&microbit_pin_get_pull_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_pull),(mp_obj_t)&microbit_pin_set_pull_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_mode), (mp_obj_t)&microbit_pin_get_mode_obj },
};

STATIC const mp_map_elem_t microbit_ann_pin_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_digital), (mp_obj_t)&microbit_pin_write_digital_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_digital), (mp_obj_t)&microbit_pin_read_digital_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_analog), (mp_obj_t)&microbit_pin_write_analog_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_analog), (mp_obj_t)&microbit_pin_read_analog_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_analog_period), (mp_obj_t)&microbit_pin_set_analog_period_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_analog_period_microseconds), (mp_obj_t)&microbit_pin_set_analog_period_microseconds_obj },
    PULL_CONSTANTS,
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_pull),(mp_obj_t)&microbit_pin_get_pull_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_pull),(mp_obj_t)&microbit_pin_set_pull_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_mode), (mp_obj_t)&microbit_pin_get_mode_obj },
};

STATIC const mp_map_elem_t microbit_touch_pin_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_digital), (mp_obj_t)&microbit_pin_write_digital_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_digital), (mp_obj_t)&microbit_pin_read_digital_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_analog), (mp_obj_t)&microbit_pin_write_analog_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_analog), (mp_obj_t)&microbit_pin_read_analog_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_analog_period), (mp_obj_t)&microbit_pin_set_analog_period_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_analog_period_microseconds), (mp_obj_t)&microbit_pin_set_analog_period_microseconds_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_touched), (mp_obj_t)&microbit_pin_is_touched_obj },
    PULL_CONSTANTS,
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_pull),(mp_obj_t)&microbit_pin_get_pull_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_pull),(mp_obj_t)&microbit_pin_set_pull_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_mode), (mp_obj_t)&microbit_pin_get_mode_obj },
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
    .protocol = NULL,
    .parent = NULL,
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
    .protocol = NULL,
    .parent = NULL,
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
    .protocol = NULL,
    .parent = NULL,
    .locals_dict = (mp_obj_dict_t*)&microbit_touch_pin_locals_dict,
};


void microbit_pin_init(void) {
    nrf_gpio_cfg_input(microbit_p5_obj.name, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(microbit_p11_obj.name, NRF_GPIO_PIN_PULLUP);
}


const microbit_pin_obj_t *microbit_obj_get_pin(mp_obj_t o) {
    mp_obj_type_t *type = mp_obj_get_type(o);
    if (type == &microbit_touch_pin_type || type == &microbit_ad_pin_type || type == &microbit_dig_pin_type) {
        return (microbit_pin_obj_t*)o;
    } else {
        mp_raise_TypeError("expecting a pin");
    }
}

uint8_t microbit_obj_get_pin_name(mp_obj_t o) {
    return microbit_obj_get_pin(o)->name;
}

}
