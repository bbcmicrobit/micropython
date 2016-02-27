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

#include "stddef.h"
#include "lib/ticker.h"
#include "nrf_gpio.h"
#include "py/runtime.h"
#include "py/gc.h"

#define PWM_TICKER_INDEX 2

typedef struct _pwm_event {
    uint32_t pins;
    struct _pwm_event *next;
    uint16_t timestamp;
} pwm_event;

// Default period of 20ms
#define DEFAULT_PERIOD ((20*1000)/MICROSECONDS_PER_TICK)

static pwm_event all_off = {
    .pins = 0,
    .next = &all_off,
    .timestamp = DEFAULT_PERIOD
};

static inline int32_t pwm_get_period_ticks(void) {
    return all_off.timestamp;
}

#if 0
void pwm_dump_state(void) {
    pwm_event *event = &all_off;
    do {
        printf("Event. pins: %d, timestamp: %d\n", event->pins, event->timestamp);
        event = event->next;
    } while (event != &all_off);
}
#endif

int32_t pwm_callback(void) {
    pwm_event *event = (pwm_event *)MP_STATE_PORT(pwm_next_event);
    if (event == &all_off) {
        nrf_gpio_pins_clear(event->pins);
    } else {
        nrf_gpio_pins_set(event->pins);
    }
    int32_t tnow = event->timestamp;
    MP_STATE_PORT(pwm_next_event) = event->next;
    int32_t tnext = event->next->timestamp;
    int32_t tdiff = tnext - tnow;
    if (tdiff <= 0) {
        tdiff += pwm_get_period_ticks();
    }
    return tdiff;
}

void pwm_start(void) {
    MP_STATE_PORT(pwm_next_event) = &all_off;
    set_ticker_callback(PWM_TICKER_INDEX, pwm_callback, 120);
}

void pwm_stop(void) {
    clear_ticker_callback(PWM_TICKER_INDEX);
}

/* Turn pin off and return relevant event if now unused */
static pwm_event *pwm_turn_off_pin(int32_t pin) {
    pwm_event *previous = &all_off;
    pwm_event *event = all_off.next;
    while (event != &all_off) {
        if (event->pins & (1UL << pin)) {
            goto found;
        }
        previous = event;
        event = event->next;
    }
    return NULL;
found:
    event->pins &= ~(1UL << pin);
    nrf_gpio_pin_clear(pin);
    all_off.pins  &= ~(1UL << pin);
    if (event->pins == 0) {
        previous->next = event->next;
        return event;
    }
    return NULL;
}

/* Turn pin on, using supplied event (if not NULL) */
static void pwm_insert_pin_on_event(pwm_event *event, int8_t pin, uint16_t timestamp) {
    nrf_gpio_cfg_output(pin);
    pwm_event *previous = &all_off;
    pwm_event *next = all_off.next;
    all_off.pins |= (1UL<<pin);
    while (next != &all_off) {
        if (next->timestamp >= timestamp) {
            break;
        }
        previous = next;
        next = next->next;
    }
    if (next->timestamp == timestamp) {
        // GC will clean up event when it is no longer reachable
        next->pins |= (1UL<<pin);
    } else {
        if (event == NULL) {
            event = gc_alloc(sizeof(pwm_event), false);
        }
        event->pins = (1UL<<pin);
        event->next = previous->next;
        event->timestamp = timestamp;
        previous->next = event;
    }
}

static int32_t pwm_set_period_ticks(int32_t ticks) {
    if (all_off.pins == 0) {
        all_off.timestamp = ticks;
        return 0;
    }
    pwm_stop();
    nrf_gpio_pins_clear(all_off.pins);
    int32_t previous = pwm_get_period_ticks();
    pwm_event *event = all_off.next;
    while (event != &all_off) {
        event->timestamp = event->timestamp * ticks / previous;
        event = event->next;
    }
    all_off.timestamp = ticks;
    pwm_start();
    return 0;
}

int32_t pwm_set_period_us(int32_t us) {
    if ((us < 1000) ||
        (us > 1000000)) {
        return -1;
    }
    return pwm_set_period_ticks(us/MICROSECONDS_PER_TICK);
}

int32_t pwm_get_period_us(void) {
    return pwm_get_period_ticks()*MICROSECONDS_PER_TICK;
}

int pwm_set_duty_cycle(int32_t pin, int32_t value) {
    if (value < 0 || value >= (1<<10)) {
        return -1;
    }
    pwm_event *event = pwm_turn_off_pin(pin);
    if (value == 0)
        return 0;
    int32_t period = pwm_get_period_ticks();
    int32_t ticks = ((value*2+1) * period) >> 11;
    if (ticks == 0)
        ticks = 1;
    if (ticks == period)
        ticks = period-1;
    pwm_insert_pin_on_event(event, pin, period-ticks);
    return 0;
}

