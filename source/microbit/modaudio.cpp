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

#include "string.h"

extern "C" {

#include "microbit/modmicrobit.h"
#include "gpio_api.h"
#include "device.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"

#include "lib/ticker.h"
#include "py/runtime0.h"
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objstr.h"
#include "py/mphal.h"
#include "microbit/modaudio.h"
#include "microbit/microbitobj.h"

#define TheTimer NRF_TIMER1
#define TheTimer_IRQn TIMER1_IRQn

void audio_gpiote_init(PinName pin, uint8_t channel)
{
    nrf_gpio_cfg(pin,
                 NRF_GPIO_PIN_DIR_OUTPUT,
                 NRF_GPIO_PIN_INPUT_DISCONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_H0H1,
                 NRF_GPIO_PIN_NOSENSE);
    NRF_GPIO->OUTCLR = (1UL << pin);

    nrf_gpiote_task_configure(channel, pin, NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);

    nrf_gpiote_task_enable(channel);

}


/** Initialize the Programmable Peripheral Interconnect peripheral.
 */
static void audio_ppi_init(uint8_t channel0, uint8_t channel1) {
    NRF_TIMER_Type *timer  = TheTimer;

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
    *(uint32_t *)0x40009C0C = 0; //for Timer 1

}

static inline void timer_start(void) {
    *(uint32_t *)0x40009C0C = 1; //for Timer 1
    TheTimer->TASKS_START = 1;
}

static int32_t previous_value = 0;
static int32_t delta = 0;
static volatile bool running = false;
static bool sample = false;
static volatile bool fetcher_ready = true;

#define audio_buffer_ptr MP_STATE_PORT(audio_buffer)
#define audio_source_iter MP_STATE_PORT(audio_source)

void audio_stop(void) {
    timer_stop();
    audio_source_iter = NULL;
    clear_ticker_callback(0);
    running = false;
    previous_value = 0;
    delta = 0;
}

static int32_t audio_ticker(void);

volatile int32_t audio_buffer_read_index;

#define AUDIO_BUFFER_MASK (AUDIO_BUFFER_SIZE-1)


void audio_start(void) {
    timer_start();
    running = true;
    sample = false;
    fetcher_ready = true;
    audio_buffer_read_index = AUDIO_BUFFER_SIZE-1;
    memset(audio_buffer_ptr, 0, AUDIO_BUFFER_SIZE);
    set_ticker_callback(0, audio_ticker, 80);
}

static void audio_data_fetcher(void) {
    if (audio_source_iter == NULL) {
        audio_stop();
        return;
    }
    /* WARNING: We are executing in an interrupt handler.
     * If an exception is raised here then we must hand it to the VM. */
    mp_obj_t buffer_obj;
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        buffer_obj = mp_iternext_allow_raise(audio_source_iter);
        nlr_pop();
    } else {
        if (!mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(((mp_obj_base_t*)nlr.ret_val)->type),
            MP_OBJ_FROM_PTR(&mp_type_StopIteration))) {
            // an exception other than StopIteration, so set it for the VM to raise later
            MP_STATE_VM(mp_pending_exception) = MP_OBJ_FROM_PTR(nlr.ret_val);
        }
        buffer_obj = MP_OBJ_STOP_ITERATION;
    }
    int32_t write_half = ((audio_buffer_read_index>>LOG_AUDIO_CHUNK_SIZE)+1)&1;
    int32_t *half_buffer = (int32_t*)(((int8_t *)audio_buffer_ptr) + (write_half<<LOG_AUDIO_CHUNK_SIZE));
    if (buffer_obj == MP_OBJ_STOP_ITERATION) {
        audio_source_iter = NULL;
        memset(half_buffer, 0, AUDIO_CHUNK_SIZE);
        fetcher_ready = true;
        return;
    }
    if (mp_obj_get_type(buffer_obj) != &microbit_audio_frame_type) {
        audio_source_iter = NULL;
        MP_STATE_VM(mp_pending_exception) = mp_obj_new_exception_msg(&mp_type_TypeError, "not an AudioFrame");
        audio_stop();
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

static PinName pin0 = P0_3;
static PinName pin1 = P0_2;

static inline void zero_pins(void) {
    NRF_GPIOTE->TASKS_OUT[0] = nrf_gpio_pin_read(pin0);
    NRF_GPIOTE->TASKS_OUT[1] = nrf_gpio_pin_read(pin1);
}

#define TICK_PER_SAMPLE (128/MICROSECONDS_PER_TICK)

#define FIRST_PHASE_START 32
#define SECOND_PHASE_START ((CYCLES_PER_TICK*TICK_PER_SAMPLE/4)+32)

static inline void set_gpiote_output_pulses(int32_t val1, int32_t val2) {
    NRF_TIMER_Type *timer = TheTimer;
    timer->TASKS_CLEAR = 1;
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
}

static int32_t audio_ticker(void) {
    int32_t val1 = previous_value + delta;
    int32_t next_value = val1 + delta;
    previous_value = next_value;
    zero_pins();
    set_gpiote_output_pulses(val1>>4, next_value>>4);
    if (sample) {
        int32_t buffer_index = (int32_t)audio_buffer_read_index;
        buffer_index = (buffer_index+1)&AUDIO_BUFFER_MASK;
        // Sample is set to 3 1/2 times the input to scale to output interval of 512 cycles.
        // Actually mutiplied by 56 to account for divide by 16 when generating pulses.
        int32_t next_sample = ((int32_t)((int8_t *)audio_buffer_ptr)[buffer_index])*56+8;
        audio_buffer_read_index = buffer_index;
        delta = (next_sample-next_value)>>2;
        if ((buffer_index&(AUDIO_CHUNK_SIZE-1)) == 0 && fetcher_ready) {
            fetcher_ready = false;
            set_low_priority_callback(audio_data_fetcher, AUDIO_CALLBACK_ID);
        }
    }
    sample = !sample;
    /* Need to be triggered twice per sample. */
    return TICK_PER_SAMPLE/2;
}

void audio_play_source(mp_obj_t src, bool wait) {
    audio_source_iter = mp_getiter(src);
    audio_start();
    if (!wait) {
        return;
    }
    while(running) {
        if (MP_STATE_VM(mp_pending_exception) != MP_OBJ_NULL) {
            audio_stop();
            return;
        }
        __WFE();
    }
}

static void init_pins(PinName p0, PinName p1) {
    audio_stop();
    pin0 = p0;
    pin1 = p1;
    audio_gpiote_init(pin0, 0);
    audio_gpiote_init(pin1, 1);
    audio_ppi_init(0, 1);
}

void audio_set_pins(mp_obj_t pin0_obj, mp_obj_t pin1_obj) {
    PinName p0 = microbit_obj_get_pin_name(pin0_obj);
    PinName p1 = microbit_obj_get_pin_name(pin1_obj);
    if (p0 != pin0 || p1 != pin1) {
        init_pins(p0, p1);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(microbit_audio_set_pins_obj, audio_set_pins);

mp_obj_t audio_reset(void) {
    init_pins(pin0, pin1);
    memset(audio_buffer_ptr, 0, AUDIO_BUFFER_SIZE);
    audio_buffer_read_index = AUDIO_BUFFER_SIZE-1;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(microbit_audio_reset_obj, audio_reset);


mp_obj_t audio_init(void) {
    static bool initialised = false;
    if (!initialised) {
        audio_source_iter = NULL;
        initialised = true;
        //Allocate buffer
        audio_buffer_ptr = m_new(int8_t, AUDIO_BUFFER_SIZE);
        NVIC_DisableIRQ(TheTimer_IRQn);
        TheTimer->POWER = 1;
        NRF_TIMER_Type *timer = TheTimer;
        timer_stop();
        timer->TASKS_CLEAR = 1;
        timer->MODE = TIMER_MODE_MODE_Timer;
        timer->BITMODE = TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos;
        timer->PRESCALER = 0; //Full speed
        timer->INTENCLR = TIMER_INTENCLR_COMPARE0_Msk | TIMER_INTENCLR_COMPARE1_Msk |
                          TIMER_INTENCLR_COMPARE2_Msk | TIMER_INTENCLR_COMPARE3_Msk;
        timer->SHORTS = 0;
    }
    return audio_reset();
}
MP_DEFINE_CONST_FUN_OBJ_0(audio___init___obj, audio_init);


STATIC mp_obj_t start() {
    audio_start();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(microbit_audio_start_obj, start);

STATIC mp_obj_t stop() {
    audio_stop();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(microbit_audio_stop_obj, stop);

STATIC mp_obj_t play(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_source, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_wait,  MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_pins,   MP_ARG_OBJ, {.u_obj = mp_const_none } },
    };
    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    mp_obj_t src = args[0].u_obj;
    mp_obj_t pins = args[2].u_obj;
    if (pins == mp_const_none) {
        audio_set_pins((mp_obj_t)&microbit_p0_obj, (mp_obj_t)&microbit_p1_obj);
    } else {
        mp_obj_t *pin_array;
        mp_obj_get_array_fixed_n(pins, 2, &pin_array);
        audio_set_pins(pin_array[0], pin_array[1]);
    }
    audio_play_source(src, args[1].u_bool);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(microbit_audio_play_obj, 0, play);

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
         nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "index out of bounds"));
    }
    if (value_in == MP_OBJ_NULL) {
        // delete
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "Cannot delete elements of AudioFrame"));
    } else if (value_in == MP_OBJ_SENTINEL) {
        // load
        return MP_OBJ_NEW_SMALL_INT(self->data[index]);
    } else {
        mp_int_t value = mp_obj_get_int(value_in);
        if (value < -128 || value >= 128) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "value out of range"));
        }
        self->data[index] = value;
        return mp_const_none;
    }
}

