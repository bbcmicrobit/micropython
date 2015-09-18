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
    // TODO support async mode

    microbit_display_obj_t *self = (microbit_display_obj_t*)args[0];

    // cancel any animations
    MP_STATE_PORT(async_data)[0] = NULL;
    MP_STATE_PORT(async_data)[1] = NULL;

    if (MP_OBJ_IS_STR(args[1])) {
        // arg is a string object
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
            // a long string
            ManagedString s(str, len);
            if (n_args == 2) {
                self->display->print(s);
            } else {
                self->display->print(s, mp_obj_get_int(args[2]));
            }
        }
    } else {
        // arg should be an image
        MicroBitImage *image = microbit_obj_get_image(args[1]);
        MicroBitImage img(*image);
        self->display->print(img, 0, 0, 0, 0);
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_display_print_obj, 2, 3, microbit_display_print);

mp_obj_t microbit_display_scroll(mp_uint_t n_args, const mp_obj_t *args) {
    // TODO support async mode
    // TODO support images

    microbit_display_obj_t *self = (microbit_display_obj_t*)args[0];

    // cancel any animations
    MP_STATE_PORT(async_data)[0] = NULL;
    MP_STATE_PORT(async_data)[1] = NULL;

    ManagedString s(mp_obj_str_get_str(args[1]));
    if (n_args == 2) {
        self->display->scroll(s);
    } else {
        self->display->scroll(s, mp_obj_get_int(args[2]));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_display_scroll_obj, 2, 3, microbit_display_scroll);

static int async_mode;
static mp_uint_t async_cur;
static mp_uint_t async_len;
static mp_obj_t *async_items;
static int async_delay;
static int async_stride;
static int async_start;
static bool async_loop;

void microbit_display_event(void) {
    if (MP_STATE_PORT(async_data)[0] != NULL && MP_STATE_PORT(async_data)[1] != NULL) {
        microbit_display_obj_t *display = (microbit_display_obj_t*)MP_STATE_PORT(async_data)[0];
        switch (async_mode) {
            case 1: {
                MicroBitImage *image = (MicroBitImage*)MP_STATE_PORT(async_data)[1];
                display->display->animateAsync(*image, async_delay, async_stride, async_start);
                break;
            }

            case 2: {
                // animate a list
                if (++async_cur >= async_len) {
                    // end of list
                    if (async_loop) {
                        // loop back to start
                        async_cur = 0;
                    } else {
                        // finish
                        MP_STATE_PORT(async_data)[0] = NULL;
                        MP_STATE_PORT(async_data)[1] = NULL;
                        return;
                    }
                }
                MicroBitImage *image = microbit_obj_get_image(async_items[async_cur]);
                display->display->animateAsync(*image, async_delay, async_stride, async_start);
                break;
            }
        }
    }
}

STATIC mp_obj_t microbit_display_animate(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_image,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_delay,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_stride,   MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5} },
        { MP_QSTR_start,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = MICROBIT_DISPLAY_ANIMATE_DEFAULT_POS} },
        { MP_QSTR_async,    MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_repeat,   MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
    };

    // parse args
    microbit_display_obj_t *self = (microbit_display_obj_t*)pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // reset repeat state
    MP_STATE_PORT(async_data)[0] = NULL;
    MP_STATE_PORT(async_data)[1] = NULL;

    mp_obj_type_t *arg0_type = mp_obj_get_type(args[0].u_obj);
    if (arg0_type == &mp_type_tuple || arg0_type == &mp_type_list) {
        mp_uint_t len;
        mp_obj_t *items;
        mp_obj_get_array(args[0].u_obj, &len, &items);

        if (!args[4].u_bool) {
            for (mp_uint_t i = 0; i < len; ++i) {
                MicroBitImage *image = microbit_obj_get_image(items[i]);
                self->display->animate(*image, args[1].u_int, args[2].u_int, args[3].u_int);
            }
        } else if (len > 0) {
            // in background
            MicroBitImage *image = microbit_obj_get_image(items[0]);
            self->display->animateAsync(*image, args[1].u_int, args[2].u_int, args[3].u_int);
            MP_STATE_PORT(async_data)[0] = self;
            MP_STATE_PORT(async_data)[1] = args[0].u_obj; // so it doesn't get GC'd
            async_mode = 2;
            async_cur = 0;
            async_len = len;
            async_items = items;
            async_delay = args[1].u_int;
            async_stride = args[2].u_int;
            async_start = args[3].u_int;
            async_loop = args[5].u_bool;
        }
    } else {
        MicroBitImage *image = microbit_obj_get_image(args[0].u_obj);

        if (!args[4].u_bool) {
            self->display->animate(*image, args[1].u_int, args[2].u_int, args[3].u_int);
        } else {
            self->display->animateAsync(*image, args[1].u_int, args[2].u_int, args[3].u_int);
        }

        if (args[5].u_bool) {
            // enable repeat
            MP_STATE_PORT(async_data)[0] = self;
            MP_STATE_PORT(async_data)[1] = image;
            async_mode = 1;
            async_delay = args[1].u_int;
            async_stride = args[2].u_int;
            async_start = args[3].u_int;
        }
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(microbit_display_animate_obj, 1, microbit_display_animate);

mp_obj_t microbit_display_clear(void) {
    // reset repeat state, cancel animation and clear screen
    MP_STATE_PORT(async_data)[0] = NULL;
    MP_STATE_PORT(async_data)[1] = NULL;
    uBit.display.resetAnimation(0);
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
