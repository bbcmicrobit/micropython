/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Damien P. George, Joe Glancy
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
#include "MicroBit.h"

extern "C" {

#include "py/mphal.h"
#include "microbitimage.h"
#include "microbitdisplay.h"

#define GET_PIXEL(x, y) microbit_display_get_pixel(&microbit_display_obj, x, y)
#define SET_PIXEL(x, y, v) microbit_display_set_pixel(&microbit_display_obj, x, y, v)

STATIC void antigravity_output_antigravity_ascii();

void antigravity(uint8_t interval = 200 /* ms */) {
    // move all of the LEDs upwards (we can move them in other directions in the future)
	antigravity_output_antigravity_ascii();

    for (uint8_t iteration = 0; iteration < 5; iteration++) {
        mp_hal_delay_ms(interval);
        bool wait = false;
        for (uint8_t row = 1; row < 5 - iteration; row++) {
            for (uint8_t col = 0; col < 5; col++) {
                // move this bit down if possible
                uint8_t val = GET_PIXEL(col, row);
                if (val) {
                    // this is why the row for loop starts at one
                    if (!GET_PIXEL(col, row - 1)) {
                        SET_PIXEL(col, row, 0);
                        SET_PIXEL(col, row - 1, val);
                        wait = true;
                    }
                } // we don't care if the LED is off
            }
        }

        if (!wait) {
            continue;
        }
    }
}

#define antigravity_output_str(s) MP_PLAT_PRINT_STRN(s, sizeof(s)-1);

STATIC void antigravity_output_char(char c) {
	char str[1] = {c};
	MP_PLAT_PRINT_STRN((char*) &str, 1);
}

STATIC void antigravity_output_x_chars(char c, uint8_t reps) {
	while (reps--)
		antigravity_output_char(c);
}

STATIC void antigravity_output_x_spaces(uint8_t reps) {
	antigravity_output_x_chars(' ', reps);
}

STATIC void antigravity_output_x_hyphens(uint8_t reps) {
	antigravity_output_x_chars('-', reps);
}

STATIC void antigravity_output_vbar_newline_spacer() {
	antigravity_output_str("|\n|");
}

/* due to size issues, the strip has been reduced to a load of
 * function calls below.
 * here is the ASCII comic strip (the original comic stip itself can be found at
 * http://xkcd.com/353/):
 1+-xkcd.com/353---------------------------------------------------+
 2|                                                                |
 3|                                                    \0/         |
 4|                                                  /   \         |
 5|        You're flying!                  MicroPython!  /|        |
 6|            How?                                      \ \       |
 7|            /                                                   |
 8|          0                                                     |
 9|         /|\                                                    |
 0|          |                                                     |
 1|-----____/_\______________________________----------------------|
 2|                                                                |
 3+----------------------------------------------------------------+
*/
STATIC void antigravity_output_antigravity_ascii() {
	// first line
	antigravity_output_str("+-xkcd.com/353");
	antigravity_output_x_hyphens(51);
	antigravity_output_vbar_newline_spacer();
	// second line
	antigravity_output_x_spaces(64);
	antigravity_output_vbar_newline_spacer();
	// third line
	antigravity_output_x_spaces(52);
	antigravity_output_str("\\0/");
	antigravity_output_x_spaces(9);
	antigravity_output_vbar_newline_spacer();
	// fourth line
	antigravity_output_x_spaces(50);
	antigravity_output_str("/   \\");
	antigravity_output_x_spaces(9);
	antigravity_output_vbar_newline_spacer();
	// fifth line
	antigravity_output_str("        You're flying!");
	antigravity_output_x_spaces(18);
	antigravity_output_str("MicroPython!  /|        ");
	antigravity_output_vbar_newline_spacer();
	// sixth line
	antigravity_output_x_spaces(12);
	antigravity_output_str("How?");
	antigravity_output_x_spaces(38);
	antigravity_output_str("\\ \\       ");
	antigravity_output_vbar_newline_spacer();
	// seventh line
	antigravity_output_x_spaces(12);
	antigravity_output_char('/');
	antigravity_output_x_spaces(51);
	antigravity_output_vbar_newline_spacer();
	// eigth line
	antigravity_output_x_spaces(10);
	antigravity_output_char('0');
	antigravity_output_x_spaces(53);
	// ninth line
	antigravity_output_x_spaces(9);
	antigravity_output_str("/|\\");
	antigravity_output_x_spaces(52);
	antigravity_output_vbar_newline_spacer();
	// tenth line
	antigravity_output_x_spaces(10);
	antigravity_output_char('|');
	antigravity_output_x_spaces(53);
	antigravity_output_vbar_newline_spacer();
	// eleventh line
	antigravity_output_x_hyphens(5);
	antigravity_output_str("____/_\\");
	antigravity_output_x_chars('_', 30);
	antigravity_output_x_hyphens(22);
	antigravity_output_vbar_newline_spacer();
	// twelth line
	antigravity_output_x_spaces(64);
	antigravity_output_str("|\n+");
	// thirteenth line
	antigravity_output_x_hyphens(64);
	antigravity_output_str("+\n");
}

STATIC mp_obj_t antigravity__init__(void) {
    antigravity();

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(antigravity___init___obj, antigravity__init__);

STATIC const mp_map_elem_t antigravity_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_antigravity) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&antigravity___init___obj },
};

STATIC MP_DEFINE_CONST_DICT(antigravity_module_globals, antigravity_module_globals_table);

const mp_obj_module_t antigravity_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_antigravity,
    .globals = (mp_obj_dict_t*)&antigravity_module_globals,
};

}
