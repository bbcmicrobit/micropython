/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Mark Shannon
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


#ifndef __MICROPY_INCLUDED_MICROBIT_MICROBITMATH_H__
#define __MICROPY_INCLUDED_MICROBIT_MICROBITMATH_H__

#include "py/runtime.h"


typedef struct _vector {
    int16_t x;
    int16_t y;
    int16_t z;
} vector;


/* integer square root without using floating point or division */
int32_t int_sqrt(uint32_t x);

int32_t vector_length(const vector *v);

/* Store a - b into r */
void vector_diff(const vector *a, const vector *b, vector *r);

/* |a-b| */
int32_t vector_distance(const vector *a, const vector *b);

int32_t vector_dot_product(const vector *a, const vector *b);

float vector_cos_angle(const vector *a, const vector *b);

#endif // __MICROPY_INCLUDED_MICROBIT_MICROBITMATH_H__
