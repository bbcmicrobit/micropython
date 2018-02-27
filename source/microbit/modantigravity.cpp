
/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George
 * Copyright (c) 2016 Joe Glancy
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

extern "C" {

#include "py/mphal.h"
#include "microbit/modmicrobit.h"

#define GET_PIXEL(x, y) microbit_display_get_pixel(&microbit_display_obj, x, y)
#define SET_PIXEL(x, y, v) microbit_display_set_pixel(&microbit_display_obj, x, y, v)

STATIC void antigravity_output_char(char c) {
    MP_PLAT_PRINT_STRN((char*) &c, 1);
}

// NOTE: string has to be plain ASCII
static void antigravity_print_rle(const char *s) {
    /* RLE encoding format (2 characters, [0] (first) and [1] (second)):
     * [0] the amount of times to output the specified character (max 127),
     *     bitwise or'ed with 0x80 (to set the last bit, ie: bit 7)
     * [1] the character to output
     */
    char reps;    // how many times to output the specified character
    while (*s != '\0') {
        if ((*s & 0x80) == 0x80) {
            // seventh bit set
            reps = *s ^ 0x80;
            s++;
            if (*s == '\0') {
                return;
            }
            while (reps--) {
                antigravity_output_char(*s);
            }
        }
        else {
            antigravity_output_char(*s);
        }
        s++;
    }
}

/* uncomment the following line if you want to build the antigravity module with
 * the full comic instead of the micro one
 */
//#define ANTIGRAVITY_COMIC_LARGE

void antigravity(uint8_t interval = 200 /* ms */) {
    /* move all of the LEDs upwards (we can move them in other directions in the
     * future).
     * first, output the traditional XKCD comic (either in full or micro:size :)
     */
    #ifndef ANTIGRAVITY_COMIC_LARGE
    /* micro comic (157 bytes):
    +-xkcd.com/353---------------------------------------------------+
    |                                                                |
    |                                                    \0/         |
    |                                                  /   \         |
    |        You're flying!                  MicroPython!  /|        |
    |            How?                                      \ \       |
    |            /                                                   |
    |          0                                                     |
    |         /|\                                                    |
    |          |                                                     |
    |-----____/_\______________________________----------------------|
    |                                                                |
    +----------------------------------------------------------------+
    */
    static const char *antigravity_comic_str =
    "+-xkcd.com/353\xb3-+\n"
    "|\xc0 |\n"
    "|\xb4 \\0/\x89 |\n"
    "|\xb2 /\x83 \\\x89 |\n"
    "|\x88 You're flying!\x92 MicroPython!  /|\x88 |\n"
    "|\x8c How?\xa6 \\ \\\x87 |\n"
    "|\x8c /\xb3 |\n"
    "|\x8a 0\xb5 |\n"
    "|\x89 /|\\\xb4 |\n"
    "|\x8a |\xb5 |\n"
    "|\x85-\x84_/_\\\x9e_\x96-|\n"
    "|\xc0 |\n"
    "+\xc0-+\n";
    #else
    /* full comic (922 bytes):
    +-xkcd.com/353-----------------------------------------------------------------+
    |                                                                              |
    |                                                              \0/             |
    |                                                            /   \             |
    |                 You're flying!                   MicroPython!  /|            |
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
    static const char *antigravity_comic_str =
    "+-xkcd.com/353\xc1-+\n"
    "|\xce |\n"
    "|\xbe \\0/\x8d |\n"
    "|\xbc /\x83 \\\x8d |\n"
    "|\x91 You're flying!\x93 MicroPython!  /|\x8c |\n"
    "|\x95 How?\xa7 \\ \\\x8b |\n"
    "|\x95 /\xb8 |\n"
    "|\x93 0\xba |\n"
    "|\x92 /|\\\xb9 |\n"
    "|\x93 |\xba |\n"
    "|\x8e-\x84_/_\\\x9f_\x9a-|\n"
    "|\xce |\n"
    "|\xce |\n"
    "+\xce-+\n"
    "+\x97-+ +\x99-+ +\x98-+\n"
    "|\x97 | |\x84 I dunno\x83.\x87 |\x83 | |\x86 I just typed\x86 |\n"
    "|\x89 0\x8d | |  Dynamic typing?\x83 /\x84 | |\x83 import antigravity\x83 |\n"
    "|\x88 _/\\\\_\x8a | |\x84 Whitespace?\x84 /\x85 | |\x92 |\x85 |\n"
    "|\x85 /\x85 \\//\x89 | |\x84 /\x8d /\x86 | |\x85 That's it?  /\x86 |\n"
    "|\x83 I learned it last\x83 | |\x83 |\x86 Come join us!  | |\x85 /\x8a /\x87 |\n"
    "|\x83 night! Everything\x83 | |\x83 |\x87 Programming\x83 | |\x84 /\x84 \x83.I also\x86 |\n"
    "|\x85 is so simple!\x85 | |\x83 |\x86 is fun again!  | |\x83 /\x86 sampled\x87 |\n"
    "|\x8c /\x8a | |\x83 |\x86 It's a  whole  | |  |  everything in the  |\n"
    "|\x8b /\x8b | |\x83 |\x87 new  world\x84 | |  |  medicine cabinet\x83 |\n"
    "|\x85 Hello world is\x84 | |\x83 |\x88 up here!\x85 | |  |\x83 for comparison.\x83 |\n"
    "|\x89 just\x8a | |\x83 0\x95 | |\x83 0\x89 |\x8a |\n"
    "|\x97 | |  /|\\/\x93 | |  /|\\\x84 But I think\x84 |\n"
    "| print(\"Hello, world!\")| |\x83 |\x83 \\_ But how are\x84 | |\x83 |\x85 this is the\x84 |\n"
    "|\x97 | |  / \\\x85 you flying?\x84 | |  / \\\x86 Python.\x86 |\n"
    "+\x97-+ +\x99-+ +\x98-+\n";
    #endif /* ANTIGRAVITY_COMIC_LARGE */

    antigravity_print_rle(antigravity_comic_str);

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
    .globals = (mp_obj_dict_t*)&antigravity_module_globals,
};

}
