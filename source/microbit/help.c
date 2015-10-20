/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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

#include <stdio.h>

#include "py/nlr.h"
#include "py/obj.h"
#include "microbit/modmicrobit.h"

STATIC const char *help_text =
"Welcome to MicroPython on the micro:bit!\n"
"\n"
"Type 'import microbit', press return and try these commands:\n"
"  microbit.display.scroll('Hello')\n"
"  microbit.running_time()\n"
"  microbit.sleep(1000)\n"
"  microbit.button_a.is_pressed()\n"
"What do these commands do? Can you improve them? HINT: use the up and down\n"
"arrow keys to get your command history. Press the TAB key to auto-complete\n"
"unfinished words (so 'mi' becomes 'microbit' after you press TAB). These\n"
"tricks save a lot of typing and look cool!\n"
"\n"
"Explore:\n"
"Type 'help(something)' to find out about it. Type 'dir(something)' to see what\n"
"it can do. For goodness sake, don't type 'import this'.\n"
"\n"
"Stuff to explore:\n"
"  microbit.accelerometer         -- detect the device's position (orientation)\n"
"  microbit.button_a.is_pressed() -- is button A pressed? (True or False)\n"
"  microbit.button_b.is_pressed() -- is button B pressed? (True or False)\n"
"  microbit.compass               -- detect the device's heading\n"
"  microbit.display               -- display things (pixels, characters, words)\n"
"  microbit.Image                 -- make pictures for the display\n"
"  microbit.pin0                  -- control the gold input/output (IO) pin0\n"
"  microbit.panic()               -- enter panic mode (requires a restart)\n"
"  microbit.random(n)             -- get a random number between 0 and n-1\n"
"  microbit.reset()               -- reset the device\n"
"  microbit.sleep(n)              -- wait for n milliseconds (1 second = 1000)\n"
"  microbit.running_time()        -- get the number of milliseconds since reset\n"
"\n"
"Control commands:\n"
"  CTRL-C        -- stop a running program\n"
"  CTRL-D        -- on a blank line, do a soft reset of the micro:bit\n"
"\n"
"Available modules: array, collections, microbit, micropython, gc, struct, sys,\n"
"this\n"
"\n"
"For more information about Python, visit: http://python.org/\n"
"To find out about MicroPython, visit: http://micropython.org/\n"
;

typedef struct _mp_doc_t {
    mp_const_obj_t obj;
    const char *doc;
} mp_doc_t;

STATIC const mp_doc_t help_table_types[] = {
    {&microbit_accelerometer_type, "MicroBitAccelerometer type\n"},
};

STATIC const mp_doc_t help_table_instances[] = {
    {&microbit_module, "A toolbox of code to control the micro:bit hardware.\n"},
    {&microbit_panic_obj, "panic() -- enter panic mode (requires a restart).\n"},
    {&microbit_random_obj, "random(n) -- return a random number between 0 and 'n'-1.\n"},
    {&microbit_sleep_obj, "sleep(n) -- wait for 'n' milliseconds (1 second = 1000).\n"},
    {&microbit_running_time_obj, "running_time() -- get the number of milliseconds since reset.\n"},
    {&microbit_accelerometer_obj, "A toolbox of code to detect the device's position (orientation).\n"},
    {&microbit_accelerometer_get_x_obj, "get_x() -- get the device's X axis reading.\n"},
    {&microbit_accelerometer_get_y_obj, "get_y() -- get the device's Y axis reading.\n"},
    {&microbit_accelerometer_get_z_obj, "get_z() -- get the device's Z axis reading.\n"},
    {&microbit_button_a_obj, "Represents button A. Use its is_pressed() method to discover its status.\n"},
    {&microbit_button_b_obj, "Represents button B. Use its is_pressed() method to discover its status.\n"},
    {&microbit_button_is_pressed_obj, "is_pressed() -- True if the button is pressed, else False.\n"},
    {&microbit_compass_obj, "A toolbox of code to detect the device's heading.\n"},
    {&microbit_compass_is_calibrated_obj, "is_calibrated() -- True if the compass is adjusted for accuracy, else False.\n"},
    {&microbit_compass_calibrate_obj, "calibrate() -- adjusts the compass for accuracy. Ensures reliable results.\n"},
    {&microbit_compass_is_calibrating_obj, "is_calibrating() -- True if the compass is adjusting itself for accuracy.\n"},
    {&microbit_compass_clear_calibration_obj, "clear_calibration() - re-sets the compass to an un-calibrated state.\n"},
    {&microbit_compass_get_x_obj, "get_x() -- get the device's X axis reading.\n"},
    {&microbit_compass_get_y_obj, "get_y() -- get the device's Y axis reading.\n"},
    {&microbit_compass_get_z_obj, "get_z() -- get the device's Z axis reading.\n"},
    {&microbit_display_obj, "A toolbox of code to work with the device's 5x5 display.\n"},
    {&microbit_display_print_obj, "print(s) -- print the string 's' to the display. E.g. print('bit').\nprint(s, i) -- print string 's', one character at a time with a delay of 'i'.\n"},
    {&microbit_display_scroll_obj, "scroll(s) -- scroll the string 's' from left to right on the display.\nscroll(s, i) -- scroll string 's' with delay 'i' between characters.\n"},
    {&microbit_display_clear_obj, "clear() -- clear the display.\n"},
    {&microbit_display_animate_obj, "animate(img, delay, stride, start=0, async=False, repeat=False) -- animate\n    image 'img' with 'delay' milliseconds and 'stride' pixels offset between\n    frames. Optional: 'start' offset from left hand side, 'async' to run in the\n    background, 'repeat' to loop the animation.\n"},
    {&microbit_display_get_pixel_obj, "get_brightness(x, y) -- gets the brightness of the display pixel (x,y).\n"},
    {&microbit_display_set_pixel_obj, "set_brightness(x, y, b) -- sets the brightness 'b' of the display pixel (x,y).\n"},
    {&microbit_p0_obj, "Represents pin 0 on the gold edge connector.\n"},
    {&microbit_p1_obj, "Represents pin 1 on the gold edge connector.\n"},
    {&microbit_p2_obj, "Represents pin 2 on the gold edge connector.\n"},
    {&microbit_p3_obj, "Represents pin 3 on the gold edge connector.\n"},
    {&microbit_p4_obj, "Represents pin 4 on the gold edge connector.\n"},
    {&microbit_p5_obj, "Represents pin 5 on the gold edge connector.\n"},
    {&microbit_p6_obj, "Represents pin 6 on the gold edge connector.\n"},
    {&microbit_p7_obj, "Represents pin 7 on the gold edge connector.\n"},
    {&microbit_p8_obj, "Represents pin 8 on the gold edge connector.\n"},
    {&microbit_p9_obj, "Represents pin 9 on the gold edge connector.\n"},
    {&microbit_p10_obj, "Represents pin 10 on the gold edge connector.\n"},
    {&microbit_p11_obj, "Represents pin 11 on the gold edge connector.\n"},
    {&microbit_p12_obj, "Represents pin 12 on the gold edge connector.\n"},
    {&microbit_p13_obj, "Represents pin 13 on the gold edge connector.\n"},
    {&microbit_p14_obj, "Represents pin 14 on the gold edge connector.\n"},
    {&microbit_p15_obj, "Represents pin 15 on the gold edge connector.\n"},
    {&microbit_p16_obj, "Represents pin 16 on the gold edge connector.\n"},
    {&microbit_p19_obj, "Represents pin 19 on the gold edge connector.\n"},
    {&microbit_p20_obj, "Represents pin 20 on the gold edge connector.\n"},
    {&microbit_pin_write_digital_obj, "write_digital(v) -- write a digital value v to the pin. Use 0 (lo) or 1 (hi).\n"},
    {&microbit_pin_read_digital_obj, "read_digital() -- read the current digital value from the pin: 0 (lo) or 1 (hi).\n"},
    {&microbit_pin_write_analog_obj, "write_analog(v) -- write an analog value v between 0..255 to the pin.\n"},
    {&microbit_pin_read_analog_obj, "read_analog() -- read the current analog value from the pin: 0 - 65535.\n"},
    {&microbit_pin_is_touched_obj, "is_touched() -- True if the pin is touched, otherwise False.\n"},
};

