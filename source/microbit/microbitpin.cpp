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
#include "microbit/microbitpin.h"
#include "nrf_gpio.h"
#include "py/mphal.h"

static const qstr initial_modes[] = {
    MP_QSTR_unused,  /* pin 0 */
    MP_QSTR_unused,  /* pin 1 */
    MP_QSTR_unused,  /* pin 2 */
    MP_QSTR_display, /* pin 3 */
    MP_QSTR_display, /* pin 4 */
    MP_QSTR_button,  /* pin 5 - button A */
    MP_QSTR_display, /* pin 6 */
    MP_QSTR_display, /* pin 7 */
    MP_QSTR_unused,  /* pin 8 */
    MP_QSTR_display, /* pin 9 */
    MP_QSTR_display, /* pin 10 */
    MP_QSTR_button,  /* pin 11 - button B */
    MP_QSTR_unused,  /* pin 12 */
    MP_QSTR_unused,  /* pin 13 */
    MP_QSTR_unused,  /* pin 14 */
    MP_QSTR_unused,  /* pin 15 */
    MP_QSTR_unused,  /* pin 16 */
    MP_QSTR_3v,      /* pin 17 - 3V */
    MP_QSTR_3v,      /* pin 18 - 3V */
    MP_QSTR_unused,  /* pin 19 */
    MP_QSTR_unused,  /* pin 20 */
    MP_QSTR_stop     /* Sentinel value to mark end of array */
};

uint8_t microbit_pinmodes[24];


static void noop(const microbit_pin_obj_t *pin) {
    (void)pin;
}

static void mode_error(const microbit_pin_obj_t *pin);

static void analog_release(const microbit_pin_obj_t *pin) {
    pwm_release(pin->name);
}

static const microbit_pinmode_t pinmodes[] = {
    {
        MP_QSTR_unused, noop,
    },
    {
        MP_QSTR_write_analog, analog_release,
    },
    {
        MP_QSTR_read_digital, noop,
    },
    {
        MP_QSTR_write_digital, noop,
    },
    {
        MP_QSTR_display, mode_error,
    },
    {
        MP_QSTR_button, mode_error,
    },
    {
        MP_QSTR_music, mode_error,
    },
    {
        MP_QSTR_touch, noop,
    },
    {
        MP_QSTR_audio_play, mode_error,
    },
    {
        MP_QSTR_i2c, mode_error,
    },
    {
        MP_QSTR_spi, mode_error,
    },
    // Add any new pin modes here
    {
        MP_QSTR_3v, mode_error, /* This miust be the end of the array as it acts as a sentinel */
    },
};

#define DEBUG 0

static const microbit_pinmode_t *mode_for_pin(const microbit_pin_obj_t *pin) {
    uint8_t pinmode = microbit_pinmodes[pin->number];
#if DEBUG
    if (pinmode >= sizeof(pinmodes)/sizeof(microbit_pinmode_t)) {
        mp_hal_display_string("Illegal pinmode");
        return &pinmodes[0];
    }
#endif
    return &pinmodes[pinmode];
}

static void mode_error(const microbit_pin_obj_t *pin) {
    const microbit_pinmode_t *current_mode = mode_for_pin(pin);
    nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Pin %d in %q mode", pin->number, current_mode->name));
}

void set_mode(uint32_t pin, qstr name) {
    for (uint32_t index = 0;; index++) {
        if (pinmodes[index].name == name) {
            microbit_pinmodes[pin] = index;
            return;
        }
        if (pinmodes[index].name == MP_QSTR_3v) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_AssertionError, "illegal mode name"));
        }
    }
}

qstr microbit_obj_pin_get_mode(const microbit_pin_obj_t *pin) {
    return mode_for_pin(pin)->name;
}

