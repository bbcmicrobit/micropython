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

void vector_diff(const vector *a, const vector *b, vector *r) {
    r->x = a->x - b->x;
    r->y = a->y - b->y;
    r->z = a->z - b->z;
}

/* Avoid using floating vector or division */
int32_t vector_distance(const vector *a, const vector *b) {
    vector r;
    vector_diff(a, b, &r);
    return vector_length(&r);
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

int32_t vector_length(const vector *v) {
    uint32_t len_sq = v->x*v->x+v->y*v->y+v->z*v->z;
    return int_sqrt(len_sq);
}

int32_t vector_dot_product(const vector *a, const vector *b) {
    return a->x*b->x + a->y*b->y + a->z*b->z;
}

float vector_cos_angle(const vector *a, const vector *b) {
    int dot = vector_dot_product(a, b);
    int alen = vector_length(a);
    int blen = vector_length(b);
    float res = ((float)dot)/alen/blen;
    return res;
}
