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

typedef struct _microbit_display_obj_t {
    mp_obj_base_t base;
    MicroBitDisplay *display;
} microbit_display_obj_t;

mp_obj_t microbit_display_print(mp_uint_t n_args, const mp_obj_t *args) {
    microbit_display_obj_t *self = (microbit_display_obj_t*)args[0];
    mp_uint_t len;
    const char *str = mp_obj_str_get_data(args[1], &len);
    if (len == 0) {
        // no chars, do nothing
    } else if (len == 1) {
        // single char
        char c = str[0];
        if (n_args == 2) {
            self->display->print(c);
        } else {
            self->display->print(c, mp_obj_get_int(args[2]));
        }
    } else {
        // a string
        ManagedString s(str, len);
        if (n_args == 2) {
            self->display->print(s);
        } else {
            self->display->print(s, mp_obj_get_int(args[2]));
        }
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_display_print_obj, 2, 3, microbit_display_print);

mp_obj_t microbit_display_scroll(mp_uint_t n_args, const mp_obj_t *args) {
    microbit_display_obj_t *self = (microbit_display_obj_t*)args[0];
    ManagedString s(mp_obj_str_get_str(args[1]));
    if (n_args == 2) {
        self->display->scroll(s);
    } else {
        self->display->scroll(s, mp_obj_get_int(args[2]));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_display_scroll_obj, 2, 3, microbit_display_scroll);

mp_obj_t microbit_display_animate(mp_uint_t n_args, const mp_obj_t *args) {
    microbit_display_obj_t *self = (microbit_display_obj_t*)args[0];
    MicroBitImage img(*microbit_obj_get_image(args[1]));
    mp_int_t delay = mp_obj_get_int(args[2]);
    mp_int_t stride = mp_obj_get_int(args[3]);
    if (n_args == 4) {
        self->display->animate(img, delay, stride);
    } else {
        self->display->animate(img, delay, stride, mp_obj_get_int(args[4]));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_display_animate_obj, 4, 5, microbit_display_animate);

mp_obj_t microbit_display_animate_async(mp_uint_t n_args, const mp_obj_t *args) {
    microbit_display_obj_t *self = (microbit_display_obj_t*)args[0];
    MicroBitImage img(*microbit_obj_get_image(args[1]));
    mp_int_t delay = mp_obj_get_int(args[2]);
    mp_int_t stride = mp_obj_get_int(args[3]);
    if (n_args == 4) {
        self->display->animateAsync(img, delay, stride);
    } else {
        self->display->animateAsync(img, delay, stride, mp_obj_get_int(args[4]));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_display_animate_async_obj, 4, 5, microbit_display_animate_async);

mp_obj_t microbit_display_clear(void) {
    uBit.display.clear();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_display_clear_obj, microbit_display_clear);

mp_obj_t microbit_display_set_brightness(mp_obj_t self_in, mp_obj_t br_in) {
    microbit_display_obj_t *self = (microbit_display_obj_t*)self_in;
    self->display->setBrightness(mp_obj_get_int(br_in));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_display_set_brightness_obj, microbit_display_set_brightness);

mp_obj_t microbit_display_set_display_mode(mp_obj_t self_in, mp_obj_t mode_in) {
    microbit_display_obj_t *self = (microbit_display_obj_t*)self_in;
    self->display->setDisplayMode((DisplayMode)mp_obj_get_int(mode_in));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_display_set_display_mode_obj, microbit_display_set_display_mode);

STATIC const mp_map_elem_t microbit_display_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_image), (mp_obj_t)&microbit_image_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_set_brightness), (mp_obj_t)&microbit_display_set_brightness_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_display_mode), (mp_obj_t)&microbit_display_set_display_mode_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_print), (mp_obj_t)&microbit_display_print_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_scroll), (mp_obj_t)&microbit_display_scroll_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_animate), (mp_obj_t)&microbit_display_animate_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_animate_async), (mp_obj_t)&microbit_display_animate_async_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_clear), (mp_obj_t)&microbit_display_clear_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_display_locals_dict, microbit_display_locals_dict_table);

STATIC const mp_obj_type_t microbit_display_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitDisplay,
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
    .bases_tuple = MP_OBJ_NULL,
    /* .locals_dict = */ (mp_obj_t)&microbit_display_locals_dict,
};

const microbit_display_obj_t microbit_display_obj = {
    {&microbit_display_type},
    .display = &uBit.display
};

}