mp_obj_t microbit_pin_get_mode(mp_obj_t self_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    return MP_OBJ_NEW_QSTR(microbit_obj_pin_get_mode(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_pin_get_mode_obj, microbit_pin_get_mode);

mp_obj_t microbit_pin_write_digital(mp_obj_t self_in, mp_obj_t value_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    int val = mp_obj_get_int(value_in);
    if (val >> 1) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "value must be 0 or 1"));
    }
    if (microbit_obj_pin_get_mode(self) != MP_QSTR_write_digital) {
        microbit_obj_pin_acquire(self, MP_QSTR_write_digital);
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
    if (microbit_obj_pin_get_mode(self) != MP_QSTR_read_digital) {
        microbit_obj_pin_acquire(self, MP_QSTR_read_digital);
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
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid pull"));
    }
    qstr mode = microbit_obj_pin_get_mode(self);
    /* Pull only applies in an read digital mode */
    if (mode != MP_QSTR_read_digital) {
        mode_error(self);
    }
    nrf_gpio_cfg_input(self->name, (nrf_gpio_pin_pull_t)pull);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_pin_set_pull_obj, microbit_pin_set_pull);

#define PULL_MASK (NRF_GPIO_PIN_PULLDOWN | NRF_GPIO_PIN_NOPULL | NRF_GPIO_PIN_PULLUP)

mp_obj_t microbit_pin_get_pull(mp_obj_t self_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    qstr mode = microbit_obj_pin_get_mode(self);
    /* Pull only applies in an read digital mode */
    if (mode != MP_QSTR_read_digital) {
        mode_error(self);
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
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "value must be between 0 and 1023"));
    }
    if (microbit_obj_pin_get_mode(self) != MP_QSTR_write_analog) {
        microbit_obj_pin_acquire(self, MP_QSTR_write_analog);
        nrf_gpio_cfg_output(self->name);
    }
    pwm_set_duty_cycle(self->name, set_value);
    if (set_value == 0)
        microbit_obj_pin_acquire(self, MP_QSTR_unused);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_pin_write_analog_obj, microbit_pin_write_analog);

mp_obj_t microbit_pin_read_analog(mp_obj_t self_in) {
    microbit_pin_obj_t *self = (microbit_pin_obj_t*)self_in;
    microbit_obj_pin_acquire(self, MP_QSTR_unused);
    analogin_t obj;
    analogin_init(&obj, self->name);
    int val = analogin_read_u16(&obj);
    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Disabled;
    return mp_obj_new_int(val);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_pin_read_analog_obj, microbit_pin_read_analog);

