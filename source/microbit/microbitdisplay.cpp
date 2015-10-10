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
#include "microbitimage.h"

typedef struct _microbit_display_obj_t {
    mp_obj_base_t base;
    MicroBitDisplay *display;
} microbit_display_obj_t;

STATIC void display_print(const microbit_display_obj_t *display, microbit_image_obj_t *image) {
    MicroBitImage *display_image = &display->display->image;
    mp_int_t w = min(image->width(), 5);
    mp_int_t h = min(image->height(), 5);
    mp_int_t x = 0;
    for (; x < w; ++x) {
        mp_int_t y = 0;
        for (; y < h; ++y) {
            display_image->setPixelValue(x, y, BRIGHTNESS_SCALE[image->getPixelValue(x, y)]);
        }
        for (; y < 5; ++y) {
            display_image->setPixelValue(x, y, 0);
        }
    }
    for (; x < 5; ++x) {
        for (mp_int_t y = 0; y < 5; ++y) {
            display_image->setPixelValue(x, y, 0);
        }
    }
}

STATIC void do_synchronous_animation(microbit_display_obj_t *display, mp_obj_t iterable, mp_int_t delay, bool repeat);

mp_obj_t microbit_display_print(mp_uint_t n_args, const mp_obj_t *args) {
    // TODO support async mode

    microbit_display_obj_t *self = (microbit_display_obj_t*)args[0];

    // cancel any animations
    MP_STATE_PORT(async_data)[0] = NULL;
    MP_STATE_PORT(async_data)[1] = NULL;
    MP_STATE_PORT(async_data)[2] = NULL;

    if (MP_OBJ_IS_STR(args[1])) {
        // arg is a string object
        mp_uint_t len;
        const char *str = mp_obj_str_get_data(args[1], &len);
        if (len == 0) {
            // no chars, do nothing
        } else if (len == 1) {
            // single char
            display_print(self, microbit_image_for_char(str[0]));
        } else {
            mp_int_t delay;
            if (n_args == 3) {
                delay = mp_obj_get_int(args[2]);
            } else {
                delay = MICROBIT_DEFAULT_PRINT_SPEED;
            }
            do_synchronous_animation(self, args[1], delay, false);
        }
    } else if (mp_obj_get_type(args[1]) == &microbit_image_type) {
        display_print(self, (microbit_image_obj_t *)args[1]);
    } else {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "expecting an image or a st"));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_display_print_obj, 2, 3, microbit_display_print);

#define ASYNC_MODE_STOPPED 0
#define ASYNC_MODE_ANIMATION 1
#define ASYNC_MODE_CLEAR 2
#define ASYNC_MODE_WAIT_ONLY 3

static int async_mode = ASYNC_MODE_STOPPED;
static mp_obj_t async_repeat_iterable = NULL;
static mp_obj_t async_iterator = NULL;
static const char *async_error = NULL;
static uint16_t async_nonce = 0;
static int async_delay = 1000;
static int async_tick = 0;

STATIC void wakeup_event() {
    // Wake up any fibers that were blocked on the animation (if any).
    MicroBitEvent event(MICROBIT_ID_ALERT, async_nonce);
}

STATIC void wait_for_event() {
    async_nonce = uBit.MessageBus.nonce();
    fiber_wait_for_event(MICROBIT_ID_ALERT, async_nonce);
    if (async_error) {
        const char *msg = async_error;
        async_error = NULL;
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, msg));
    }
}

STATIC void async_stop(void) {
    async_repeat_iterable = NULL;
    async_iterator = NULL;
    async_mode = ASYNC_MODE_STOPPED;
    async_tick = 0;
    async_delay = 1000;
    MP_STATE_PORT(async_data)[0] = NULL;
    MP_STATE_PORT(async_data)[1] = NULL;
    MP_STATE_PORT(async_data)[2] = NULL;
    wakeup_event();
}

void microbit_display_tick(void) {
    async_tick += FIBER_TICK_PERIOD_MS;
    if(async_tick < async_delay)
        return;
    async_tick = 0;
    switch (async_mode) {
        case ASYNC_MODE_WAIT_ONLY:
            wakeup_event();
            break;
        case ASYNC_MODE_ANIMATION:
        {
            if (MP_STATE_PORT(async_data)[0] == NULL || MP_STATE_PORT(async_data)[1] == NULL) {
                break;
            }
            microbit_display_obj_t *display = (microbit_display_obj_t*)MP_STATE_PORT(async_data)[0];
            /* WARNING: We are executing in an interrupt handler.
                * If an exception is raised here, then a reset is the only way to recover */
            mp_obj_t obj = mp_iternext(async_iterator);
            if (obj == MP_OBJ_STOP_ITERATION) {
                if (async_repeat_iterable) {
                    async_iterator = mp_getiter(async_repeat_iterable);
                } else {
                    display_print(display, (microbit_image_obj_t *)&BLANK_IMAGE);
                    async_stop();
                }
            } else if (mp_obj_get_type(obj) == &microbit_image_type) {
                display_print(display, (microbit_image_obj_t *)obj);
            } else if (MP_OBJ_IS_STR(obj)) {
                mp_uint_t len;
                const char *str = mp_obj_str_get_data(obj, &len);
                if (len == 1) {
                    display_print(display, microbit_image_for_char(str[0]));
                } else {
                    async_error = "items in animation sequence must be images or a single character strings.";
                }
            } else {
                async_error = "items in animation sequence must be images or a single character strings.";
                async_stop();
            }
            break;
        }
        case ASYNC_MODE_CLEAR:
            display_print(&microbit_display_obj, (microbit_image_obj_t *)&BLANK_IMAGE);
            async_stop();
            break;
    }
}

