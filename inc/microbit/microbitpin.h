/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
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
#ifndef __MICROPY_INCLUDED_MICROBIT_MICROBITPIN_H__
#define __MICROPY_INCLUDED_MICROBIT_MICROBITPIN_H__

#include "py/obj.h"

typedef struct _microbit_pin_obj_t {
    mp_obj_base_t base;
    uint8_t number; // The pin number on microbit board
    uint8_t name; // The pin number in the GPIO port.
    uint8_t initial_mode;
} microbit_pin_obj_t;

typedef void(*release_func)(const microbit_pin_obj_t *pin);

typedef struct _pinmode {
    qstr name;
    release_func release; /* Call this function to release pin */
} microbit_pinmode_t;

extern const microbit_pinmode_t microbit_pinmodes[];

/* Leave 0 to mean default mode. */
#define MODE_UNUSED 1
#define MODE_READ_DIGITAL 2
#define MODE_WRITE_DIGITAL 3
#define MODE_DISPLAY 4
#define MODE_BUTTON 5
#define MODE_MUSIC 6
#define MODE_AUDIO_PLAY 7
#define MODE_TOUCH 8
#define MODE_I2C 9
#define MODE_SPI 10
#define MODE_WRITE_ANALOG 11

#define microbit_pin_mode_unused        (&microbit_pinmodes[MODE_UNUSED])
#define microbit_pin_mode_write_analog  (&microbit_pinmodes[MODE_WRITE_ANALOG])
#define microbit_pin_mode_read_digital  (&microbit_pinmodes[MODE_READ_DIGITAL])
#define microbit_pin_mode_write_digital (&microbit_pinmodes[MODE_WRITE_DIGITAL])
#define microbit_pin_mode_display       (&microbit_pinmodes[MODE_DISPLAY])
#define microbit_pin_mode_button        (&microbit_pinmodes[MODE_BUTTON])
#define microbit_pin_mode_music         (&microbit_pinmodes[MODE_MUSIC])
#define microbit_pin_mode_audio_play    (&microbit_pinmodes[MODE_AUDIO_PLAY])
#define microbit_pin_mode_touch         (&microbit_pinmodes[MODE_TOUCH])
#define microbit_pin_mode_i2c           (&microbit_pinmodes[MODE_I2C])
#define microbit_pin_mode_spi           (&microbit_pinmodes[MODE_SPI])

extern const microbit_pin_obj_t microbit_pins[];

#define microbit_p0_obj microbit_pins[0]
#define microbit_p1_obj microbit_pins[1]
#define microbit_p2_obj microbit_pins[2]
#define microbit_p3_obj microbit_pins[3]
#define microbit_p4_obj microbit_pins[4]
#define microbit_p5_obj microbit_pins[5]
#define microbit_p6_obj microbit_pins[6]
#define microbit_p7_obj microbit_pins[7]
#define microbit_p8_obj microbit_pins[8]
#define microbit_p9_obj microbit_pins[9]
#define microbit_p10_obj microbit_pins[10]
#define microbit_p11_obj microbit_pins[11]
#define microbit_p12_obj microbit_pins[12]
#define microbit_p13_obj microbit_pins[13]
#define microbit_p14_obj microbit_pins[14]
#define microbit_p15_obj microbit_pins[15]
#define microbit_p16_obj microbit_pins[16]
#define microbit_p19_obj microbit_pins[19]
#define microbit_p20_obj microbit_pins[20]

/** Can this pin be acquired? Safe to call in an interrupt. Not safe to call in an interrupt. */
void microbit_obj_pin_fail_if_cant_acquire(const microbit_pin_obj_t *pin);

/** Release pin for use by other modes. Safe to call in an interrupt.
 * If pin is NULL or pin already unused, then this is a no-op
 */
void microbit_obj_pin_free(const microbit_pin_obj_t *pin);

/** Acquire pin (causing analog/digital modes to release) for mode.
 * If pin is already in specified mode, this is a no-op.
 * Not safe to call in an interrupt as it may raise. */
void microbit_obj_pin_acquire(const microbit_pin_obj_t *pin, const microbit_pinmode_t *mode);

bool microbit_pin_high_debounced(microbit_pin_obj_t *pin);

const microbit_pinmode_t *microbit_pin_get_mode(const microbit_pin_obj_t *pin);

const microbit_pin_obj_t *microbit_pin_from_number(uint32_t number);

bool microbit_obj_pin_can_be_acquired(const microbit_pin_obj_t *pin);

void pinmode_error(const microbit_pin_obj_t *pin);

#endif // __MICROPY_INCLUDED_MICROBIT_MICROBITPIN_H__
