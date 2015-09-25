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

STATIC void microbit_image_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    for (int y = 0; y < self->image->getHeight(); ++y) {
        for (int x = 0; x < self->image->getWidth(); ++x) {
            if (x > 0) {
                mp_printf(print, ",");
            }
            mp_printf(print, "%u", self->image->getPixelValue(x, y));
        }
        mp_printf(print, "\n");
    }
}

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

mp_obj_t microbit_image_get_width(mp_obj_t self_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    return MP_OBJ_NEW_SMALL_INT(self->image->getWidth());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_image_get_width_obj, microbit_image_get_width);

mp_obj_t microbit_image_get_height(mp_obj_t self_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    return MP_OBJ_NEW_SMALL_INT(self->image->getHeight());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_image_get_height_obj, microbit_image_get_height);

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

STATIC const mp_map_elem_t microbit_image_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_width), (mp_obj_t)&microbit_image_get_width_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_height), (mp_obj_t)&microbit_image_get_height_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_pixel_value), (mp_obj_t)&microbit_image_set_pixel_value_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_pixel_value), (mp_obj_t)&microbit_image_get_pixel_value_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shift_left), (mp_obj_t)&microbit_image_shift_left_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shift_right), (mp_obj_t)&microbit_image_shift_right_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shift_up), (mp_obj_t)&microbit_image_shift_up_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shift_down), (mp_obj_t)&microbit_image_shift_down_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_HEART), (mp_obj_t)&microbit_const_image_heart_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_HEART_SMALL), (mp_obj_t)&microbit_const_image_heart_small_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_HAPPY), (mp_obj_t)&microbit_const_image_happy_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SAD), (mp_obj_t)&microbit_const_image_sad_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CONFUSED), (mp_obj_t)&microbit_const_image_confused_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ANGRY), (mp_obj_t)&microbit_const_image_angry_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ASLEEP), (mp_obj_t)&microbit_const_image_asleep_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SURPRISED), (mp_obj_t)&microbit_const_image_surprised_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_YES), (mp_obj_t)&microbit_const_image_yes_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_NO), (mp_obj_t)&microbit_const_image_no_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK12), (mp_obj_t)&microbit_const_image_clock12_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK1), (mp_obj_t)&microbit_const_image_clock1_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK2), (mp_obj_t)&microbit_const_image_clock2_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK3), (mp_obj_t)&microbit_const_image_clock3_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK4), (mp_obj_t)&microbit_const_image_clock4_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK5), (mp_obj_t)&microbit_const_image_clock5_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK6), (mp_obj_t)&microbit_const_image_clock6_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK7), (mp_obj_t)&microbit_const_image_clock7_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK8), (mp_obj_t)&microbit_const_image_clock8_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK9), (mp_obj_t)&microbit_const_image_clock9_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK10), (mp_obj_t)&microbit_const_image_clock10_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK11), (mp_obj_t)&microbit_const_image_clock11_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_N), (mp_obj_t)&microbit_const_image_arrow_n_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_NE), (mp_obj_t)&microbit_const_image_arrow_ne_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_E), (mp_obj_t)&microbit_const_image_arrow_e_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_SE), (mp_obj_t)&microbit_const_image_arrow_se_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_S), (mp_obj_t)&microbit_const_image_arrow_s_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_SW), (mp_obj_t)&microbit_const_image_arrow_sw_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_W), (mp_obj_t)&microbit_const_image_arrow_w_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_NW), (mp_obj_t)&microbit_const_image_arrow_nw_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_image_locals_dict, microbit_image_locals_dict_table);

const mp_obj_type_t microbit_image_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitImage,
    .print = microbit_image_print,
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
        return microbit_obj_get_const_image(o);
    }
}

}