STATIC void pyb_help_print_info_about_object(mp_obj_t name_o, mp_obj_t value) {
    mp_printf(&mp_plat_print, "  ");
    mp_obj_print(name_o, PRINT_STR);
    mp_printf(&mp_plat_print, " -- ");
    mp_obj_print(value, PRINT_STR);
    mp_printf(&mp_plat_print, "\n");
}

STATIC mp_obj_t pyb_help(uint n_args, const mp_obj_t *args) {
    if (n_args == 0) {
        // print a general help message
        mp_printf(&mp_plat_print, "%s", help_text);

    } else {
        mp_obj_t args0 = args[0];
        mp_obj_type_t *args0_type = mp_obj_get_type(args0);
        if (args0_type->name == MP_QSTR_bound_method) {
            args0 = ((mp_obj_t*)args0)[1]; // extract method
            args0_type = mp_obj_get_type(args0);
        }

        // see if we have specific help info for this instance
        for (size_t i = 0; i < MP_ARRAY_SIZE(help_table_instances); i++) {
            if (args0 == help_table_instances[i].obj) {
                mp_print_str(&mp_plat_print, help_table_instances[i].doc);
                //if (args0_type == &mp_type_module) {
                //TODO here we can list the things inside the module
                //}
                return mp_const_none;
            }
        }

        // see if we have specific help info for this type
        for (size_t i = 0; i < MP_ARRAY_SIZE(help_table_types); i++) {
            if (args0 == help_table_types[i].obj || args0_type == help_table_types[i].obj) {
                mp_print_str(&mp_plat_print, help_table_types[i].doc);
                return mp_const_none;
            }
        }

        // don't have specific help info, try instead to print something sensible

        mp_printf(&mp_plat_print, "object ");
        mp_obj_print(args0, PRINT_STR);
        mp_printf(&mp_plat_print, " is of type %q\n", args0_type->name);

        mp_map_t *map = NULL;
        if (args0_type == &mp_type_module) {
            map = mp_obj_dict_get_map(mp_obj_module_get_globals(args0));
        } else {
            mp_obj_type_t *type;
            if (args0_type == &mp_type_type) {
                type = args0;
            } else {
                type = args0_type;
            }
            if (type->locals_dict != MP_OBJ_NULL && MP_OBJ_IS_TYPE(type->locals_dict, &mp_type_dict)) {
                map = mp_obj_dict_get_map(type->locals_dict);
            }
        }
        if (map != NULL) {
            for (uint i = 0; i < map->alloc; i++) {
                if (map->table[i].key != MP_OBJ_NULL) {
                    pyb_help_print_info_about_object(map->table[i].key, map->table[i].value);
                }
            }
        }
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_builtin_help_obj, 0, 1, pyb_help);
