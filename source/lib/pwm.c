/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Mark Shannon
 * Copyright (c) 2017 Damien P. George
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

#include "stddef.h"
#include "lib/ticker.h"
#include "nrf_gpio.h"
#include "py/runtime.h"
#include "py/gc.h"

#define PWM_TICKER_INDEX 2

// The maximum number of pins that can have PWM at once, plus 1 for OFF_EVENT
#define PWM_MAX_PINS (20)

// Default period of 20ms
#define DEFAULT_PERIOD ((20*1000)/MICROSECONDS_PER_TICK)

typedef struct _pwm_event {
    uint16_t time : 11;
    uint16_t pin : 5;
} pwm_event;

typedef struct _pwm_events {
    uint8_t count;
    uint16_t period;
    uint32_t all_pins;
    pwm_event events[PWM_MAX_PINS];
} pwm_events;

static const pwm_events OFF_EVENTS = {
    .count = 1,
    .period = DEFAULT_PERIOD,
    .all_pins = 0,
    .events = {
        {
            .time = 1024,
            .pin = 31,
        }
    }
};

// Data in RAM to hold the PWM events, double buffered
static pwm_events pwm_events_buf0;
static pwm_events pwm_events_buf1;

// Pointers to active and pending event buffers
static const pwm_events *active_events;
static const pwm_events *pending_events;

// Index inte the events array of the active_events buffer
static uint8_t next_event;

void pwm_init(void) {
    active_events = &OFF_EVENTS;
    pending_events = NULL;
    next_event = 0;
}

static inline int32_t pwm_get_period_ticks(void) {
    const pwm_events *tmp = pending_events;
    if (tmp == NULL)
        tmp = active_events;
    return tmp->period;
}

#if 0
void pwm_dump_events(const pwm_events *events) {
    printf("Count %d, period %d, all pins %d\r\n", events->count, events->period, events->all_pins);
    for (uint32_t i = 0; i < events->count; i++) {
        const pwm_event *event = &events->events[i];
        printf("Event. pin: %d, duty cycle: %d\r\n", event->pin, event->time);
    }
}

void pwm_dump_state(void) {
    while(pending_events);
    pwm_dump_events(active_events);
}
#endif

static pwm_events *get_pending_for_update(void) {
    // Atomically take the pending events
    __disable_irq();
    const pwm_events *orig = pending_events;
    pending_events = NULL;
    __enable_irq();

    // If the pending events were not NULL then they can be now used for update
    if (orig != NULL) {
        return (pwm_events*)orig; // Will be one of buf0 or buf1, in RAM
    }

    // Pending events were NULL so make a copy of active ones and return them
    orig = active_events;
    pwm_events *events;
    if (orig == &pwm_events_buf0) {
        events = &pwm_events_buf1;
    } else {
        events = &pwm_events_buf0;
    }
    events->count = orig->count;
    events->period = orig->period;
    events->all_pins = orig->all_pins;
    for (uint32_t i = 0; i < orig->count; i++) {
        events->events[i] = orig->events[i];
    }
    return events;
}

static int find_pin_in_events(const pwm_events *events, uint32_t pin) {
    for (int i = 0; i < events->count; i++) {
        if (events->events[i].pin == pin)
            return i;
    }
    return -1;
}

static void sort_events(pwm_events *events) {
    // Insertion sort
    for (int32_t i = 1; i < events->count; i++) {
        pwm_event x = events->events[i];
        int32_t j;
        for (j = i - 1; j >= 0 && events->events[j].time > x.time; j--) {
            events->events[j+1] = events->events[j];
        }
        events->events[j+1] = x;
    }
}

int32_t pwm_callback(void) {
    int32_t tdiff;
    const pwm_events *events = active_events;
    const pwm_event *event = &events->events[next_event];
    int32_t tnow = (event->time*events->period)>>10;
    do {
        if (event->pin < 31) {
            // Event is to turn on a specific pin
            nrf_gpio_pin_set(event->pin);
            next_event++;
        } else {
            // Event is to turn off all pins and start period again
            nrf_gpio_pins_clear(events->all_pins);
            next_event = 0;
            tnow = 0;
            if (pending_events) {
                events = pending_events;
                active_events = events;
                pending_events = NULL;
            }
        }
        event = &events->events[next_event];
        tdiff = ((event->time*events->period)>>10) - tnow;
    } while (tdiff == 0);
    return tdiff;
}

void pwm_start(void) {
    set_ticker_callback(PWM_TICKER_INDEX, pwm_callback, 120);
}

void pwm_stop(void) {
    clear_ticker_callback(PWM_TICKER_INDEX);
}

static void pwm_set_period_ticks(int32_t ticks) {
    pwm_events *events = get_pending_for_update();
    events->period = ticks;
    pending_events = events;
}

int pwm_set_period_us(int32_t us) {
    if ((us < 256) ||
        (us > 1000000)) {
        return -1;
    }
    pwm_set_period_ticks(us/MICROSECONDS_PER_TICK);
    return 0;
}

int32_t pwm_get_period_us(void) {
    return pwm_get_period_ticks()*MICROSECONDS_PER_TICK;
}

void pwm_set_duty_cycle(int32_t pin, uint32_t value) {
    if (value >= (1<<10)) {
        value = (1<<10)-1;
    }
    uint32_t turn_on_time = 1024-value;
    pwm_events *events = get_pending_for_update();

    // If pin is not already in PWM list then set its GPIO as output
    if (((1 << pin) & events->all_pins) == 0) {
         nrf_gpio_cfg_output(pin);
    }

    int ev = find_pin_in_events(events, pin);
    if (ev < 0 && value == 0) {
        // Pin not in PWM mode and set to 0 duty, so nothing to do
        pending_events = events;
        return;
    } else if (ev < 0) {
        // Pin not in PWM mode, so add it to the list
        if (events->count >= PWM_MAX_PINS) {
            // No more room left for another PWM, so must ignore this pin!
            pending_events = events;
            return;
        }
        events->all_pins |= 1 << pin;
        events->events[events->count].time = turn_on_time;
        events->events[events->count].pin = pin;
        events->count += 1;
    } else if (value == 0) {
        // Pin in PWM mode and set to 0 duty, so remove it from the list
        events->count -= 1;
        events->all_pins &= ~(1 << pin);
        if (ev < events->count) {
            events->events[ev] = events->events[events->count];
        }
    } else {
        // Pin in PWM mode, change duty cycle
        events->events[ev].time = turn_on_time;
    }
    sort_events(events);
    pending_events = events;
}

void pwm_release(int32_t pin) {
    pwm_set_duty_cycle(pin, 0);
    const pwm_events *ev = active_events;
    int i = find_pin_in_events(ev, pin);
    if (i < 0)
        return;
    // If i >= 0 it means that `ev` is in RAM, so it safe to discard the const qualifier
    ((pwm_events *)ev)->events[i].pin = 31;
    nrf_gpio_pin_clear(pin);
}