STATIC void do_synchronous_animation_once(microbit_display_obj_t *display, mp_obj_t iterator, mp_int_t delay) {
    do {
        async_tick = 0;
        async_mode = ASYNC_MODE_WAIT_ONLY;
        async_delay = delay;
        mp_obj_t obj = mp_iternext(iterator);
        if (obj == MP_OBJ_STOP_ITERATION) {
            break;
        } else if (mp_obj_get_type(obj) == &microbit_image_type) {
            display_print(display, (microbit_image_obj_t *)obj);
        } else if (MP_OBJ_IS_STR(obj)) {
            mp_uint_t len;
            const char *str = mp_obj_str_get_data(obj, &len);
            if (len == 1) {
                display_print(display, microbit_image_for_char(str[0]));
            } else {
                async_mode = ASYNC_MODE_STOPPED;
                nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "items in animation sequence must be images or single characters."));
            }
        } else {
            async_mode = ASYNC_MODE_STOPPED;
            nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "items in animation sequence must be images or single characters."));
        }
        wait_for_event();
    } while (1);
    async_mode = ASYNC_MODE_STOPPED;
}

STATIC void do_synchronous_animation(microbit_display_obj_t *display, mp_obj_t iterable, mp_int_t delay, bool repeat) {
    do {
        mp_obj_t iterator = mp_getiter(iterable);
        do_synchronous_animation_once(display, iterator, delay);
    } while(repeat);
}

STATIC void start_asynchronous_animation(microbit_display_obj_t *self, mp_obj_t iterable, mp_int_t delay, bool repeat) {
    async_iterator = mp_getiter(iterable);
    async_error = NULL;
    async_delay = delay;
    MP_STATE_PORT(async_data)[0] = self; // so it doesn't get GC'd
    MP_STATE_PORT(async_data)[1] = async_iterator;
    if (repeat) {
        async_repeat_iterable = iterable;
        MP_STATE_PORT(async_data)[2] = iterable;
    }
    async_mode = ASYNC_MODE_ANIMATION;
}

STATIC void do_animation(microbit_display_obj_t *self, mp_obj_t iterable, mp_int_t delay, bool wait, bool loop) {
    // reset repeat state
    MP_STATE_PORT(async_data)[0] = NULL;
    MP_STATE_PORT(async_data)[1] = NULL;
    MP_STATE_PORT(async_data)[2] = NULL;
    if (wait) {
        do_synchronous_animation(self, iterable, delay, loop);
    } else {
        start_asynchronous_animation(self, iterable, delay, loop);
    }
}

STATIC mp_obj_t microbit_display_animate(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t animate_allowed_args[] = {
        { MP_QSTR_image,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_delay,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_stride,   MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5} },
        { MP_QSTR_start,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -5} },
        { MP_QSTR_wait,     MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_loop,     MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
    };

    // parse args
    microbit_display_obj_t *self = (microbit_display_obj_t*)pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(animate_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(animate_allowed_args), animate_allowed_args, args);
    
    mp_obj_t arg0 = args[0].u_obj;
    mp_int_t stride = args[2].u_int;
    mp_int_t start = args[3].u_int;
    mp_obj_t iterable;
    if (mp_obj_get_type(arg0) == &microbit_image_type) {
        iterable = microbit_image_slice((microbit_image_obj_t *)arg0, start, /*width*/5, stride);
    } else {
        iterable = arg0;
    }

    do_animation(self, iterable, args[1].u_int /*delay*/, args[4].u_bool /*wait? */, args[5].u_bool/*loop?*/);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(microbit_display_animate_obj, 2, microbit_display_animate);

