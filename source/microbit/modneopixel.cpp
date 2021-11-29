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
#define COL_IDX_RED (1)
#define COL_IDX_GREEN (0)
#define COL_IDX_BLUE (2)
#define COL_IDX_WHITE (3)
#define COL_IDX_MAP (COL_IDX_WHITE << 12 | COL_IDX_BLUE << 8 | COL_IDX_GREEN << 4 | COL_IDX_RED)

// This is implemented in assembler to get the right timing
extern void sendNeopixelBuffer(uint32_t pin, uint8_t* data_address, uint16_t num_bytes);

extern const mp_obj_type_t neopixel_type;

typedef struct _neopixel_obj_t {
    mp_obj_base_t base;
    uint16_t pin_num;
    uint16_t num_pixels;
    uint8_t bytes_per_pixel;
    uint8_t data[];
} neopixel_obj_t;

STATIC mp_obj_t neopixel_show_(mp_obj_t self_in);

static inline void neopixel_clear_data(neopixel_obj_t *self) {
    memset(&self->data[0], 0, self->bytes_per_pixel * self->num_pixels);
}

STATIC mp_obj_t neopixel_make_new(const mp_obj_type_t *type_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    (void)type_in;

    enum { ARG_pin, ARG_n, ARG_bpp };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bpp, MP_ARG_INT, {.u_int = 3} },
    };
    mp_arg_val_t vals[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, vals);

    PinName pin = (PinName)microbit_obj_get_pin_name(vals[ARG_pin].u_obj);
    mp_int_t num_pixels = vals[ARG_n].u_int;
    mp_int_t bytes_per_pixel = vals[ARG_bpp].u_int;

    if (num_pixels <= 0) {
        mp_raise_ValueError("invalid number of pixels");
    }

    if (!(bytes_per_pixel == 3 || bytes_per_pixel == 4)) {
        mp_raise_ValueError("invalid bpp");
    }

    neopixel_obj_t *self = m_new_obj_var(neopixel_obj_t, uint8_t, bytes_per_pixel * num_pixels);
    self->base.type = &neopixel_type;
    self->pin_num = pin;
    self->num_pixels = num_pixels;
    self->bytes_per_pixel = bytes_per_pixel;
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
    index *= self->bytes_per_pixel;
    if (value == MP_OBJ_NULL) {
        // delete item
        return MP_OBJ_NULL; // op not supported
    } else if (value == MP_OBJ_SENTINEL) {
        // load
        mp_obj_t rgb[] = {
            MP_OBJ_NEW_SMALL_INT(self->data[index + COL_IDX_RED]),
            MP_OBJ_NEW_SMALL_INT(self->data[index + COL_IDX_GREEN]),
            MP_OBJ_NEW_SMALL_INT(self->data[index + COL_IDX_BLUE]),
            MP_OBJ_NEW_SMALL_INT(self->data[index + COL_IDX_WHITE]), // unused if bpp==3
        };
        return mp_obj_new_tuple(self->bytes_per_pixel, rgb);
    } else {
        // store
        mp_obj_t *rgb;
        mp_obj_get_array_fixed_n(value, self->bytes_per_pixel, &rgb);
        for (uint8_t i = 0; i < self->bytes_per_pixel; ++i) {
            mp_int_t c = mp_obj_get_int(rgb[i]);
            if (c < 0 || c > 255) {
                mp_raise_ValueError("invalid colour");
            }
            self->data[index + ((COL_IDX_MAP >> (4 * i)) & 0xf)] = c;
        }
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
    sendNeopixelBuffer(pin_mask, &self->data[0], self->bytes_per_pixel * self->num_pixels);

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
