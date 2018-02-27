/*
 * This file is part of the MicroPython project, http://micropython.org/
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

#include "string.h"

extern "C" {

#include "gpio_api.h"
#include "device.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_delay.h"

#include "lib/ticker.h"
#include "py/runtime0.h"
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objstr.h"
#include "py/mphal.h"
#include "py/gc.h"
#include "microbit/modmicrobit.h"
#include "microbit/modaudio.h"

#define TheTimer NRF_TIMER2
#define TheTimer_IRQn TIMER2_IRQn
#define TheTimer_Anomaly73_Addr (NRF_TIMER2_BASE + 0xC0C)

#define DEBUG_AUDIO 0
#if DEBUG_AUDIO
#include <stdio.h>
#define DEBUG(s) printf s
#else
#define DEBUG(s) (void)0
#endif

static void disable_gpiote(uint8_t channel)
{
    nrf_gpiote_task_configure(channel, 31, NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
    DEBUG(("GPIOTE disable channel %d\r\n", channel));
    nrf_gpiote_te_default(channel);
}

static void audio_gpiote_init(uint32_t pin, uint8_t channel)
{
    DEBUG(("GPIOTE init. pin %d, channel %d\r\n", pin, channel));
    nrf_gpio_pin_clear(pin);
    nrf_gpio_cfg(pin,
                 NRF_GPIO_PIN_DIR_OUTPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_H0H1,
                 NRF_GPIO_PIN_NOSENSE);
    nrf_gpiote_task_configure(channel, pin, NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
    nrf_gpiote_task_enable(channel);
}

static void audio_ppi_disconnect(void) {
    NRF_PPI->CHEN &= ~30;
}

/** Initialize the Programmable Peripheral Interconnect peripheral.
 */
static void audio_ppi_init(uint8_t channel0, uint8_t channel1) {
    DEBUG(("PPI init. Channels %d, %d\r\n", channel0, channel1));
    NRF_TIMER_Type *timer  = TheTimer;
    audio_ppi_disconnect();

    /* Attach CLOCK[0] and CLOCK[1] to GPIOTE channel0
     * CLOCK[2] and CLOCK[3] to GPIOTE channel1
     * Use PPI channels 1-4 (I think 0 is used by twi) */
    NRF_PPI->CH[1].EEP = (uint32_t)&timer->EVENTS_COMPARE[0];
    NRF_PPI->CH[1].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[channel0];
    NRF_PPI->CH[2].EEP = (uint32_t)&timer->EVENTS_COMPARE[1];
    NRF_PPI->CH[2].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[channel0];
    NRF_PPI->CH[3].EEP = (uint32_t)&timer->EVENTS_COMPARE[2];
    NRF_PPI->CH[3].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[channel1];
    NRF_PPI->CH[4].EEP = (uint32_t)&timer->EVENTS_COMPARE[3];
    NRF_PPI->CH[4].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[channel1];

    // Enable PPI channels.
    NRF_PPI->CHEN |= 30;

}


/* Start and stop timer 1 including workarounds for Anomaly 73 for Timer
* http://www.nordicsemi.com/eng/content/download/29490/494569/file/nRF51822-PAN%20v3.0.pdf
*/
static inline void timer_stop(void) {
    TheTimer->TASKS_STOP = 1;
    *(uint32_t *)TheTimer_Anomaly73_Addr = 0;
    TheTimer->TASKS_CLEAR = 1;
    TheTimer->CC[0] = 0xfffc;
    TheTimer->CC[1] = 0xfffc;
    TheTimer->CC[2] = 0xfffc;
    TheTimer->CC[3] = 0xfffc;
}

static inline void timer_start(void) {
    *(uint32_t *)TheTimer_Anomaly73_Addr = 1;
    TheTimer->TASKS_START = 1;
}

static int32_t previous_value = 0;
static int32_t delta = 0;
static volatile bool running = false;
static bool sample = false;
static volatile bool fetcher_ready = true;
static bool double_pin = true;
static volatile int32_t audio_buffer_read_index;
static const microbit_pin_obj_t *pin0 = NULL;
static const microbit_pin_obj_t *pin1 = NULL;

