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

#define antigravity_output_str(s,l) MP_PLAT_PRINT_STRN(s,l)

STATIC void antigravity_output_char(char c) {
	MP_PLAT_PRINT_STRN((char*) &c, 1);
}

STATIC void antigravity_output_x_chars(char c, uint8_t reps) {
	while (reps--)
		antigravity_output_char(c);
}

STATIC void antigravity_output_x_spaces(uint8_t reps) {
	antigravity_output_x_chars(' ', reps);
}

STATIC void antigravity_output_6_spaces() {
	antigravity_output_x_spaces(6);
}

STATIC void antigravity_output_vbar_newline_spacer() {
	antigravity_output_str("|\n|", 3);
}

STATIC void antigravity_output_vbar_spacer() {
	antigravity_output_str("| |", 3);
}

STATIC void antigravity_output_vbar_spacer_and_3_spaces() {
	antigravity_output_vbar_spacer();
	antigravity_output_x_spaces(3);
}

STATIC void antigravity_output_x_hyphens(uint8_t reps) {
	antigravity_output_x_chars('-', reps);
}

STATIC void antigravity_output_plus_separators()
{
	antigravity_output_str("+ +", 3);
}

/*STATIC void antigravity_output_str_len_3(const char *str)
{
	antigravity_output_str(str, 3);
}*/

