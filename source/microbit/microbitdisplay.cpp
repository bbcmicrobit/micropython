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
#include "nrf_gpio.h"

extern "C" {

#include "py/runtime.h"
#include "modmicrobit.h"
#include "microbitimage.h"
#include "microbitdisplay.h"
#include "lib/iters.h"

void microbit_display_show(microbit_display_obj_t *display, microbit_image_obj_t *image) {
    mp_int_t w = min(image->width(), 5);
    mp_int_t h = min(image->height(), 5);
    mp_int_t x = 0;
    mp_int_t brightnesses = 0;
    for (; x < w; ++x) {
        mp_int_t y = 0;
        for (; y < h; ++y) {
            uint8_t pix = image->getPixelValue(x, y);
            display->image_buffer[x][y] = pix;
            brightnesses |= (1 << pix);
        }
        for (; y < 5; ++y) {
            display->image_buffer[x][y] = 0;
        }
    }
    for (; x < 5; ++x) {
        for (mp_int_t y = 0; y < 5; ++y) {
            display->image_buffer[x][y] = 0;
        }
    }
    display->brightnesses = brightnesses;
}

mp_obj_t microbit_display_show_func(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {

    // Cancel any animations.
    MP_STATE_PORT(async_data)[0] = NULL;
    MP_STATE_PORT(async_data)[1] = NULL;

    static const mp_arg_t show_allowed_args[] = {
        { MP_QSTR_image,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_delay,    MP_ARG_INT, {.u_int = MICROBIT_DEFAULT_PRINT_SPEED} },
        { MP_QSTR_clear,     MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_wait,     MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_loop,     MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
    };

    // Parse the args.
    microbit_display_obj_t *self = (microbit_display_obj_t*)pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(show_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(show_allowed_args), show_allowed_args, args);

    mp_obj_t image = args[0].u_obj;
    mp_int_t delay = args[1].u_int;
    bool clear = args[2].u_bool;
    bool wait = args[3].u_bool;
    bool loop = args[4].u_bool;

    if (MP_OBJ_IS_STR(image)) {
        // arg is a string object
        mp_uint_t len;
        const char *str = mp_obj_str_get_data(image, &len);
        if (len == 0) {
            // There are no chars; do nothing.
            return mp_const_none;
        } else if (len == 1) {
            if (!clear && !loop) {
                // A single char; convert to an image and print that.
                image = microbit_image_for_char(str[0]);
                goto single_image_immediate;
            }
        }
    } else if (mp_obj_get_type(image) == &microbit_image_type) {
        if (!clear && !loop) {
            goto single_image_immediate;
        }
        image = mp_obj_new_tuple(1, &image);
    }
    // iterable:
    if (args[4].u_bool) { /*loop*/
        image = microbit_repeat_iterator(image);
    }
    microbit_display_animate(self, image, delay, clear, wait);
    return mp_const_none;

single_image_immediate:
    microbit_display_show(self, (microbit_image_obj_t *)image);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(microbit_display_show_obj, 1, microbit_display_show_func);

static uint8_t async_mode;
static mp_obj_t async_iterator = NULL;
// Record if an error occurs in async animation. Unfortunately there is no way to report this.
static volatile bool wakeup_event = false;
static mp_uint_t async_delay = 1000;
static mp_uint_t async_tick = 0;
static bool async_clear = false;
static bool async_error = true;

STATIC void async_stop(void) {
    async_iterator = NULL;
    async_mode = ASYNC_MODE_STOPPED;
    async_tick = 0;
    async_delay = 1000;
    async_clear = false;
    MP_STATE_PORT(async_data)[0] = NULL;
    MP_STATE_PORT(async_data)[1] = NULL;
    wakeup_event = true;
}

STATIC void wait_for_event() {
    while (!wakeup_event) {
        // allow CTRL-C to stop the animation
        if (MP_STATE_VM(mp_pending_exception) != MP_OBJ_NULL) {
            async_stop();
            return;
        }
        __WFI();
    }
    wakeup_event = false;
}

struct DisplayPoint {
    uint8_t x;
    uint8_t y;
};

#define NO_CONN 0

static const DisplayPoint display_map[MICROBIT_DISPLAY_COLUMN_COUNT][MICROBIT_DISPLAY_ROW_COUNT] = {
    {{0,0}, {4,2}, {2,4}},
    {{2,0}, {0,2}, {4,4}},
    {{4,0}, {2,2}, {0,4}},
    {{4,3}, {1,0}, {0,1}},
    {{3,3}, {3,0}, {1,1}},
    {{2,3}, {3,4}, {2,1}},
    {{1,3}, {1,4}, {3,1}},
    {{0,3}, {NO_CONN,NO_CONN}, {4,1}},
    {{1,2}, {NO_CONN,NO_CONN}, {3,2}}
};

inline void microbit_display_obj_t::setPinsForRow(uint8_t brightness) {
    int column_strobe = 0;

    // Calculate the bitpattern to write.
    for (int i = 0; i < MICROBIT_DISPLAY_COLUMN_COUNT; i++) {
        if (row_brightness[i] >= brightness) {
            column_strobe |= (1 << i);
        }
    }

    // Wwrite the new bit pattern.
    // Set port 0 4-7 and retain lower 4 bits.
    nrf_gpio_port_write(NRF_GPIO_PORT_SELECT_PORT0, (~column_strobe<<4 & 0xF0) | (nrf_gpio_port_read(NRF_GPIO_PORT_SELECT_PORT0) & 0x0F));

    // Set port 1 8-12 for the current row.
    nrf_gpio_port_write(NRF_GPIO_PORT_SELECT_PORT1, strobe_mask | (~column_strobe>>4 & 0x1F));
}

void microbit_display_obj_t::advanceRow() {
    // First, clear the old row.

    // Clear the old bit pattern for this row.
    // Clear port 0 4-7 and retain lower 4 bits.
    nrf_gpio_port_write(NRF_GPIO_PORT_SELECT_PORT0, 0xF0 | (nrf_gpio_port_read(NRF_GPIO_PORT_SELECT_PORT0) & 0x0F));
    // Clear port 1 8-12 for the current row.
    nrf_gpio_port_write(NRF_GPIO_PORT_SELECT_PORT1, strobe_mask | 0x1F);

    // Move on to the next row.
    strobe_mask <<= 1;
    strobe_row++;

    // Reset the row counts and bit mask when we have hit the max.
    if (strobe_row == MICROBIT_DISPLAY_ROW_COUNT) {
        strobe_row = 0;
        strobe_mask = 0x20;
    }

    // Prepare row for rendering.
    for (int i = 0; i < MICROBIT_DISPLAY_COLUMN_COUNT; i++) {
        int x = display_map[i][strobe_row].x;
        int y = display_map[i][strobe_row].y;
        row_brightness[i] = microbit_display_obj.image_buffer[x][y];
    }
    // Turn on any pixels that are at max.
    setPinsForRow(MAX_BRIGHTNESS);
}

static const uint16_t render_timings[] =
// The timer precision is only about 32us, so these timing will be rounded.
// The scale is exponential, each step is approx x1.9 greater than the previous.
{   0, // Brightness, Duration (approx)
    35,   //    1,     35
    32,   //    2,     67
    61,   //    3,     128
    115,  //    4,     243
    219,  //    5,     462
    417,  //    6,     879
    791,  //    7,     1670
    1500, //    8,     3170
//  Always on   9,    ~6000
};


// Egregious hack to work around paranoia in the DAL API.
struct FakeMicroBitDisplay : public MicroBitComponent
{
    uint8_t width;
    uint8_t height;
    uint8_t brightness;
    uint8_t strobeRow;
    uint8_t strobeBitMsk;
    uint8_t rotation;
    uint8_t mode;
    uint8_t greyscaleBitMsk;
    uint8_t timingCount;
    Timeout renderTimer;
};

Timeout *renderTimer = &((FakeMicroBitDisplay*)(&(uBit.display)))->renderTimer;

void microbit_display_obj_t::renderRow() {
    mp_uint_t brightness = previous_brightness+1;
    setPinsForRow(brightness);
    if (brightness == MAX_BRIGHTNESS) {
        return;
    }
    previous_brightness = brightness;
    // Attach this function to the timer.
    renderTimer->attach_us(this, &microbit_display_obj_t::renderRow, render_timings[brightness]);
}


static void microbit_display_update(void) {
    async_tick += FIBER_TICK_PERIOD_MS;
    if (async_tick < async_delay) {
        return;
    }
    async_tick = 0;
    switch (async_mode) {
        case ASYNC_MODE_ANIMATION:
        {
            if (MP_STATE_PORT(async_data)[0] == NULL || MP_STATE_PORT(async_data)[1] == NULL) {
                async_stop();
                break;
            }
            microbit_display_obj_t *display = (microbit_display_obj_t*)MP_STATE_PORT(async_data)[0];
            /* WARNING: We are executing in an interrupt handler.
             * If an exception is raised here, then a reset is the only way to recover. */
            mp_obj_t obj = mp_iternext(async_iterator);
            if (obj == MP_OBJ_STOP_ITERATION) {
                if (async_clear) {
                    microbit_display_show(&microbit_display_obj, BLANK_IMAGE);
                    async_clear = false;
                } else {
                    async_stop();
                }
            } else if (mp_obj_get_type(obj) == &microbit_image_type) {
                microbit_display_show(display, (microbit_image_obj_t *)obj);
            } else if (MP_OBJ_IS_STR(obj)) {
                mp_uint_t len;
                const char *str = mp_obj_str_get_data(obj, &len);
                if (len == 1) {
                    microbit_display_show(display, microbit_image_for_char(str[0]));
                } else {
                    async_error = true;
                    async_stop();
                }
            } else {
                async_error = true;
                async_stop();
            }
            break;
        }
        case ASYNC_MODE_CLEAR:
            microbit_display_show(&microbit_display_obj, BLANK_IMAGE);
            async_stop();
            break;
    }
}

#define GREYSCALE_MASK ((1<<MAX_BRIGHTNESS)-2)

void microbit_display_tick(void) {

    microbit_display_obj.advanceRow();

    microbit_display_update();
    microbit_display_obj.previous_brightness = 0;
    if (microbit_display_obj.brightnesses & GREYSCALE_MASK) {
        microbit_display_obj.renderRow();
    }
}


void microbit_display_animate(microbit_display_obj_t *self, mp_obj_t iterable, mp_int_t delay, bool clear, bool wait) {
    // Reset the repeat state.
    MP_STATE_PORT(async_data)[0] = NULL;
    MP_STATE_PORT(async_data)[1] = NULL;
    async_iterator = mp_getiter(iterable);
    async_error = false;
    async_delay = delay;
    async_clear = clear;
    MP_STATE_PORT(async_data)[0] = self; // so it doesn't get GC'd
    MP_STATE_PORT(async_data)[1] = async_iterator;
    wakeup_event = false;
    async_mode = ASYNC_MODE_ANIMATION;
    if (wait) {
        wait_for_event();
    }
}

void microbit_display_scroll(microbit_display_obj_t *self, const char* str) {
    mp_obj_t iterable = scrolling_string_image_iterable(str, strlen(str), NULL, false);
    microbit_display_animate(self, iterable, MICROBIT_DEFAULT_SCROLL_SPEED, false, false);
}


mp_obj_t microbit_display_scroll_func(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t scroll_allowed_args[] = {
        { MP_QSTR_text, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_delay, MP_ARG_INT, {.u_int = MICROBIT_DEFAULT_SCROLL_SPEED} },
        { MP_QSTR_wait, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_monospace, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_loop, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
    };
    // Parse the args.
    microbit_display_obj_t *self = (microbit_display_obj_t*)pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(scroll_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(scroll_allowed_args), scroll_allowed_args, args);
    mp_uint_t len;
    const char* str = mp_obj_str_get_data(args[0].u_obj, &len);
    mp_obj_t iterable = scrolling_string_image_iterable(str, len, args[0].u_obj, args[3].u_bool /*monospace?*/);
    if (args[4].u_bool) { /*loop*/
        iterable = microbit_repeat_iterator(iterable);
    }
    microbit_display_animate(self, iterable, args[1].u_int /*delay*/, false/*clear*/, args[2].u_bool/*wait?*/);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(microbit_display_scroll_obj, 1, microbit_display_scroll_func);

void microbit_display_clear(void) {
    // Reset repeat state, cancel animation and clear screen.
    wakeup_event = false;
    async_mode = ASYNC_MODE_CLEAR;
    async_tick = async_delay - FIBER_TICK_PERIOD_MS;
    wait_for_event();
}

mp_obj_t microbit_display_clear_func(void) {
    microbit_display_clear();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_display_clear_obj, microbit_display_clear_func);

void microbit_display_set_pixel(microbit_display_obj_t *display, mp_int_t x, mp_int_t y, mp_int_t bright) {
    if (x < 0 || y < 0 || x > 4 || y > 4) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "index out of bounds."));
    }
    if (bright < 0 || bright > MAX_BRIGHTNESS) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "brightness out of bounds."));
    }
    display->image_buffer[x][y] = bright;
    display->brightnesses |= (1 << bright);
}