#define audio_buffer_ptr MP_STATE_PORT(audio_buffer)
#define audio_source_iter MP_STATE_PORT(audio_source)

void audio_stop(void) {
    timer_stop();
    audio_source_iter = NULL;
    clear_ticker_callback(0);
    running = false;
    previous_value = 0;
    delta = 0;
    audio_ppi_disconnect();
    disable_gpiote(0);
    nrf_gpio_pin_write(pin0->name, 0);
    microbit_obj_pin_free(pin0);
    if (double_pin) {
        disable_gpiote(1);
        nrf_gpio_pin_write(pin1->name, 0);
        microbit_obj_pin_free(pin1);
    }
}

static int32_t audio_ticker(void);


#define AUDIO_BUFFER_MASK (AUDIO_BUFFER_SIZE-1)

static void init_pin(const microbit_pin_obj_t *p0) {
    microbit_obj_pin_acquire(p0, microbit_pin_mode_audio_play);
    pin0 = p0;
    nrf_gpio_pin_write(pin0->name, 0);
    audio_gpiote_init(pin0->name, 0);
    audio_ppi_init(0, 0);
    double_pin = false;
}

static void init_pins(const microbit_pin_obj_t *p0, const microbit_pin_obj_t *p1) {
    microbit_obj_pin_acquire(p1, microbit_pin_mode_audio_play);
    microbit_obj_pin_acquire(p0, microbit_pin_mode_audio_play);
    pin0 = p0;
    pin1 = p1;
    nrf_gpio_pin_write(pin0->name, 0);
    nrf_gpio_pin_write(pin1->name, 0);
    audio_gpiote_init(pin0->name, 0);
    audio_gpiote_init(pin1->name, 1);
    audio_ppi_init(0, 1);
    double_pin = true;
}



static void audio_data_fetcher(bool lock) {
    if (audio_source_iter == NULL) {
        audio_stop();
        return;
    }
    /* WARNING: We are executing in an interrupt handler.
     * If an exception is raised here then we must hand it to the VM. */
    mp_obj_t buffer_obj;
    if (lock)
        gc_lock();
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        buffer_obj = mp_iternext_allow_raise(audio_source_iter);
        nlr_pop();
        if (lock)
            gc_unlock();
    } else {
        if (lock)
            gc_unlock();
        if (!mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(((mp_obj_base_t*)nlr.ret_val)->type),
            MP_OBJ_FROM_PTR(&mp_type_StopIteration))) {
            // an exception other than StopIteration, so set it for the VM to raise later
            //If memory error, add appropriate message.
            mp_obj_exception_t *ex = (mp_obj_exception_t *)nlr.ret_val;
            if (mp_obj_get_type(nlr.ret_val) == &mp_type_MemoryError) {
                ex->args = (mp_obj_tuple_t *)mp_obj_new_tuple(1, NULL);
                ex->args->items[0] = mp_obj_new_str("Allocation in interrupt handler",
                                                    strlen("Allocation in interrupt handler"), false);
            }
            MP_STATE_VM(mp_pending_exception) = MP_OBJ_FROM_PTR(nlr.ret_val);
        }
        buffer_obj = MP_OBJ_STOP_ITERATION;
    }
    int32_t write_half = ((audio_buffer_read_index>>LOG_AUDIO_CHUNK_SIZE)+1)&1;
    int32_t *half_buffer = (int32_t*)(((uint8_t *)audio_buffer_ptr) + (write_half<<LOG_AUDIO_CHUNK_SIZE));
    if (buffer_obj == MP_OBJ_STOP_ITERATION) {
        audio_source_iter = NULL;
        memset(half_buffer, 0, AUDIO_CHUNK_SIZE);
        fetcher_ready = true;
        return;
    }
    if (mp_obj_get_type(buffer_obj) != &microbit_audio_frame_type) {
        audio_source_iter = NULL;
        MP_STATE_VM(mp_pending_exception) = mp_obj_new_exception_msg(&mp_type_TypeError, "not an AudioFrame");
        return;
    }
    microbit_audio_frame_obj_t *buffer = (microbit_audio_frame_obj_t *)buffer_obj;
    const int32_t *data = (const int32_t*)buffer->data;
    half_buffer[0] = data[0];
    half_buffer[1] = data[1];
    half_buffer[2] = data[2];
    half_buffer[3] = data[3];
    half_buffer[4] = data[4];
    half_buffer[5] = data[5];
    half_buffer[6] = data[6];
    half_buffer[7] = data[7];
    fetcher_ready = true;
    return;
}