/* due to size issues, the strip has been reduced to a load of
 * function calls below.
 * here is the original ASCII comic strip (the original comic stip itself can
 * be found at http://xkcd.com/353/):
+-xkcd.com/353-----------------------------------------------------------------+
|                                                                              |
|                                                              \0/             |
|                                                            /   \             |
|                 You're flying!                        Python!  /|            |
|                     How?                                       \ \           |
|                     /                                                        |
|                   0                                                          |
|                  /|\                                                         |
|                   |                                                          |
|--------------____/_\_______________________________--------------------------|
|                                                                              |
|                                                                              |
+------------------------------------------------------------------------------+
+-----------------------+ +-------------------------+ +------------------------+
|                       | |    I dunno...       |   | |      I just typed      |
|         0             | |  Dynamic typing?   /    | |   import antigravity   |
|        _/\\_          | |    Whitespace?    /     | |                  |     |
|     /     \//         | |    /             /      | |     That's it?  /      |
|   I learned it last   | |   |      Come join us!  | |     /          /       |
|   night! Everything   | |   |       Programming   | |    /    ...I also      |
|     is so simple!     | |   |      is fun again!  | |   /      sampled       |
|            /          | |   |      It's a  whole  | |  |  everything in the  |
|           /           | |   |       new  world    | |  |  medicine cabinet   |
|     Hello world is    | |   |        up here!     | |  |   for comparison.   |
|         just          | |   0                     | |   0         |          |
|                       | |  /|\/                   | |  /|\    But I think    |
| print("Hello, world!")| |   |   \_ But how are    | |   |     this is the    |
|                       | |  / \     you flying?    | |  / \      Python.      |
+-----------------------+ +-------------------------+ +------------------------+
*/
STATIC void antigravity_output_antigravity_ascii() {
	// first line
	antigravity_output_str("+-xkcd.com/353", 14);
	antigravity_output_x_hyphens(65);
	antigravity_output_str("+\n|", 3);
	// second line
	antigravity_output_x_spaces(78);
	antigravity_output_vbar_newline_spacer();
	// third line
	antigravity_output_x_spaces(62);
	antigravity_output_str("\\0/", 3);
	antigravity_output_x_spaces(13);
	antigravity_output_vbar_newline_spacer();
	// fourth line
	antigravity_output_x_spaces(60);
	antigravity_output_str("/   \\", 5);
	antigravity_output_x_spaces(13);
	antigravity_output_vbar_newline_spacer();
	// fifth line
	antigravity_output_x_spaces(17);
	antigravity_output_str("You're flying!", 14);
	antigravity_output_x_spaces(24);
	antigravity_output_str("Python!  /|", 11);
	antigravity_output_x_spaces(12);
	antigravity_output_vbar_newline_spacer();
	// sixth line
	antigravity_output_x_spaces(21);
	antigravity_output_str("How?", 4);
	antigravity_output_x_spaces(39);
	antigravity_output_str("\\ \\, 3");
	antigravity_output_x_spaces(11);
	antigravity_output_vbar_newline_spacer();
	// seventh line
	antigravity_output_x_spaces(21);
	antigravity_output_char('/');
	antigravity_output_x_spaces(56);
	antigravity_output_vbar_newline_spacer();
	// eigth line
	antigravity_output_x_spaces(19);
	antigravity_output_char('0');
	antigravity_output_x_spaces(58);
	antigravity_output_vbar_newline_spacer();
	// ninth line
	antigravity_output_x_spaces(18);
	antigravity_output_str("/|\\", 3);
	antigravity_output_x_spaces(57);
	antigravity_output_vbar_newline_spacer();
	// tenth line
	antigravity_output_x_spaces(19);
	antigravity_output_char('|');
	antigravity_output_x_spaces(58);
	antigravity_output_vbar_newline_spacer();
	// eleventh line
	antigravity_output_x_hyphens(14);
	antigravity_output_str("____/_\\", 7);
	antigravity_output_x_chars('_', 31);
	antigravity_output_x_hyphens(26);
	antigravity_output_vbar_newline_spacer();
	// twelth line
	antigravity_output_x_spaces(78);
	antigravity_output_vbar_newline_spacer();
	// thirteenth line
	antigravity_output_x_spaces(78);
	antigravity_output_str("|\n+", 3);
	// fourteenth line
	antigravity_output_x_hyphens(78);
	antigravity_output_str("+\n+", 3);
	// fifteenth line
	antigravity_output_x_hyphens(23);
	antigravity_output_plus_separators();
	antigravity_output_x_hyphens(25);
	antigravity_output_plus_separators();
	antigravity_output_x_hyphens(24);
	antigravity_output_str("+\n|", 3);
	// sixteenth line
	antigravity_output_x_spaces(23);
	antigravity_output_vbar_spacer();
	antigravity_output_str("     I dunno...          | |      I just typed", 46);
	antigravity_output_6_spaces();
	antigravity_output_vbar_newline_spacer();
	// seventeenth line
	antigravity_output_x_spaces(9);
	antigravity_output_char('0');
	antigravity_output_x_spaces(13);
	antigravity_output_str("| |   Dynamic typing?  /    | |   import "
						   "antigravity   |\n|        _/\\\\_", 71);
	// eighteenth line
	antigravity_output_x_spaces(10);
	antigravity_output_vbar_spacer();
	antigravity_output_str("    Whitespace?    /     | |", 28);
	antigravity_output_x_spaces(18);
	// nineteenth line (and a bit of the previous one)
	antigravity_output_str("|     |\n|     /     \\//", 23);
	antigravity_output_x_spaces(9);
	antigravity_output_vbar_spacer();
	antigravity_output_str("    /", 5);
	antigravity_output_x_spaces(13);
	antigravity_output_str("/      | |     That's it?  /", 28);
	antigravity_output_6_spaces();
	antigravity_output_vbar_newline_spacer();
	// twentieth line
	antigravity_output_str("   I learned it last   ", 23);
	antigravity_output_vbar_spacer_and_3_spaces();
	antigravity_output_str("|      Come join us!  | |     /", 31);
	antigravity_output_x_spaces(10);
	antigravity_output_str("/       ", 8);
	antigravity_output_vbar_newline_spacer();
	// twenty-first line
	antigravity_output_str("   night! Everything   ", 23);
	antigravity_output_vbar_spacer_and_3_spaces();
	antigravity_output_str("|       Programming   | |    /    ...I also", 43);
	antigravity_output_6_spaces();
	antigravity_output_vbar_newline_spacer();
	// twenty-second line
	antigravity_output_str("     is so simple!     ", 23);
	antigravity_output_vbar_spacer_and_3_spaces();
	antigravity_output_str("|      is fun again!  ", 22);
	antigravity_output_vbar_spacer_and_3_spaces();
	antigravity_output_str("/      sampled       ", 22);
	antigravity_output_vbar_newline_spacer();
	// twenty-third line
	antigravity_output_x_spaces(12);
	antigravity_output_char('/');
	antigravity_output_x_spaces(10);
	antigravity_output_vbar_spacer_and_3_spaces();
	antigravity_output_str("|      It's a  whole  | |  |  everything in the  |\n|", 52);
	// twenty-fourth line
	antigravity_output_x_spaces(11);
	antigravity_output_char('/');
	antigravity_output_x_spaces(11);
	antigravity_output_vbar_spacer_and_3_spaces();
	// a mix-up of lines
	antigravity_output_str("|       new  world    | |  |  medicine cabinet   |\n|     "
						   "Hello world is    | |   |        up here!     | |  |   for comparison.   |\n|", 133);
	antigravity_output_x_spaces(10);
	antigravity_output_str("just         | |   0", 20);
	antigravity_output_x_spaces(21);
	antigravity_output_str("| |   0         |          |\n|", 30);
	antigravity_output_x_spaces(23);
	antigravity_output_str("| |  /|\\/", 9);
	antigravity_output_x_spaces(19);
	antigravity_output_str("| |  /|\\    But I think    |\n| print(\"Hello, world!\")"
						   "| |   |   \\_ But how are    | |   |     this is the    |\n|", 111);
	antigravity_output_x_spaces(23);
	antigravity_output_str("| |  / \\     you flying?    | |  / \\      Python.      |\n+", 58);
	antigravity_output_x_hyphens(23);
	antigravity_output_plus_separators();
	antigravity_output_x_hyphens(25);
	antigravity_output_plus_separators();
	antigravity_output_x_hyphens(24);
	antigravity_output_str("+\n", 2);
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
