/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Mark Shannon
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

#include "lib/ticker.h"

/*************************************
 * 40kHz (25µs cycle time) ticker.
 ************************************/

#define FastTicker NRF_TIMER0
#define FastTicker_IRQn TIMER0_IRQn
#define FastTicker_IRQHandler TIMER0_IRQHandler


#define SlowTicker_IRQn SWI3_IRQn
#define SlowTicker_IRQHandler SWI3_IRQHandler

static int32_t last_clock;

void ticker_init(void) {
    NRF_TIMER_Type *ticker = FastTicker;
    ticker->POWER = 1;
    __NOP();
    ticker_stop();
    ticker->TASKS_CLEAR = 1;
    last_clock = ticker->CC[3] = MICROSECONDS_PER_TICK;
    ticker->MODE = TIMER_MODE_MODE_Timer;
    ticker->BITMODE = TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos;
    ticker->PRESCALER = 4; // 1 tick == i microsecond
    ticker->INTENSET = TIMER_INTENSET_COMPARE3_Msk;
    ticker->SHORTS = 0;
    NVIC_SetPriority(FastTicker_IRQn, 1);
    NVIC_SetPriority(SlowTicker_IRQn, 3);
    NVIC_EnableIRQ(SlowTicker_IRQn);
}

/* Start and stop timer 1 including workarounds for Anomaly 73 for Timer
* http://www.nordicsemi.com/eng/content/download/29490/494569/file/nRF51822-PAN%20v3.0.pdf
*/
void ticker_start(void) {
    NVIC_EnableIRQ(FastTicker_IRQn);
    *(uint32_t *)0x40008C0C = 1; //for Timer 0
    FastTicker->TASKS_START = 1;
}

void ticker_stop(void) {
    NVIC_DisableIRQ(FastTicker_IRQn);
    FastTicker->TASKS_STOP = 1;
    *(uint32_t *)0x40008C0C = 0; //for Timer 0
}

static inline uint32_t timer_clock(void) {
    FastTicker->TASKS_CAPTURE[2] = 1;
    uint32_t t = FastTicker->CC[2];
    return t;
}

int32_t silent(int32_t previous_value) {
    return previous_value;
}

int32_t noop(void) {
    return -1;
}

static sound_func_ptr sound = silent;
static ticker_callback_ptr display_callback = noop;
static ticker_callback_ptr other_callback = noop;
static int16_t microseconds = 125;
static int16_t sub_ticks = 0;
static int32_t display_ticks = -1;
static int32_t other_ticks = -1;

extern int32_t ticks;

void FastTicker_IRQHandler(void)
{
    static int32_t sound_data = 0;
    FastTicker->EVENTS_COMPARE[3] = 0;
    last_clock = FastTicker->CC[3] = last_clock+MICROSECONDS_PER_TICK;
    /* Do sound */
    sound_data = sound(sound_data);
    /* Now display */
    if (--display_ticks == 0) {
        display_ticks = display_callback();
    }
    /* Now the other registered callback (PWM?) */
    if (--other_ticks == 0) {
        other_ticks = other_callback();
    }
    microseconds -= MICROSECONDS_PER_TICK;
    if (microseconds < 0) {
        microseconds += 1000;
        ticks += 1;
    }
    sub_ticks += 1;
    if (sub_ticks == TICKS_PER_MACRO_TICK) {
        sub_ticks = 0;
        NVIC_SetPendingIRQ(SlowTicker_IRQn);
    }
}

extern void ticker(void);

void SlowTicker_IRQHandler(void)
{
    ticker();
}

void set_sound_callback(sound_func_ptr func) {
    sound = func;
}

void set_display_callback(ticker_callback_ptr func, int32_t initial_delay) {
    display_ticks = -1;
    display_callback = func;
    display_ticks = initial_delay;
}

void clear_display_callback(void) {
    set_display_callback(noop, -1);
}

void clear_other_callback(void) {
    set_other_callback(noop, -1);
}

void set_other_callback(ticker_callback_ptr func, int32_t initial_delay) {
    other_ticks = -1;
    other_callback = func;
    other_ticks = initial_delay;
}