static void audio_data_fetcher_no_gc(void) {
    audio_data_fetcher(true);
}

static void audio_data_fetcher_allow_gc(void) {
    audio_data_fetcher(false);
}

#define TICK_PER_SAMPLE (128/MICROSECONDS_PER_TICK)

#define FIRST_PHASE_START 4
#define SECOND_PHASE_START ((CYCLES_PER_TICK*TICK_PER_SAMPLE/4)+FIRST_PHASE_START)

static inline void set_gpiote_output_pulses(int32_t val1, int32_t val2) {
    NRF_TIMER_Type *timer = TheTimer;
    if (double_pin) {
        //Start with output zero; pins 00
        if (val1 < 0) {
            timer->CC[0] = FIRST_PHASE_START;
            // -ve 10
            timer->CC[2] = FIRST_PHASE_START-val1;
            // zero 11
        } else {
            timer->CC[2] = FIRST_PHASE_START;
            // +ve 01
            timer->CC[0] = FIRST_PHASE_START+val1;
            // zero 11
        }
        // Output zero; pins 11.
        if (val2 < 0) {
            timer->CC[3] = SECOND_PHASE_START;
            // -ve 10
            timer->CC[1] = SECOND_PHASE_START-val2;
            // zero 00
        } else {
            timer->CC[1] = SECOND_PHASE_START;
            // +ve 01
            timer->CC[3] = SECOND_PHASE_START+val2;
            // zero 00
        }
        //End with output zero; pins 00
    } else {
        timer->CC[0] = FIRST_PHASE_START;
        timer->CC[1] = FIRST_PHASE_START+val1;
        timer->CC[2] = SECOND_PHASE_START;
        timer->CC[3] = SECOND_PHASE_START+val2;
    }
    timer->TASKS_CLEAR = 1;
}

static int32_t audio_ticker(void) {
    int32_t val1 = previous_value + delta;
    int32_t next_value = val1 + delta;
    previous_value = next_value;
    set_gpiote_output_pulses(val1>>4, next_value>>4);
    if (sample) {
        int32_t buffer_index = (int32_t)audio_buffer_read_index;
        buffer_index = (buffer_index+1)&AUDIO_BUFFER_MASK;
        int32_t next_sample = (int32_t)((uint8_t *)audio_buffer_ptr)[buffer_index];
        if (double_pin) {
            // Convert 0 to 255 to -256 to +254
            next_sample = next_sample*2-256;
        }
       // Sample is set to 7/4 times the input to scale to output interval of 512 cycles.
        // Actually mutiplied by 28 to account for divide by 16 when generating pulses.
        next_sample = next_sample*28+8;
        audio_buffer_read_index = buffer_index;
        delta = (next_sample-next_value)>>2;
        if ((buffer_index&(AUDIO_CHUNK_SIZE-1)) == 0 && fetcher_ready) {
            fetcher_ready = false;
            set_low_priority_callback(audio_data_fetcher_no_gc, AUDIO_CALLBACK_ID);
        }
    }
    sample = !sample;
    /* Need to be triggered twice per sample. */
    return TICK_PER_SAMPLE/2;
}

static void audio_set_pins(mp_obj_t pin0_obj, mp_obj_t pin1_obj) {
    const microbit_pin_obj_t *p0 = microbit_obj_get_pin(pin0_obj);
    if (pin1_obj == mp_const_none) {
        init_pin(p0);
    } else {
        const microbit_pin_obj_t *p1 = microbit_obj_get_pin(pin1_obj);
        init_pins(p0, p1);
    }
}

static int32_t pin_read_digital(const microbit_pin_obj_t *pin) {
    nrf_gpio_cfg_input(pin->name, NRF_GPIO_PIN_NOPULL);
    // Allow 1µs to settle.
    nrf_delay_us(1);
    return nrf_gpio_pin_read(pin->name);
}

