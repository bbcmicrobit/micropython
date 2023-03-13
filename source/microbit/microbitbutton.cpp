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

extern "C" {

#include "nrf_gpio.h"
#include "py/runtime.h"
#include "microbit/modmicrobit.h"

// Maps pin to an index in the debounce_state array.
// Mapping from pin number to index: 0 1 2 5 11 -> 0 1 2 3 4
#define DEBOUNCE_STATE_INDEX(pin) ((0x00304210 >> (((pin)->number & 7) * 4)) & 0xf)

typedef struct _microbit_button_obj_t {
    mp_obj_base_t base;
    const microbit_pin_obj_t *pin;
} microbit_button_obj_t;

typedef struct _debounce_state_t {
    uint16_t pressed; // Stores pressed count in top 15 bits and was_pressed in the low bit
    int8_t sigmas;
    bool debounced_high;
} debounce_state_t;

static debounce_state_t debounce_state[5] = {
    { 0, 5, true },
    { 0, 5, true },
    { 0, 5, true },
    { 0, 5, true },
    { 0, 5, true },
};

static inline debounce_state_t *get_debounce_state(const microbit_pin_obj_t *pin) {
    return &debounce_state[DEBOUNCE_STATE_INDEX(pin)];
}

mp_obj_t microbit_button_is_pressed(mp_obj_t self_in) {
    microbit_button_obj_t *self = (microbit_button_obj_t*)self_in;
    /* Button is pressed if pin is low */
    return mp_obj_new_bool(!microbit_pin_debounce_is_high(self->pin));
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_button_is_pressed_obj, microbit_button_is_pressed);

mp_obj_t microbit_button_get_presses(mp_obj_t self_in) {
    microbit_button_obj_t *self = (microbit_button_obj_t*)self_in;
    return mp_obj_new_int(microbit_pin_debounce_get_presses(self->pin));
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_button_get_presses_obj, microbit_button_get_presses);

mp_obj_t microbit_button_was_pressed(mp_obj_t self_in) {
    microbit_button_obj_t *self = (microbit_button_obj_t*)self_in;
    return mp_obj_new_bool(microbit_pin_debounce_was_pressed(self->pin));
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_button_was_pressed_obj, microbit_button_was_pressed);

STATIC const mp_map_elem_t microbit_button_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_pressed), (mp_obj_t)&microbit_button_is_pressed_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_was_pressed), (mp_obj_t)&microbit_button_was_pressed_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_presses), (mp_obj_t)&microbit_button_get_presses_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_button_locals_dict, microbit_button_locals_dict_table);

STATIC const mp_obj_type_t microbit_button_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitButton,
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
    .locals_dict = (mp_obj_dict_t*)&microbit_button_locals_dict,
};

const microbit_button_obj_t microbit_button_a_obj = {
    {&microbit_button_type},
    .pin = &microbit_p5_obj,
};

const microbit_button_obj_t microbit_button_b_obj = {
    {&microbit_button_type},
    .pin = &microbit_p11_obj,
};

enum PinTransition
{
    LOW_LOW = 0,
    LOW_HIGH = 1,
    HIGH_LOW = 2,
    HIGH_HIGH = 3
};

static PinTransition update(const microbit_pin_obj_t *pin) {
    debounce_state_t *debounce = get_debounce_state(pin);

    int32_t sigma = debounce->sigmas;
    PinTransition result;
    if (nrf_gpio_pin_read(pin->name))
        sigma++;
    else
        sigma--;
    if (sigma < 3) {
        if (sigma < 0) {
            sigma = 0;
            result = LOW_LOW;
        } else if (debounce->debounced_high) {
            result = HIGH_LOW;
            debounce->debounced_high = false;
        } else {
            result = LOW_LOW;
        }
    } else if (sigma > 7) {
        if (sigma > 12) {
            sigma = 12;
            result = HIGH_HIGH;
        } else if (debounce->debounced_high) {
            result = HIGH_HIGH;
        } else {
            result = LOW_HIGH;
            debounce->debounced_high = true;
        }
    } else if (debounce->debounced_high) {
        result = HIGH_HIGH;
    } else {
        result = LOW_LOW;
    }
    debounce->sigmas = sigma;

    if (result == HIGH_LOW) {
        debounce->pressed = (debounce->pressed + 2) | 1;
    }

    return result;
}

void microbit_button_tick(void) {
    // Update both buttons and the touch pins.
    // Button is pressed when its pin transfers from HIGH to LOW.
    update(microbit_button_a_obj.pin);
    update(microbit_button_b_obj.pin);
    if (microbit_pin_get_mode(&microbit_p0_obj) == microbit_pin_mode_touch)
        update(&microbit_p0_obj);
    if (microbit_pin_get_mode(&microbit_p1_obj) == microbit_pin_mode_touch)
        update(&microbit_p1_obj);
    if (microbit_pin_get_mode(&microbit_p2_obj) == microbit_pin_mode_touch)
        update(&microbit_p2_obj);
}

bool microbit_pin_debounce_is_high(const microbit_pin_obj_t *pin) {
    debounce_state_t *debounce = get_debounce_state(pin);
    return debounce->debounced_high;
}

bool microbit_pin_debounce_was_pressed(const microbit_pin_obj_t *pin) {
    debounce_state_t *debounce = get_debounce_state(pin);
    uint16_t presses = debounce->pressed;
    bool result = presses & 1;
    debounce->pressed = presses & -2;
    return result;
}

unsigned int microbit_pin_debounce_get_presses(const microbit_pin_obj_t *pin) {
    debounce_state_t *debounce = get_debounce_state(pin);
    unsigned int n_presses = debounce->pressed >> 1;
    debounce->pressed &= 1;
    return n_presses;
}

}