STATIC mp_obj_t mp_obj_audio_frame_unary_op(mp_uint_t op, mp_obj_t self_in) {
    (void)self_in;
    switch (op) {
        case MP_UNARY_OP_LEN: return MP_OBJ_NEW_SMALL_INT(32);
        default: return MP_OBJ_NULL; // op not supported
    }
}

const mp_obj_type_t microbit_audio_frame_type = {
    { &mp_type_type },
    .name = MP_QSTR_AudioFrame,
    .print = NULL,
    .make_new = microbit_audio_frame_new,
    .call = NULL,
    .unary_op = mp_obj_audio_frame_unary_op,
    .binary_op = NULL,
    .attr = NULL,
    .subscr = audio_frame_subscr,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .stream_p = NULL,
    .bases_tuple = NULL,
    .locals_dict = NULL,
};

microbit_audio_frame_obj_t *new_microbit_audio_frame(void) {
    microbit_audio_frame_obj_t *res = m_new_obj(microbit_audio_frame_obj_t);
    res->base.type = &microbit_audio_frame_type;
    return res;
}

STATIC const mp_map_elem_t audio_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_audio) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&audio___init___obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t)&microbit_audio_stop_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_play), (mp_obj_t)&microbit_audio_play_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_reset), (mp_obj_t)&microbit_audio_reset_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AudioFrame), (mp_obj_t)&microbit_audio_frame_type },
};

STATIC MP_DEFINE_CONST_DICT(audio_module_globals, audio_globals_table);

const mp_obj_module_t audio_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_audio,
    .globals = (mp_obj_dict_t*)&audio_module_globals,
};


}