static const microbit_pin_obj_t *big_pins[3] = { &microbit_p0_obj, &microbit_p1_obj, &microbit_p2_obj };

static void audio_auto_set_pins(void) {
    // Test to see if two of the "big" pins are connected by some sort of resistor.
    uint32_t i, j, count;
    bool usable[3];
    if (microbit_obj_pin_can_be_acquired(&microbit_p0_obj)) {
        usable[0] = true;
        microbit_obj_pin_acquire(&microbit_p0_obj, microbit_pin_mode_unused);
    }
    if (microbit_obj_pin_can_be_acquired(&microbit_p1_obj)) {
        usable[1] = true;
        microbit_obj_pin_acquire(&microbit_p1_obj, microbit_pin_mode_unused);
    }
    if (microbit_obj_pin_can_be_acquired(&microbit_p2_obj)) {
        usable[2] = true;
        microbit_obj_pin_acquire(&microbit_p2_obj, microbit_pin_mode_unused);
    }
    for (i = 0; i < 2; i++) {
        if (!usable[i])
            continue;
        const microbit_pin_obj_t *pin1 = big_pins[i];
        nrf_gpio_cfg_output(pin1->name);
        for (j = i+1; j < 3; j++) {
            if (!usable[j])
                continue;
            const microbit_pin_obj_t *pin2 = big_pins[j];
            for (count = 0; count < 4; count++) {
                nrf_gpio_pin_set(pin1->name);
                if (pin_read_digital(pin2) != 1)
                    break;
                nrf_gpio_pin_clear(pin1->name);
                if (pin_read_digital(pin2) != 0)
                    break;
            }
            DEBUG(("Count: %lu\r\n", count));
            if (count == 4) {
                init_pins(pin1, pin2);
                return;
            }
        }
    }
    /* Set to default: single pin0 */
    audio_set_pins((mp_obj_t)&microbit_p0_obj, mp_const_none);
}

static void audio_init(void) {
    if (audio_buffer_ptr == NULL) {
        audio_source_iter = NULL;
        //Allocate buffer
        audio_buffer_ptr = m_new(uint8_t, AUDIO_BUFFER_SIZE);
    }
    //NRF_CLOCK->TASKS_HFCLKSTART = 1;
    NVIC_DisableIRQ(TheTimer_IRQn);
    TheTimer->POWER = 1;
    NRF_TIMER_Type *timer = TheTimer;
    timer_stop();
    timer->MODE = TIMER_MODE_MODE_Timer;
    timer->BITMODE = TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos;
    timer->PRESCALER = 0; //Full speed
    timer->INTENCLR = TIMER_INTENCLR_COMPARE0_Msk | TIMER_INTENCLR_COMPARE1_Msk |
                        TIMER_INTENCLR_COMPARE2_Msk | TIMER_INTENCLR_COMPARE3_Msk;
    timer->SHORTS = 0;
}

void audio_play_source(mp_obj_t src, mp_obj_t pin1, mp_obj_t pin2, bool wait) {
    if (running) {
        audio_stop();
    }
    audio_init();
    if (pin1 == mp_const_none) {
        if (pin2 == mp_const_none) {
            audio_auto_set_pins();
        } else {
            mp_raise_TypeError("cannot set return_pin without pin");
        }
    } else {
        audio_set_pins(pin1, pin2);
    }
    audio_source_iter = mp_getiter(src, NULL);
    sample = false;
    fetcher_ready = true;
    audio_buffer_read_index = AUDIO_BUFFER_SIZE-1;
    memset(audio_buffer_ptr, 128, AUDIO_BUFFER_SIZE);
    audio_data_fetcher_allow_gc();
    timer_start();
    running = true;
    set_ticker_callback(0, audio_ticker, 80);
    if (!wait) {
        return;
    }
    while(running) {
        if (MP_STATE_VM(mp_pending_exception) != MP_OBJ_NULL) {
            return;
        }
        __WFE();
    }
}

