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
"  microbit.system_time()\n"
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
"  microbit.io                    -- control the gold input/output (IO) pins\n"
"  microbit.panic()               -- enter panic mode (requires a restart)\n"
"  microbit.random(n)             -- get a random number between 0 and n-1\n"
"  microbit.sleep(n)              -- wait for n milliseconds (1 second = 1000)\n"
"  microbit.system_time()         -- get the number of milliseconds since reset\n"
"\n"
"Control commands:\n"
"  CTRL-C        -- stop a running program\n"
"  CTRL-D        -- on a blank line, do a soft reset of the micro:bit\n"
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
    {&microbit_module, "A toolbox of code to control the micro:bit hardware\n"},
    {&microbit_panic_obj, "panic() -- enter panic mode (requires a restart)\n"},
    {&microbit_random_obj, "random(n) -- return a random number between 0 and n-1\n"},
    {&microbit_sleep_obj, "sleep(n) -- wait for n milliseconds (1 second = 1000)\n"},
    {&microbit_system_time_obj, "system_time() -- get the number of milliseconds since reset\n"},
    {&microbit_accelerometer_obj, "A toolbox of code to detect the device's position (orientation)\n"},
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
        // see if we have specific help info for this instance
        for (size_t i = 0; i < MP_ARRAY_SIZE(help_table_instances); i++) {
            if (args[0] == help_table_instances[i].obj) {
                mp_print_str(&mp_plat_print, help_table_instances[i].doc);
                //if (mp_obj_get_type(args[0]) == &mp_type_module) {
                //TODO here we can list the things inside the module
                //}
                return mp_const_none;
            }
        }

        // see if we have specific help info for this type
        for (size_t i = 0; i < MP_ARRAY_SIZE(help_table_types); i++) {
            if (args[0] == help_table_types[i].obj) {
                mp_print_str(&mp_plat_print, help_table_types[i].doc);
                return mp_const_none;
            }
        }

        // don't have specific help info, try instead to print something sensible

        mp_printf(&mp_plat_print, "object ");
        mp_obj_print(args[0], PRINT_STR);
        mp_printf(&mp_plat_print, " is of type %s\n", mp_obj_get_type_str(args[0]));

        mp_map_t *map = NULL;
        if (MP_OBJ_IS_TYPE(args[0], &mp_type_module)) {
            map = mp_obj_dict_get_map(mp_obj_module_get_globals(args[0]));
        } else {
            mp_obj_type_t *type;
            if (MP_OBJ_IS_TYPE(args[0], &mp_type_type)) {
                type = args[0];
            } else {
                type = mp_obj_get_type(args[0]);
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
