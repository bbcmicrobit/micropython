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
#include "py/objstr.h"
#include "modmicrobit.h"

typedef struct _microbit_image_obj_t {
    mp_obj_base_t base;
    MicroBitImage *image;
} microbit_image_obj_t;

STATIC mp_obj_t microbit_image_make_new(mp_obj_t type_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    (void)type_in;
    mp_arg_check_num(n_args, n_kw, 0, 3, false);

    switch (n_args) {
        case 0: {
            // return a new, empty image
            microbit_image_obj_t *img = m_new_obj(microbit_image_obj_t);
            img->base.type = &microbit_image_type;
            img->image = new MicroBitImage();
            return img;
        }

        case 1: {
            // make image from string
            microbit_image_obj_t *img = m_new_obj(microbit_image_obj_t);
            img->base.type = &microbit_image_type;
            img->image = new MicroBitImage(mp_obj_str_get_str(args[0]));
            return img;
        }

        case 2: {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError,
                "Image() takes 0, 1 or 3 arguments"));
        }

        case 3:
        default: {
            // make image from x,y,data

            mp_int_t w = mp_obj_get_int(args[0]);
            mp_int_t h = mp_obj_get_int(args[1]);
            mp_buffer_info_t bufinfo;
            mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);

            if (w < 0 || h < 0 || (size_t)(w * h) != bufinfo.len) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError,
                    "image data is incorrect size"));
            }

            microbit_image_obj_t *img = m_new_obj(microbit_image_obj_t);
            img->base.type = &microbit_image_type;
            img->image = new MicroBitImage(w, h, (const uint8_t*)bufinfo.buf);

            return img;
        }
    }
}

mp_obj_t microbit_image_set_pixel_value(mp_uint_t n_args, const mp_obj_t *args) {
    (void)n_args;
    microbit_image_obj_t *self = (microbit_image_obj_t*)args[0];
    self->image->setPixelValue(mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_image_set_pixel_value_obj, 4, 4, microbit_image_set_pixel_value);

mp_obj_t microbit_image_get_pixel_value(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    int value = self->image->getPixelValue(mp_obj_get_int(x_in), mp_obj_get_int(y_in));
    return MP_OBJ_NEW_SMALL_INT(value);
}
MP_DEFINE_CONST_FUN_OBJ_3(microbit_image_get_pixel_value_obj, microbit_image_get_pixel_value);

mp_obj_t microbit_image_shift_left(mp_obj_t self_in, mp_obj_t n_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    self->image->shiftLeft(mp_obj_get_int(n_in));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_image_shift_left_obj, microbit_image_shift_left);

mp_obj_t microbit_image_shift_right(mp_obj_t self_in, mp_obj_t n_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    self->image->shiftRight(mp_obj_get_int(n_in));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_image_shift_right_obj, microbit_image_shift_right);

mp_obj_t microbit_image_shift_up(mp_obj_t self_in, mp_obj_t n_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    self->image->shiftUp(mp_obj_get_int(n_in));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_image_shift_up_obj, microbit_image_shift_up);

mp_obj_t microbit_image_shift_down(mp_obj_t self_in, mp_obj_t n_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    self->image->shiftDown(mp_obj_get_int(n_in));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_image_shift_down_obj, microbit_image_shift_down);

STATIC const MP_DEFINE_STR_OBJ(microbit_image_heart_obj,
    "0,1,0,1,0, 0,0,0,0,0\n"
    "1,1,1,1,1, 0,1,0,1,0\n"
    "1,1,1,1,1, 0,1,1,1,0\n"
    "0,1,1,1,0, 0 0 1,0,0\n"
    "0,0,1,0,0, 0,0,0,0,0\n"
);

STATIC const mp_map_elem_t microbit_image_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_pixel_value), (mp_obj_t)&microbit_image_set_pixel_value_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_pixel_value), (mp_obj_t)&microbit_image_get_pixel_value_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shift_left), (mp_obj_t)&microbit_image_shift_left_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shift_right), (mp_obj_t)&microbit_image_shift_right_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shift_up), (mp_obj_t)&microbit_image_shift_up_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shift_down), (mp_obj_t)&microbit_image_shift_down_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_HEART), (mp_obj_t)&microbit_image_heart_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_image_locals_dict, microbit_image_locals_dict_table);

const mp_obj_type_t microbit_image_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitImage,
    .print = NULL,
    .make_new = microbit_image_make_new,
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
    /* .locals_dict = */ (mp_obj_t)&microbit_image_locals_dict,
};

const microbit_image_obj_t microbit_image_obj = {
    {&microbit_image_type},
    .image = &uBit.display.image,
};

MicroBitImage *microbit_obj_get_image(mp_obj_t o) {
    if (mp_obj_get_type(o) == &microbit_image_type) {
        microbit_image_obj_t *img = (microbit_image_obj_t*)o;
        return img->image;
    } else {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "expecting an image"));
    }
}

}