STATIC mp_obj_t stop() {
    audio_stop();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(microbit_audio_stop_obj, stop);

STATIC mp_obj_t play(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_source, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_wait,  MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_pin,   MP_ARG_OBJ, {.u_obj = mp_const_none } },
        { MP_QSTR_return_pin,   MP_ARG_OBJ, {.u_obj = mp_const_none } },
    };
    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    mp_obj_t src = args[0].u_obj;
    mp_obj_t pin1 = args[2].u_obj;
    mp_obj_t pin2 = args[3].u_obj;
    audio_play_source(src, pin1, pin2, args[1].u_bool);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(microbit_audio_play_obj, 0, play);

bool microbit_audio_is_playing(void) {
    return running;
}

mp_obj_t is_playing(void) {
    return mp_obj_new_bool(running);
}
MP_DEFINE_CONST_FUN_OBJ_0(microbit_audio_is_playing_obj, is_playing);


microbit_audio_frame_obj_t *new_microbit_audio_frame(void);

STATIC mp_obj_t microbit_audio_frame_new(const mp_obj_type_t *type_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    (void)type_in;
    (void)args;
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    return new_microbit_audio_frame();
}

STATIC mp_obj_t audio_frame_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value_in) {
    microbit_audio_frame_obj_t *self = (microbit_audio_frame_obj_t *)self_in;
    mp_int_t index = mp_obj_get_int(index_in);
    if (index < 0 || index >= AUDIO_CHUNK_SIZE) {
         mp_raise_ValueError("index out of bounds");
    }
    if (value_in == MP_OBJ_NULL) {
        // delete
        mp_raise_TypeError("cannot delete elements of AudioFrame");
    } else if (value_in == MP_OBJ_SENTINEL) {
        // load
        return MP_OBJ_NEW_SMALL_INT(self->data[index]);
    } else {
        mp_int_t value = mp_obj_get_int(value_in);
        if (value < 0 || value > 255) {
            mp_raise_ValueError("value out of range");
        }
        self->data[index] = value;
        return mp_const_none;
    }
}

static mp_obj_t audio_frame_unary_op(mp_uint_t op, mp_obj_t self_in) {
    (void)self_in;
    switch (op) {
        case MP_UNARY_OP_LEN: return MP_OBJ_NEW_SMALL_INT(32);
        default: return MP_OBJ_NULL; // op not supported
    }
}

static mp_int_t audio_frame_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, mp_uint_t flags) {
    (void)flags;
    microbit_audio_frame_obj_t *self = (microbit_audio_frame_obj_t *)self_in;
    bufinfo->buf = self->data;
    bufinfo->len = AUDIO_CHUNK_SIZE;
    bufinfo->typecode = 'b';
    return 0;
}

static void add_into(microbit_audio_frame_obj_t *self, microbit_audio_frame_obj_t *other, bool add) {
    int mult = add ? 1 : -1;
    for (int i = 0; i < AUDIO_CHUNK_SIZE; i++) {
        unsigned val = (int)self->data[i] + mult*(other->data[i]-128);
        // Clamp to 0-255
        if (val > 255) {
            val = (1-(val>>31))*255;
        }
        self->data[i] = val;
    }
}

static microbit_audio_frame_obj_t *copy(microbit_audio_frame_obj_t *self) {
    microbit_audio_frame_obj_t *result = new_microbit_audio_frame();
    for (int i = 0; i < AUDIO_CHUNK_SIZE; i++) {
        result->data[i] = self->data[i];
    }
    return result;
}