mp_obj_t microbit_pin_set_analog_period(mp_obj_t self_in, mp_obj_t period_in) {
    (void)self_in;
    int err = pwm_set_period_us(mp_obj_get_int(period_in)*1000);
    if (err) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid period"));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_pin_set_analog_period_obj, microbit_pin_set_analog_period);

mp_obj_t microbit_pin_set_analog_period_microseconds(mp_obj_t self_in, mp_obj_t period_in) {
    (void)self_in;
    int err = pwm_set_period_us(mp_obj_get_int(period_in));
    if (err) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid period"));
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
    qstr mode = microbit_obj_pin_get_mode(self);
    if (mode != MP_QSTR_touch && mode != MP_QSTR_button) {
        microbit_obj_pin_acquire(self, MP_QSTR_touch);
        nrf_gpio_cfg_input(self->name, NRF_GPIO_PIN_NOPULL);
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

const microbit_pin_obj_t microbit_p0_obj = {{&microbit_touch_pin_type}, 0, MICROBIT_PIN_P0};
const microbit_pin_obj_t microbit_p1_obj = {{&microbit_touch_pin_type}, 1, MICROBIT_PIN_P1};
const microbit_pin_obj_t microbit_p2_obj = {{&microbit_touch_pin_type}, 2, MICROBIT_PIN_P2};
const microbit_pin_obj_t microbit_p3_obj = {{&microbit_ad_pin_type},   3,  MICROBIT_PIN_P3};
const microbit_pin_obj_t microbit_p4_obj = {{&microbit_ad_pin_type},   4,  MICROBIT_PIN_P4};
const microbit_pin_obj_t microbit_p5_obj = {{&microbit_dig_pin_type},  5,  MICROBIT_PIN_P5};
const microbit_pin_obj_t microbit_p6_obj = {{&microbit_dig_pin_type},  6,  MICROBIT_PIN_P6};
const microbit_pin_obj_t microbit_p7_obj = {{&microbit_dig_pin_type},  7,  MICROBIT_PIN_P7};
const microbit_pin_obj_t microbit_p8_obj = {{&microbit_dig_pin_type},  8,  MICROBIT_PIN_P8};
const microbit_pin_obj_t microbit_p9_obj = {{&microbit_dig_pin_type},  9,  MICROBIT_PIN_P9};
const microbit_pin_obj_t microbit_p10_obj = {{&microbit_ad_pin_type},  10, MICROBIT_PIN_P10};
const microbit_pin_obj_t microbit_p11_obj = {{&microbit_dig_pin_type}, 11, MICROBIT_PIN_P11};
const microbit_pin_obj_t microbit_p12_obj = {{&microbit_dig_pin_type}, 12, MICROBIT_PIN_P12};
const microbit_pin_obj_t microbit_p13_obj = {{&microbit_dig_pin_type}, 13, MICROBIT_PIN_P13};
const microbit_pin_obj_t microbit_p14_obj = {{&microbit_dig_pin_type}, 14, MICROBIT_PIN_P14};
const microbit_pin_obj_t microbit_p15_obj = {{&microbit_dig_pin_type}, 15, MICROBIT_PIN_P15};
const microbit_pin_obj_t microbit_p16_obj = {{&microbit_dig_pin_type}, 16, MICROBIT_PIN_P16};
const microbit_pin_obj_t microbit_p19_obj = {{&microbit_dig_pin_type}, 19, MICROBIT_PIN_P19};
const microbit_pin_obj_t microbit_p20_obj = {{&microbit_dig_pin_type}, 20, MICROBIT_PIN_P20};


void microbit_pin_init(void) {
    nrf_gpio_cfg_input(microbit_p5_obj.name, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(microbit_p11_obj.name, NRF_GPIO_PIN_PULLUP);
    for (uint32_t pin = 0;; pin++) {
        if (initial_modes[pin] == MP_QSTR_stop)
            return;
        set_mode(pin, initial_modes[pin]);
    }
}

void microbit_obj_pin_fail_if_cant_acquire(const microbit_pin_obj_t *pin) {
    const microbit_pinmode_t *current_mode = mode_for_pin(pin);
    if (current_mode->release == mode_error) {
        mode_error(pin);
    }
}

void microbit_obj_pin_free(const microbit_pin_obj_t *pin) {
    set_mode(pin->number, MP_QSTR_unused);
}

bool microbit_obj_pin_can_be_acquired(const microbit_pin_obj_t *pin) {
    const microbit_pinmode_t *current_mode = mode_for_pin(pin);
    return current_mode->release != NULL;
}

void microbit_obj_pin_acquire(const microbit_pin_obj_t *pin, qstr new_mode) {
    const microbit_pinmode_t *current_mode = mode_for_pin(pin);
    current_mode->release(pin);
    set_mode(pin->number, new_mode);
}

const microbit_pin_obj_t *microbit_obj_get_pin(mp_obj_t o) {
    mp_obj_type_t *type = mp_obj_get_type(o);
    if (type == &microbit_touch_pin_type || type == &microbit_ad_pin_type || type == &microbit_dig_pin_type) {
        return (microbit_pin_obj_t*)o;
    } else {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "expecting a pin"));
    }
}

PinName microbit_obj_get_pin_name(mp_obj_t o) {
    return microbit_obj_get_pin(o)->name;
}

}
