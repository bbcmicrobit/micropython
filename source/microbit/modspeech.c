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

#include "py/obj.h"
#include "filesystem.h"
#include "py/objtuple.h"
#include "py/objstr.h"
#include "microbit/modaudio.h"
#include "lib/sam/render.h"
#include "lib/sam/reciter.h"
#include "lib/sam/sam.h"

extern unsigned char speed;
extern unsigned char pitch;


extern int sam_main(int argc, char **argv);

/** Called by SAM to output byte `b` at `pos` */

static microbit_audio_frame_obj_t *buf;
static microbit_audio_frame_obj_t *empty;
static volatile unsigned int buf_start_pos = 0;
static volatile unsigned char previous = 128;
volatile bool rendering = false;
volatile bool last_frame = false;
volatile bool exhausted = false;

void SamOutputByte(unsigned int pos, unsigned char b) {
    //printf("%d, %d, %d\r\n", pos, SCALE_RATE(pos), b);
    unsigned int actual_pos = SCALE_RATE(pos);
    while ((actual_pos & (-32)) > buf_start_pos) {
        // We have filled buffer
        rendering = true;
        /* XXX - Busy wait */
    }
    unsigned int offset = actual_pos & 31;
    // write a little bit in advance
    unsigned int end = min(offset+4, 32);
    while (offset < end) {
        buf->data[offset] = b;
        offset++;
    }
    previous = b;
    return;
}

/** This iterator assumes that the speech renderer can generate samples
 * at least as fast as we can consume them */
static mp_obj_t next(mp_obj_t iter) {
    (void)iter;
    // May need to wait for reciter to do its job before renderer generate samples.
    if (!rendering) {
        return empty;
    }
    if (exhausted) {
        return MP_OBJ_STOP_ITERATION;
    }
    if (last_frame) {
        exhausted = true;
        last_frame = false;
    }
    buf_start_pos += 32;
    return buf;
}

const mp_obj_type_t speech_iterator_type = {
    { &mp_type_type },
    .name = MP_QSTR_iterator,
    .print = NULL,
    .make_new = NULL,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = NULL,
    .subscr = NULL,
    .getiter = mp_identity,
    .iternext = next,
    .buffer_p = {NULL},
    .stream_p = NULL,
    .bases_tuple = NULL,
    .locals_dict = NULL,
};


typedef struct _speech_iterator_t {
    mp_obj_base_t base;
} speech_iterator_t;

static mp_obj_t make_speech_iter(void) {
    speech_iterator_t *result = m_new_obj(speech_iterator_t);
    result->base.type = &speech_iterator_type;
    return result;
}

static mp_obj_t pronounce(mp_obj_t words) {
    unsigned char *input;
    mp_uint_t len, outlen;
    reciter_memory *mem = m_new(reciter_memory, 1);
    MP_STATE_PORT(speech_data) = mem;
    const char *txt = mp_obj_str_get_data(words, &len);
    if (len > 254) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "text too long."));
    }
    for (mp_uint_t i = 0; i < len; i++) {
        mem->input[i] = txt[i];
    }
    mem->input[len] = '[';
    if (!TextToPhonemes(mem)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "could not parse input."));
    }
    for (outlen = 0; outlen < 255; outlen++) {
        if (mem->input[outlen] == 155) {
            break;
        }
    }
    mp_obj_t res = mp_obj_new_str_of_type(&mp_type_str, mem->input, outlen);
    // Prevent input becoming invisible to GC due to tail-call optimisation.
    MP_STATE_PORT(speech_data) = NULL;
    return res;
}MP_DEFINE_CONST_FUN_OBJ_1(pronounce_obj, pronounce);

#define DEFAULT_SING     false
#define DEFAULT_PITCH    64
#define DEFAULT_SPEED    72
#define DEFAULT_MOUTH    128
#define DEFAULT_THROAT   128

static mp_obj_t say(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_sing,     MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = DEFAULT_SING} },
        { MP_QSTR_pitch,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = DEFAULT_PITCH} },
        { MP_QSTR_speed,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = DEFAULT_SPEED} },
        { MP_QSTR_mouth,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = DEFAULT_MOUTH} },
        { MP_QSTR_throat,   MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = DEFAULT_THROAT} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

     // set the current saved speech state
    bool sing = args[0].u_bool;
    pitch  = args[1].u_int;
    speed  = args[2].u_int;
    mp_int_t mouth  = args[3].u_int;
    mp_int_t throat = args[4].u_int;
    SetMouthThroat(mouth, throat);

    mp_uint_t len;
    const char *input = mp_obj_str_get_data(pos_args[0], &len);
    // prepare audio
    audio_init();
    empty = new_microbit_audio_frame();
    buf = new_microbit_audio_frame();
    buf_start_pos = 0;
    mp_obj_t src = make_speech_iter();
    /* We need to wait for reciter to do its job */
    rendering = false;
    exhausted = false;
    audio_play_source(src, mp_const_none, mp_const_none, false);


    SetInput(input, len);
    if (!SAMMain())
    {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "SAM error"));
    }

    last_frame = true;
    /* Wait for audio finish before returning */
    while (microbit_audio_is_playing());
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(say_obj, 1, say);

static const mp_map_elem_t _globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_speech) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_say), (mp_obj_t)&say_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pronounce), (mp_obj_t)&pronounce_obj },
};

static MP_DEFINE_CONST_DICT(_globals, _globals_table);

const mp_obj_module_t speech_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_speech,
    .globals = (mp_obj_dict_t*)&_globals,
};
