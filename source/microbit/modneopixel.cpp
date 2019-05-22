/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Damien P. George
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

#include <stdio.h>
#include <string.h>
#include "gpio_api.h"
#include "nrf_gpio.h"
#include "py/runtime0.h"
#include "py/runtime.h"
#include "microbit/modmicrobit.h"

// This is the pixel colour ordering
#define COL_NUM_COMPONENTS (3)
#define COL_IDX_RED (1)
#define COL_IDX_GREEN (0)
#define COL_IDX_BLUE (2)

// This is implemented in assembler to get the right timing
extern void sendNeopixelBuffer(uint32_t pin, uint8_t* data_address, uint16_t num_leds);

extern const mp_obj_type_t neopixel_type;

typedef struct _neopixel_obj_t {
    mp_obj_base_t base;
    uint16_t pin_num;
    uint16_t num_pixels;
    uint8_t data[];
} neopixel_obj_t;

STATIC mp_obj_t neopixel_show_(mp_obj_t self_in);

static inline void neopixel_clear_data(neopixel_obj_t *self) {
    memset(&self->data[0], 0, COL_NUM_COMPONENTS * self->num_pixels);
}

STATIC mp_obj_t neopixel_make_new(const mp_obj_type_t *type_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    (void)type_in;
    mp_arg_check_num(n_args, n_kw, 2, 2, false);

    PinName pin = (PinName)microbit_obj_get_pin_name(args[0]);
    mp_int_t num_pixels = mp_obj_get_int(args[1]);

    if (num_pixels <= 0) {
        mp_raise_ValueError("invalid number of pixels");
    }

    neopixel_obj_t *self = m_new_obj_var(neopixel_obj_t, uint8_t, COL_NUM_COMPONENTS * num_pixels);
    self->base.type = &neopixel_type;
    self->pin_num = pin;
    self->num_pixels = num_pixels;
    neopixel_clear_data(self);

    // Configure pin as output and set it low
    nrf_gpio_cfg_output(pin);
    NRF_GPIO->OUTCLR = (1UL << pin);

    return self;
}

STATIC mp_obj_t neopixel_unary_op(mp_uint_t op, mp_obj_t self_in) {
    neopixel_obj_t *self = (neopixel_obj_t*)self_in;
    switch (op) {
        case MP_UNARY_OP_LEN: return MP_OBJ_NEW_SMALL_INT(self->num_pixels);
        default: return MP_OBJ_NULL; // op not supported
    }
}

STATIC mp_obj_t neopixel_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
    neopixel_obj_t *self = (neopixel_obj_t*)self_in;
    mp_uint_t index = mp_get_index(self->base.type, self->num_pixels, index_in, false);
    index *= COL_NUM_COMPONENTS;
    if (value == MP_OBJ_NULL) {
        // delete item
        return MP_OBJ_NULL; // op not supported
    } else if (value == MP_OBJ_SENTINEL) {
        // load
        mp_obj_t rgb[COL_NUM_COMPONENTS] = {
            MP_OBJ_NEW_SMALL_INT(self->data[index + COL_IDX_RED]),
            MP_OBJ_NEW_SMALL_INT(self->data[index + COL_IDX_GREEN]),
            MP_OBJ_NEW_SMALL_INT(self->data[index + COL_IDX_BLUE]),
        };
        return mp_obj_new_tuple(COL_NUM_COMPONENTS, rgb);
    } else {
        // store
        mp_obj_t *rgb;
        mp_obj_get_array_fixed_n(value, COL_NUM_COMPONENTS, &rgb);
        mp_int_t r = mp_obj_get_int(rgb[0]);
        mp_int_t g = mp_obj_get_int(rgb[1]);
        mp_int_t b = mp_obj_get_int(rgb[2]);
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
            mp_raise_ValueError("invalid colour");
        }
        self->data[index + COL_IDX_RED] = r;
        self->data[index + COL_IDX_GREEN] = g;
        self->data[index + COL_IDX_BLUE] = b;
        return mp_const_none;
    }
}

STATIC mp_obj_t neopixel_clear_(mp_obj_t self_in) {
    neopixel_obj_t *self = (neopixel_obj_t*)self_in;
    neopixel_clear_data(self);
    neopixel_show_(self_in);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(neopixel_clear_obj, neopixel_clear_);

STATIC mp_obj_t neopixel_show_(mp_obj_t self_in) {
    neopixel_obj_t *self = (neopixel_obj_t*)self_in;

    // Call external assember to do the time critical pin waggling.
    // Be aware that this runs with interrupts off.
    uint32_t pin_mask = (1UL << self->pin_num);
    NRF_GPIO->OUTCLR = pin_mask;
    sendNeopixelBuffer(pin_mask, &self->data[0], self->num_pixels);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(neopixel_show_obj, neopixel_show_);

STATIC const mp_map_elem_t neopixel_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_clear), (mp_obj_t)&neopixel_clear_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_show), (mp_obj_t)&neopixel_show_obj },
};

STATIC MP_DEFINE_CONST_DICT(neopixel_locals_dict, neopixel_locals_dict_table);

const mp_obj_type_t neopixel_type = {
    { &mp_type_type },
    .name = MP_QSTR_NeoPixel,
    .print = NULL,
    .make_new = neopixel_make_new,
    .call = NULL,
    .unary_op = neopixel_unary_op,
    .binary_op = NULL,
    .attr = NULL,
    .subscr = neopixel_subscr,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .protocol = NULL,
    .parent = NULL,
    .locals_dict = (mp_obj_dict_t*)&neopixel_locals_dict,
};

STATIC const mp_map_elem_t neopixel_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_neopixel) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_NeoPixel), (mp_obj_t)&neopixel_type },
};

STATIC MP_DEFINE_CONST_DICT(neopixel_module_globals, neopixel_module_globals_table);

const mp_obj_module_t neopixel_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&neopixel_module_globals,
};

}
