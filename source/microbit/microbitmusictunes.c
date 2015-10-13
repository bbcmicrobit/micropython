/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George
 * Copyright (c) 2015 Nicholas H. Tollervey
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

#include "py/objtuple.h"
#include "modmicrobit.h"

#define N(q) MP_OBJ_NEW_QSTR(MP_QSTR_ ## q)
#define T(name, ...) const mp_obj_tuple_t microbit_music_tune_ ## name ## _obj = {{&mp_type_tuple}, .len = (sizeof((mp_obj_t[]){__VA_ARGS__})/sizeof(mp_obj_t)), .items = {__VA_ARGS__}};

T(entertainer,
    N(d3_colon_1), N(d_hash_), N(e), N(c4_colon_2), N(e3_colon_1), N(c4_colon_2),
    N(e3_colon_1), N(c4_colon_3), N(c_colon_1), N(d), N(d_hash_), N(e),
    N(c), N(d), N(e_colon_2), N(b3_colon_1), N(d4_colon_2), N(c_colon_4));

T(nyan,
    N(f_hash_4_colon_2), N(g_hash_), N(c_hash__colon_1), N(d_hash__colon_2), N(b3_colon_1), N(d4_colon_1),
    N(c_hash_), N(b3_colon_2), N(b), N(c_hash_4), N(d), N(d_colon_1),
    N(c_hash_), N(b3_colon_1), N(c_hash_4_colon_1), N(d_hash_), N(f_hash_), N(g_hash_),
    N(d_hash_), N(f_hash_), N(c_hash_), N(d), N(b3), N(c_hash_4),
    N(b3), N(d_hash_4_colon_2), N(f_hash_), N(g_hash__colon_1), N(d_hash_), N(f_hash_),
    N(c_hash_), N(d_hash_), N(b3), N(d4), N(d_hash_), N(d),
    N(c_hash_), N(b3), N(c_hash_4), N(d_colon_2), N(b3_colon_1), N(c_hash_4),
    N(d_hash_), N(f_hash_), N(c_hash_), N(d), N(c_hash_), N(b3),
    N(c_hash_4_colon_2), N(b3), N(c_hash_4), N(b3), N(f_hash__colon_1), N(g_hash_),
    N(b_colon_2), N(f_hash__colon_1), N(g_hash_), N(b), N(c_hash_4), N(d_hash_),
    N(b3), N(e4), N(d_hash_), N(e), N(f_hash_), N(b3_colon_2),
    N(b), N(f_hash__colon_1), N(g_hash_), N(b), N(f_hash_), N(e4),
    N(d_hash_), N(c_hash_), N(b3), N(f_hash_), N(d_hash_), N(e),
    N(f_hash_), N(b3_colon_2), N(f_hash__colon_1), N(g_hash_), N(b_colon_2), N(f_hash__colon_1),
    N(g_hash_), N(b), N(b), N(c_hash_4), N(d_hash_), N(b3),
    N(f_hash_), N(g_hash_), N(f_hash_), N(b_colon_2), N(b_colon_1), N(a_hash_),
    N(b), N(f_hash_), N(g_hash_), N(b), N(e4), N(d_hash_),
    N(e), N(f_hash_), N(b3_colon_2), N(c_hash_4))

#undef N
#undef T
