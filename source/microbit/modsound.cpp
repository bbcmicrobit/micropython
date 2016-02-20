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
#include "MicroBit.h"
#include "microbitmusic.h"

extern "C" {

#include "microbit/modmicrobit.h"
#include "gpio_api.h"
#include "device.h"
#include "nrf_gpio.h"

#include "lib/ticker.h"
#include "lib/sound.h"
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objstr.h"
#include "py/mphal.h"
#include "microbit/modsound.h"
#include "microbit/microbitobj.h"

#define TheTimer NRF_TIMER1

extern void gpiote_init(PinName pin, uint8_t channel_number);


/** Initialize the Programmable Peripheral Interconnect peripheral.
 * Mostly copied from pwmout_api.c
 */
static void ppi_init(uint8_t channel0, uint8_t channel1) {
    NRF_TIMER_Type *timer  = TheTimer;

    /* Attach CLOCK[0] and CLOCK[1] to channel0
     * CLOCK[2] and CLOCK[3] to channel1 */
    NRF_PPI->CH[0].EEP = (uint32_t)&timer->EVENTS_COMPARE[0];
    NRF_PPI->CH[0].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[channel0];
    NRF_PPI->CH[1].EEP = (uint32_t)&timer->EVENTS_COMPARE[1];
    NRF_PPI->CH[1].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[channel0];
    NRF_PPI->CH[2].EEP = (uint32_t)&timer->EVENTS_COMPARE[2];
    NRF_PPI->CH[2].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[channel1];
    NRF_PPI->CH[3].EEP = (uint32_t)&timer->EVENTS_COMPARE[3];
    NRF_PPI->CH[3].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[channel1];

    // Enable PPI channels.
    NRF_PPI->CHEN |= 15;
}

static inline void timer_stop(void) {
    TheTimer->TASKS_STOP = 1;
}

static inline void timer_start(void) {
    TheTimer->TASKS_START = 1;
    __NOP();
    __NOP();
    __NOP();
}

static int32_t previous_value = 0;
static int32_t delta = 0;
static bool running = false;
static bool sample;
static volatile bool fetcher_ready = true;

void sound_stop(void) {
    timer_stop();
    clear_ticker_callback(0);
    running = false;
    previous_value = 0;
    delta = 0;
}

static int32_t sound_ticker(void);

void sound_start(void) {
    timer_start();
    running = true;
    sample = false;
    fetcher_ready = true;
    set_ticker_callback(0, sound_ticker, 80);
}

volatile int32_t sound_buffer_read_index;

#define SOUND_BUFFER_MASK (SOUND_BUFFER_SIZE-1)

#define sound_buffer_ptr (MP_STATE_PORT(async_data)[2])
#define buffer_iter (MP_STATE_PORT(async_data)[3])

static void sound_data_fetcher(void) {
    if (buffer_iter == NULL)
        return;
    mp_obj_t buffer_obj = mp_iternext(buffer_iter);
    if (buffer_obj == MP_OBJ_STOP_ITERATION) {
        sound_stop();
        buffer_iter = NULL;
        return;
    }
    if (mp_obj_get_type(buffer_obj) != &microbit_sound_bytes_type) {
        sound_stop();
        buffer_iter = NULL;
        /* TO DO -- report error */
        return;
    }
    microbit_sound_bytes_obj_t *buffer = (microbit_sound_bytes_obj_t *)buffer_obj;
    const int32_t *data = (const int32_t*)buffer->data;
    int32_t write_half = ((sound_buffer_read_index>>LOG_SOUND_CHUNK_SIZE)+1)&1;
    int32_t *half_buffer = (int32_t*)(((int8_t *)sound_buffer_ptr) + (write_half<<LOG_SOUND_CHUNK_SIZE));
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

#define FIRST_PHASE_START 40
#define SECOND_PHASE_START (FIRST_PHASE_START+30*16)

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

static int32_t sound_ticker(void) {
    int32_t val1 = previous_value + delta;
    int32_t next_value = val1 + delta;
    previous_value = next_value;
    zero_pins();
    set_gpiote_output_pulses(val1>>1, next_value>>1);
    if (sample) {
        int32_t buffer_index = (int32_t)sound_buffer_read_index;
        buffer_index = (buffer_index+1)&SOUND_BUFFER_MASK;
        int32_t sample = ((int32_t)((int8_t *)sound_buffer_ptr)[buffer_index])*6;
        sound_buffer_read_index = buffer_index;
        delta = (sample-next_value)>>2;
        if ((buffer_index&(SOUND_CHUNK_SIZE-1)) == 0 && fetcher_ready) {
            fetcher_ready = false;
            set_low_priority_callback(sound_data_fetcher, SOUND_CALLBACK_ID);
        }
    }
    sample = !sample;
    /* Need to be triggered every 64µs. */
    return 64/MICROSECONDS_PER_TICK;
}

void sound_play_source(mp_obj_t src, bool wait) {
    buffer_iter = mp_getiter(src);
    sound_start();
    if (!wait) {
        return;
    }
    while(running) {
        if (MP_STATE_VM(mp_pending_exception) != MP_OBJ_NULL) {
            buffer_iter = NULL;
            sound_stop();
            return;
        }
        __WFI();
    }
}

static void init_pins(PinName p0, PinName p1) {
    sound_stop();
    pin0 = p0;
    pin1 = p1;
    gpiote_init(pin0, 0);
    gpiote_init(pin1, 1);
    ppi_init(0, 1);
}

mp_obj_t sound_set_pins(mp_obj_t pin0_obj, mp_obj_t pin1_obj) {
    PinName p0 = microbit_obj_get_pin(pin0_obj)->name;
    PinName p1 = microbit_obj_get_pin(pin1_obj)->name;
    if (p0 != pin0 || p1 != pin1) {
        init_pins(p0, p1);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(microbit_sound_set_pins_obj, sound_set_pins);

void sound_mute(void) {
    clear_ticker_callback(0);
    buffer_iter = NULL;
    memset(sound_buffer_ptr, 0, SOUND_BUFFER_SIZE);
}

mp_obj_t sound_reset(void) {
    init_pins(pin0, pin1);
    memset(sound_buffer_ptr, 0, SOUND_BUFFER_SIZE);
    sound_buffer_read_index = 0;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(microbit_sound_reset_obj, sound_reset);


mp_obj_t sound_init(void) {
    static bool initialised = false;
    if (!initialised) {
        buffer_iter = NULL;
        initialised = true;
        //Allocate buffer
        sound_buffer_ptr = (int8_t *)malloc(SOUND_BUFFER_SIZE);
        TheTimer->POWER = 1;
        NRF_TIMER_Type *timer = TheTimer;
        timer_stop();
        timer->TASKS_CLEAR = 1;
        timer->MODE = TIMER_MODE_MODE_Timer;
        timer->BITMODE = TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos;
        timer->PRESCALER = 0; //Full speed
        timer->SHORTS = 0;
    }
    return sound_reset();
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(sound___init___obj, sound_init);


STATIC mp_obj_t start() {
    sound_start();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(microbit_sound_start_obj, start);

STATIC mp_obj_t stop() {
    sound_stop();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(microbit_sound_stop_obj, stop);

STATIC mp_obj_t play_source(mp_obj_t src) {
    sound_play_source(src, false);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_sound_play_source_obj, play_source);


const mp_obj_type_t microbit_sound_bytes_type = {
    { &mp_type_type },
    .name = MP_QSTR_SoundBytes,
    .print = NULL,
    .make_new = NULL,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = NULL,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .stream_p = NULL,
    .bases_tuple = NULL,
    .locals_dict = NULL,
};

microbit_sound_bytes_obj_t *new_microbit_sound_bytes(void) {
    microbit_sound_bytes_obj_t *res = m_new_obj(microbit_sound_bytes_obj_t);
    res->base.type = &microbit_sound_bytes_type;
    return res;
}

STATIC const mp_map_elem_t globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_sound) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&sound___init___obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start), (mp_obj_t)&microbit_sound_start_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t)&microbit_sound_stop_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_play_source), (mp_obj_t)&microbit_sound_play_source_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_reset), (mp_obj_t)&microbit_sound_reset_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_pins), (mp_obj_t)&microbit_sound_set_pins_obj },
};

STATIC MP_DEFINE_CONST_DICT(module_globals, globals_table);

const mp_obj_module_t sound_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_sound,
    .globals = (mp_obj_dict_t*)&module_globals,
};


}