STATIC mp_obj_t microbit_display_set_pixel_func(mp_uint_t n_args, const mp_obj_t *args) {
    (void)n_args;
    microbit_display_obj_t *self = (microbit_display_obj_t*)args[0];
    microbit_display_set_pixel(self, mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_display_set_pixel_obj, 4, 4, microbit_display_set_pixel_func);

mp_int_t microbit_display_get_pixel(microbit_display_obj_t *display, mp_int_t x, mp_int_t y) {
    if (x < 0 || y < 0 || x > 4 || y > 4) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "index out of bounds."));
    }
    return display->image_buffer[x][y];
}

STATIC mp_obj_t microbit_display_get_pixel_func(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
    microbit_display_obj_t *self = (microbit_display_obj_t*)self_in;
    return MP_OBJ_NEW_SMALL_INT(microbit_display_get_pixel(self, mp_obj_get_int(x_in), mp_obj_get_int(y_in)));
}
MP_DEFINE_CONST_FUN_OBJ_3(microbit_display_get_pixel_obj, microbit_display_get_pixel_func);

STATIC const mp_map_elem_t microbit_display_locals_dict_table[] = {

    { MP_OBJ_NEW_QSTR(MP_QSTR_get_pixel),  (mp_obj_t)&microbit_display_get_pixel_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_pixel),  (mp_obj_t)&microbit_display_set_pixel_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_show), (mp_obj_t)&microbit_display_show_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_scroll), (mp_obj_t)&microbit_display_scroll_obj },
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
    .bases_tuple = NULL,
    .locals_dict = (mp_obj_dict_t*)&microbit_display_locals_dict,
};

microbit_display_obj_t microbit_display_obj = {
    {&microbit_display_type},
    { 0 },
    .row_brightness = { 0 },
    .previous_brightness = 0,
    .strobe_row = 0,
    .brightnesses = 0,
    .strobe_mask = 0x20
};

void microbit_display_init(void) {
    //  Set pins as output.
    nrf_gpio_range_cfg_output(MICROBIT_DISPLAY_COLUMN_START,MICROBIT_DISPLAY_COLUMN_START + MICROBIT_DISPLAY_COLUMN_COUNT + MICROBIT_DISPLAY_ROW_COUNT);

    uBit.display.disable();
}

}
