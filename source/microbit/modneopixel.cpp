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

// This is the pixel channel colour ordering       |R G B W|
// WS8212/WS8212B chips use GRB-W ordering, hence: |1 0 2 3|
#define COL_IDX_RED (1)
#define COL_IDX_GREEN (0)
#define COL_IDX_BLUE (2)
#define COL_IDX_WHITE (3)

// sends bytes to pixels
// This is implemented in assembler to get the right timing
extern void sendNeopixelBuffer(uint32_t pin, uint8_t* data_address, uint16_t num_leds, uint16_t channels);

extern const mp_obj_type_t neopixel_type;

//Input args when calling neopixel.NeoPixel()?
typedef struct _neopixel_obj_t {
    mp_obj_base_t base;
    uint16_t pin_num;
    uint16_t num_pixels;
    uint16_t num_channels;
    uint8_t data[];
} neopixel_obj_t;

STATIC mp_obj_t neopixel_show_(mp_obj_t self_in);

static inline void neopixel_clear_data(neopixel_obj_t *self) {
    memset(&self->data[0], 0, self->num_channels * self->num_pixels);
}

STATIC mp_obj_t neopixel_make_new(const mp_obj_type_t *type_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    (void)type_in;
    mp_arg_check_num(n_args, n_kw, 3, 3, false);


    //TODO: Figure out how kword args work and implement a channels= or similar arg for more pythonic code
    PinName pin = (PinName)microbit_obj_get_pin_name(args[0]);
    mp_int_t num_pixels = mp_obj_get_int(args[1]);
    mp_int_t num_channels = mp_obj_get_int(args[2]);

    if (num_pixels <= 0) {
        mp_raise_ValueError("invalid number of pixels");
    }
    //at the moment only 3 or 4 channels are supported. Better to implement as bool?
    if (num_channels != 3 && num_channels != 4){
        mp_raise_ValueError("Invalid number of channels! Try 3 (RGB) or 4 (RGB-W)");
    }

    neopixel_obj_t *self = m_new_obj_var(neopixel_obj_t, uint8_t, num_channels * num_pixels);
    self->base.type = &neopixel_type;
    self->pin_num = pin;
    self->num_pixels = num_pixels;
    self->num_channels = num_channels;
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


//this is called when operating on pixel list (data[i] or similar) in python
STATIC mp_obj_t neopixel_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
    neopixel_obj_t *self = (neopixel_obj_t*)self_in;
    mp_uint_t index = mp_get_index(self->base.type, self->num_pixels, index_in, false);
    index *= self->num_channels;


    if (value == MP_OBJ_NULL) {
        // delete item
        return MP_OBJ_NULL; // op not supported
    } else if (value == MP_OBJ_SENTINEL) {
        // load and return color tuple at index

        //create the tuple with 4 channels regardless of how many we actually wanted.
        //num_channels variable will simply cut off the last channel if it's 3.
        mp_obj_t rgbw[self->num_channels] = {
            MP_OBJ_NEW_SMALL_INT(self->data[index + COL_IDX_RED]),
            MP_OBJ_NEW_SMALL_INT(self->data[index + COL_IDX_GREEN]),
            MP_OBJ_NEW_SMALL_INT(self->data[index + COL_IDX_BLUE]),
            MP_OBJ_NEW_SMALL_INT(self->data[index + COL_IDX_WHITE]),
        };


        return mp_obj_new_tuple(self->num_channels, rgbw);
    } else {
        // store the assigned tuple at the given index. 
        // Stored internally as a continuous array, with each channel at a certain offset to the index.
        mp_obj_t *rgbw;
        mp_obj_get_array_fixed_n(value, self->num_channels, &rgbw);
        mp_int_t r = mp_obj_get_int(rgbw[0]);
        mp_int_t g = mp_obj_get_int(rgbw[1]);
        mp_int_t b = mp_obj_get_int(rgbw[2]);

        //check if the number of channels passed is 3, then check for valid data range:
        if (self->num_channels == 3) {
            if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
                mp_raise_ValueError("invalid colour");
            }
        }
        //we always create 3 channels, regardless of number specified in num_channels (misleading?)
        self->data[index + COL_IDX_RED] = r;
        self->data[index + COL_IDX_GREEN] = g;
        self->data[index + COL_IDX_BLUE] = b;

        //check whether a 4th 'white' channel needs to be appended and stored (somewhat hacky!)
        if (self->num_channels == 4){

            mp_int_t w = mp_obj_get_int(rgbw[3]);

            //for RGB+W WS8212 pixels, the W channel can either be 255 or 0. Intermediate values cause flickering
            //Question: Do we allow users to do this behaviour anyway or restrict to a boolean value?
            if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || w < 0 || w > 255) {
                mp_raise_ValueError("invalid colour");
            }

        self->data[index + COL_IDX_WHITE] = w;

        }

        return mp_const_none;
    }
}
//shorthand function to wipe data list and call show() again - effectively turning off pixels.
STATIC mp_obj_t neopixel_clear_(mp_obj_t self_in) {
    neopixel_obj_t *self = (neopixel_obj_t*)self_in;
    neopixel_clear_data(self);
    neopixel_show_(self_in);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(neopixel_clear_obj, neopixel_clear_);

//actually executes changes made to data list. Corresponds to .show()
STATIC mp_obj_t neopixel_show_(mp_obj_t self_in) {
    neopixel_obj_t *self = (neopixel_obj_t*)self_in;

    // Call external assember to do the time critical pin waggling.
    // Be aware that this runs with interrupts off.
    uint32_t pin_mask = (1UL << self->pin_num);
    NRF_GPIO->OUTCLR = pin_mask;
    sendNeopixelBuffer(pin_mask, &self->data[0], self->num_pixels, self->num_channels);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(neopixel_show_obj, neopixel_show_);

//maps internal methods to python Q-Strings. 
//don't quite understand how this work but assume text after QSTR_ is the name (e.g. MP_QSTR_clear = .clear())
STATIC const mp_map_elem_t neopixel_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_clear), (mp_obj_t)&neopixel_clear_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_show), (mp_obj_t)&neopixel_show_obj },
};

STATIC MP_DEFINE_CONST_DICT(neopixel_locals_dict, neopixel_locals_dict_table);

//Internal stuff for defining python object structure?
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
