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


#include "microbitmath.h"

void microbit_vector3_difference(const microbit_vector3_t *a, const microbit_vector3_t *b, microbit_vector3_t *r) {
    r->x = a->x - b->x;
    r->y = a->y - b->y;
    r->z = a->z - b->z;
}

/* Avoid using floating vector or division */
int32_t microbit_vector3_distance(const microbit_vector3_t *a, const microbit_vector3_t *b) {
    microbit_vector3_t r;
    microbit_vector3_difference(a, b, &r);
    return microbit_vector3_length(&r);
}

/* Avoid using floating point or division */
int32_t int_sqrt(uint32_t x) {
    uint32_t bit = 15;
    for(; bit; bit--) {
        if (1UL<<(bit<<1) <= x) {
            break;
        }
    }
    uint32_t estimate = 1<<bit;
    while (bit) {
        bit--;
        int bigger = estimate + (1<<bit);
        if (bigger*bigger <= x) {
            estimate = bigger;
        }
    }
    return estimate;
}

int32_t microbit_vector3_length(const microbit_vector3_t *v) {
    uint32_t len_sq = v->x*v->x+v->y*v->y+v->z*v->z;
    return int_sqrt(len_sq);
}

int32_t microbit_vector3_dot_product(const microbit_vector3_t *a, const microbit_vector3_t *b) {
    return a->x*b->x + a->y*b->y + a->z*b->z;
}

float microbit_vector3_cos_angle(const microbit_vector3_t *a, const microbit_vector3_t *b) {
    int dot = microbit_vector3_dot_product(a, b);
    int alen = microbit_vector3_length(a);
    int blen = microbit_vector3_length(b);
    float res = ((float)dot)/alen/blen;
    return res;
}