mp_obj_t microbit_display_scroll(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t scroll_allowed_args[] = {
        { MP_QSTR_text,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_delay,    MP_ARG_INT, {.u_int = MICROBIT_DEFAULT_SCROLL_SPEED} },
        { MP_QSTR_wait,     MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_loop,     MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
    };

    // parse args
    microbit_display_obj_t *self = (microbit_display_obj_t*)pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(scroll_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(scroll_allowed_args), scroll_allowed_args, args);
    mp_obj_t iterable = scrolling_string_image_iterable(args[0].u_obj);
    do_animation(self, iterable, args[1].u_int /*delay*/, args[2].u_bool/*wait?*/, args[3].u_bool/*loop?*/);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(microbit_display_scroll_obj, 1, microbit_display_scroll);

mp_obj_t microbit_display_clear(void) {
    // reset repeat state, cancel animation and clear screen
    async_mode = ASYNC_MODE_CLEAR;
    async_tick = async_delay - FIBER_TICK_PERIOD_MS;
    wait_for_event();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_display_clear_obj, microbit_display_clear);

mp_obj_t microbit_display_set_pixel_raw(mp_uint_t n_args, const mp_obj_t *args) {
    (void)n_args;
    microbit_display_obj_t *self = (microbit_display_obj_t*)args[0];
    self->display->image.setPixelValue(mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_display_set_pixel_raw_obj, 4, 4, microbit_display_set_pixel_raw);

mp_obj_t microbit_display_set_pixel(mp_uint_t n_args, const mp_obj_t *args) {
    (void)n_args;
    microbit_display_obj_t *self = (microbit_display_obj_t*)args[0];
    mp_int_t bright = mp_obj_get_int(args[3]);
    if (bright < 0 || bright > MAX_BRIGHTNESS) 
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "brightness out of bounds."));
    self->display->image.setPixelValue(mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), BRIGHTNESS_SCALE[bright]);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_display_set_pixel_obj, 4, 4, microbit_display_set_pixel);

mp_obj_t microbit_display_get_pixel_raw(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
    microbit_display_obj_t *self = (microbit_display_obj_t*)self_in;
    mp_int_t raw = self->display->image.getPixelValue(mp_obj_get_int(x_in), mp_obj_get_int(y_in));
    return MP_OBJ_NEW_SMALL_INT(raw);
}
MP_DEFINE_CONST_FUN_OBJ_3(microbit_display_get_pixel_raw_obj, microbit_display_get_pixel_raw);

mp_obj_t microbit_display_get_pixel(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
    microbit_display_obj_t *self = (microbit_display_obj_t*)self_in;
    mp_int_t raw = self->display->image.getPixelValue(mp_obj_get_int(x_in), mp_obj_get_int(y_in));
    for (int i = 0; i < MAX_BRIGHTNESS; ++i) {
        mp_int_t bright = BRIGHTNESS_SCALE[i];
        mp_int_t next = BRIGHTNESS_SCALE[i+1];
        if (raw < (bright + next)/2)
            return MP_OBJ_NEW_SMALL_INT(i);
    }
    return MP_OBJ_NEW_SMALL_INT(MAX_BRIGHTNESS);
}
MP_DEFINE_CONST_FUN_OBJ_3(microbit_display_get_pixel_obj, microbit_display_get_pixel);

mp_obj_t microbit_display_print_buffer(mp_uint_t n_args, const mp_obj_t *args) {
    (void)n_args;
    microbit_display_obj_t *self = (microbit_display_obj_t*)args[0];
    mp_obj_t buffer_in = args[1];
    mp_int_t offset = mp_obj_get_int(args[2]);
    mp_int_t stride = mp_obj_get_int(args[3]);
    if (offset < 0 || stride < 0) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_IndexError, "offset and stride must not be negative."));
    }
    mp_buffer_info_t buffer;
    mp_get_buffer_raise(buffer_in, &buffer, MP_BUFFER_READ);
    const char *ptr = ((const char *)buffer.buf) + offset;
    MicroBitImage *display_image = &self->display->image;
    if (stride*4+5 > buffer.len) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_IndexError, "buffer read out of range"));
    }
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; ++x) {
            mp_int_t bright = ptr[x];
            bright = max(0, bright);
            bright = min(bright, MAX_BRIGHTNESS);
            display_image->setPixelValue(x, y, BRIGHTNESS_SCALE[bright]);
        }
        ptr += stride;
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_display_print_buffer_obj, 4, 4, microbit_display_print_buffer);

STATIC const mp_map_elem_t microbit_display_locals_dict_table[] = {
    
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_pixel),  (mp_obj_t)&microbit_display_get_pixel_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_pixel_raw),  (mp_obj_t)&microbit_display_get_pixel_raw_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_pixel),  (mp_obj_t)&microbit_display_set_pixel_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_pixel_raw),  (mp_obj_t)&microbit_display_set_pixel_raw_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_print), (mp_obj_t)&microbit_display_print_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_print_buffer), (mp_obj_t)&microbit_display_print_buffer_obj },
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

void microbit_display_init(void) {
    microbit_display_obj.display->setBrightness(255);
    microbit_display_obj.display->setDisplayMode(DISPLAY_MODE_GREYSCALE);
}

}
