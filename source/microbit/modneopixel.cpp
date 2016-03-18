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
#include "gpio_api.h"
#include "py/runtime0.h"
#include "py/runtime.h"
#include "lib/neopixel.h"
#include "microbitobj.h"

extern const mp_obj_type_t neopixel_type;

typedef struct _neopixel_obj_t {
    mp_obj_base_t base;
    neopixel_strip_t strip;
} neopixel_obj_t;

STATIC mp_obj_t neopixel_make_new(const mp_obj_type_t *type_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    (void)type_in;
    mp_arg_check_num(n_args, n_kw, 2, 2, false);

    PinName pin = microbit_obj_get_pin_name(args[0]);
    mp_int_t num_pixels = mp_obj_get_int(args[1]);

    if (num_pixels <= 0) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid number of pixels"));
    }

    neopixel_obj_t *self = m_new_obj(neopixel_obj_t);
    self->base.type = &neopixel_type;
    neopixel_init(&self->strip, pin, num_pixels);

    return self;
}

STATIC mp_obj_t neopixel_unary_op(mp_uint_t op, mp_obj_t self_in) {
    neopixel_obj_t *self = (neopixel_obj_t*)self_in;
    switch (op) {
        case MP_UNARY_OP_LEN: return MP_OBJ_NEW_SMALL_INT(self->strip.num_leds);
        default: return MP_OBJ_NULL; // op not supported
    }
}

STATIC mp_obj_t neopixel_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
    neopixel_obj_t *self = (neopixel_obj_t*)self_in;
    mp_uint_t index = mp_get_index(self->base.type, self->strip.num_leds, index_in, false);
    if (value == MP_OBJ_NULL) {
        // delete item
        return MP_OBJ_NULL; // op not supported
    } else if (value == MP_OBJ_SENTINEL) {
        // load
        mp_obj_t rgb[3] = {
            MP_OBJ_NEW_SMALL_INT(self->strip.leds[index].simple.r),
            MP_OBJ_NEW_SMALL_INT(self->strip.leds[index].simple.g),
            MP_OBJ_NEW_SMALL_INT(self->strip.leds[index].simple.b),
        };
        return mp_obj_new_tuple(3, rgb);
    } else {
        // store
        mp_obj_t *rgb;
        mp_obj_get_array_fixed_n(value, 3, &rgb);
        mp_int_t r = mp_obj_get_int(rgb[0]);
        mp_int_t g = mp_obj_get_int(rgb[1]);
        mp_int_t b = mp_obj_get_int(rgb[2]);
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid colour"));
        }
        neopixel_set_color(&self->strip, index, r, g, b);
        return mp_const_none;
    }
}

STATIC mp_obj_t neopixel_clear_(mp_obj_t self_in) {
    neopixel_obj_t *self = (neopixel_obj_t*)self_in;
    neopixel_clear(&self->strip);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(neopixel_clear_obj, neopixel_clear_);

STATIC mp_obj_t neopixel_show_(mp_obj_t self_in) {
    neopixel_obj_t *self = (neopixel_obj_t*)self_in;
    neopixel_show(&self->strip);
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
    .stream_p = NULL,
    .bases_tuple = NULL,
    .locals_dict = (mp_obj_dict_t*)&neopixel_locals_dict,
};

STATIC const mp_map_elem_t neopixel_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_neopixel) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_NeoPixel), (mp_obj_t)&neopixel_type },
};

STATIC MP_DEFINE_CONST_DICT(neopixel_module_globals, neopixel_module_globals_table);

const mp_obj_module_t neopixel_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_neopixel,
    .globals = (mp_obj_dict_t*)&neopixel_module_globals,
};

}