mp_obj_t copyfrom(mp_obj_t self_in, mp_obj_t other) {
    microbit_audio_frame_obj_t *self = (microbit_audio_frame_obj_t *)self_in;
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(other, &bufinfo, MP_BUFFER_READ);
    uint32_t len = bufinfo.len > AUDIO_CHUNK_SIZE ? AUDIO_CHUNK_SIZE : bufinfo.len;
    for (uint32_t i = 0; i < len; i++) {
        self->data[i] = ((uint8_t *)bufinfo.buf)[i];
    }
   return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(copyfrom_obj, copyfrom);

union _i2f {
    int32_t bits;
    float value;
};

/* Convert a small float to a fixed-point number */
int32_t float_to_fixed(float f, uint32_t scale) {
    union _i2f x;
    x.value = f;
    int32_t sign = 1-((x.bits>>30)&2);
    /* Subtract 127 from exponent for IEEE-754 and 23 for mantissa scaling */
    int32_t exponent = ((x.bits>>23)&255)-150;
    /* Mantissa scaled by 2**23, including implicit 1 */
    int32_t mantissa = (1<<23) | ((x.bits)&((1<<23)-1));
    int32_t shift = scale+exponent;
    int32_t result;
    if (shift > 0) {
        result = sign*(mantissa<<shift);
    } else if (shift < -31) {
        result = 0;
    } else {
        result = sign*(mantissa>>(-shift));
    }
    // printf("Float %f: %d %d %x (scale %d) => %d\n", f, sign, exponent, mantissa, scale, result);
    return result;
}

static void mult(microbit_audio_frame_obj_t *self, float f) {
    int scaled = float_to_fixed(f, 15);
    for (int i = 0; i < AUDIO_CHUNK_SIZE; i++) {
        unsigned val = ((((int)self->data[i]-128) * scaled) >> 15)+128;
        if (val > 255) {
            val = (1-(val>>31))*255;
        }
        self->data[i] = val;
    }
}

STATIC mp_obj_t audio_frame_binary_op(mp_uint_t op, mp_obj_t lhs_in, mp_obj_t rhs_in) {
    if (mp_obj_get_type(lhs_in) != &microbit_audio_frame_type) {
        return MP_OBJ_NULL; // op not supported
    }
    microbit_audio_frame_obj_t *lhs = (microbit_audio_frame_obj_t *)lhs_in;
    switch(op) {
    case MP_BINARY_OP_ADD:
    case MP_BINARY_OP_SUBTRACT:
        lhs = copy(lhs);
    case MP_BINARY_OP_INPLACE_ADD:
    case MP_BINARY_OP_INPLACE_SUBTRACT:
        if (mp_obj_get_type(rhs_in) != &microbit_audio_frame_type) {
            return MP_OBJ_NULL; // op not supported
        }
        add_into(lhs, (microbit_audio_frame_obj_t *)rhs_in, op==MP_BINARY_OP_ADD||op==MP_BINARY_OP_INPLACE_ADD);
        return lhs;
    case MP_BINARY_OP_MULTIPLY:
        lhs = copy(lhs);
    case MP_BINARY_OP_INPLACE_MULTIPLY:
        mult(lhs, mp_obj_get_float(rhs_in));
        return lhs;
    }
    return MP_OBJ_NULL; // op not supported
}

STATIC const mp_map_elem_t microbit_audio_frame_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_copyfrom), (mp_obj_t)&copyfrom_obj },
};
STATIC MP_DEFINE_CONST_DICT(microbit_audio_frame_locals_dict, microbit_audio_frame_locals_dict_table);


const mp_obj_type_t microbit_audio_frame_type = {
    { &mp_type_type },
    .name = MP_QSTR_AudioFrame,
    .print = NULL,
    .make_new = microbit_audio_frame_new,
    .call = NULL,
    .unary_op = audio_frame_unary_op,
    .binary_op = audio_frame_binary_op,
    .attr = NULL,
    .subscr = audio_frame_subscr,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = { .get_buffer = audio_frame_get_buffer },
    .protocol = NULL,
    .parent = NULL,
    .locals_dict = (mp_obj_dict_t*)&microbit_audio_frame_locals_dict,
};

microbit_audio_frame_obj_t *new_microbit_audio_frame(void) {
    microbit_audio_frame_obj_t *res = m_new_obj(microbit_audio_frame_obj_t);
    res->base.type = &microbit_audio_frame_type;
    memset(res->data, 128, AUDIO_CHUNK_SIZE);
    return res;
}

STATIC const mp_map_elem_t audio_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_audio) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t)&microbit_audio_stop_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_play), (mp_obj_t)&microbit_audio_play_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_playing), (mp_obj_t)&microbit_audio_is_playing_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AudioFrame), (mp_obj_t)&microbit_audio_frame_type },
};

STATIC MP_DEFINE_CONST_DICT(audio_module_globals, audio_globals_table);

const mp_obj_module_t audio_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&audio_module_globals,
};


}


