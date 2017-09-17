/*
 * This file is part of the MicroPython project, http://micropython.org/
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

#include <stdio.h>

extern "C" {

#include "py/obj.h"
#include "microbit/modmicrobit.h"

STATIC mp_obj_t this__init__(void) {
    STATIC const char *this_text =
"The Zen of MicroPython, by Nicholas H. Tollervey\n"
"\n"
"Code,\n"
"Hack it,\n"
"Less is more,\n"
"Keep it simple,\n"
"Small is beautiful,\n"
"\n"
"Be brave! Break things! Learn and have fun!\n"
"Express yourself with MicroPython.\n"
"\n"
"Happy hacking! :-)\n";
    mp_printf(&mp_plat_print, "%s", this_text);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(this___init___obj, this__init__);

STATIC mp_obj_t this_authors(void) {
    /*
    If you contribute code to this project, add your name here.
    */
    STATIC const char *authors_text =
"MicroPython on the micro:bit is brought to you by:\n"
"Damien P. George, Mark Shannon, Radomir Dopieralski, Matthew Else,\n"
"Carol Willing, Tom Viner, Alan Jackson, Nick Coghlan, Joseph Haig,\n"
"Alex Chan, Andrea Grandi, Paul Egan, Piotr Kasprzyk, Andrew Mulholland,\n"
"Matt Wheeler, Joe Glancy, Abbie Brooks and Nicholas H. Tollervey.\n";
    mp_printf(&mp_plat_print, "%s", authors_text);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(this_authors_obj, this_authors);

STATIC const mp_map_elem_t this_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_this) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&this___init___obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_authors), (mp_obj_t)&this_authors_obj },
};

STATIC MP_DEFINE_CONST_DICT(this_module_globals, this_module_globals_table);

const mp_obj_module_t this_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&this_module_globals,
};

}
