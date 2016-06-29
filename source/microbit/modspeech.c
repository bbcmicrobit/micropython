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

static mp_obj_t say(mp_obj_t words) {
    audio_init();
    empty = new_microbit_audio_frame();
    buf = new_microbit_audio_frame();
    buf_start_pos = 0;
    const char *in = mp_obj_str_get_str(words);
    mp_obj_t src = make_speech_iter();
    /* We need to wait for reciter to do its job */
    rendering = false;
    exhausted = false;
    audio_play_source(src, mp_const_none, mp_const_none, false);
    char *args[2];
    args[0] = "SAM";
    args[1] = in;
    sam_main(2, args);
    last_frame = true;
    /* Wait for audio finish before returning */
    while (microbit_audio_is_playing());
    return mp_const_none;
}MP_DEFINE_CONST_FUN_OBJ_1(say_obj, say);

static const mp_map_elem_t _globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_speech) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_say), (mp_obj_t)&say_obj },
};

static MP_DEFINE_CONST_DICT(_globals, _globals_table);

const mp_obj_module_t speech_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_speech,
    .globals = (mp_obj_dict_t*)&_globals,
};
