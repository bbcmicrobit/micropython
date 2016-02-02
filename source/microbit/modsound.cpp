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

#include "MicroBit.h"
#include "microbitmusic.h"

extern "C" {

#include "lib/sound.h"
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objstr.h"
#include "py/mphal.h"


mp_obj_t sound__init__() {
    sound_init();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(sound___init___obj, sound__init__);


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

STATIC mp_obj_t set_rate(mp_obj_t rate_in) {
    mp_int_t rate = mp_obj_get_int(rate_in);
    int err = sound_set_rate(rate);
    if (err) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "not a legal sample rate"));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_sound_set_rate_obj, set_rate);

STATIC mp_obj_t get_rate() {
    mp_int_t rate = sound_get_rate();
    return  MP_OBJ_NEW_SMALL_INT(rate);
}
MP_DEFINE_CONST_FUN_OBJ_0(microbit_sound_get_rate_obj, get_rate);

STATIC mp_obj_t play_source(mp_obj_t src) {
    sound_play_source(mp_getiter(src), false);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_sound_play_source_obj, play_source);


typedef struct _sound_source_t {
    mp_obj_base_t base;
    mp_obj_t buffer;
    mp_obj_t source;
    mp_buffer_info_t source_info;
    const int32_t *buf_data;
    uint32_t index;
} sound_source_t;

STATIC mp_obj_t sound_source_next(mp_obj_t o_in) {
    sound_source_t *iter = (sound_source_t *)o_in;
    const int32_t *src = ((const int32_t *)iter->source_info.buf) + iter->index;
    int32_t *dest = (int32_t *)iter->buf_data;
    if (iter->index + SOUND_CHUNK_SIZE/4 > (iter->source_info.len>>2)) {
        unsigned len = (iter->source_info.len>>2)-iter->index;
        if (len == 0) {
            return MP_OBJ_STOP_ITERATION;
        }
        unsigned int i;
        for (i = 0; i < len; i++) {
            dest[i] = src[i];
        }
        for (; i < SOUND_CHUNK_SIZE/4; i++) {
            dest[i] = 0;
        }
        iter->index += len;
    } else {
        for (unsigned int i = 0; i < SOUND_CHUNK_SIZE/4; i++) {
            dest[i] = src[i];
        }
        iter->index += SOUND_CHUNK_SIZE/4;
    }
    return iter->buffer;
}

const mp_obj_type_t sound_source_type = {
    { &mp_type_type },
    .name = MP_QSTR_Source,
    .print = NULL,
    .make_new = NULL,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = NULL,
    .subscr = NULL,
    .getiter = mp_identity,
    .iternext = sound_source_next,
    .buffer_p = {NULL},
    .stream_p = NULL,
    .bases_tuple = NULL,
    .locals_dict = NULL,
};

extern mp_obj_str_t microbit_sound_silence_obj;
extern mp_obj_str_t microbit_sound_microbit_sample_obj;

STATIC mp_obj_t from_buffer(mp_obj_t src) {
    mp_buffer_info_t buf_info;
    if (!mp_get_buffer(src, &buf_info, MP_BUFFER_READ)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError,
                  "source must be a buffer"));
    }
    sound_source_t *iter = m_new_obj(sound_source_t);
    iter->base.type = &sound_source_type;
    iter->source = src;
    iter->source_info = buf_info;
    iter->index = 0;
    iter->buffer = mp_obj_new_bytearray(SOUND_CHUNK_SIZE, (void *)microbit_sound_silence_obj.data);
    mp_get_buffer(iter->buffer, &buf_info, MP_BUFFER_WRITE);
    iter->buf_data = (int32_t *)buf_info.buf;
    return iter;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_sound_from_buffer_obj, from_buffer);

STATIC const mp_map_elem_t globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_sound) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&sound___init___obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_rate), (mp_obj_t)&microbit_sound_get_rate_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_rate), (mp_obj_t)&microbit_sound_set_rate_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start), (mp_obj_t)&microbit_sound_start_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t)&microbit_sound_stop_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_from_buffer), (mp_obj_t)&microbit_sound_from_buffer_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_play_source), (mp_obj_t)&microbit_sound_play_source_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_microbit), (mp_obj_t)&microbit_sound_microbit_sample_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR__13_333kHz), MP_OBJ_NEW_SMALL_INT(3) },
    { MP_OBJ_NEW_QSTR(MP_QSTR__10kHz), MP_OBJ_NEW_SMALL_INT(4) },
    { MP_OBJ_NEW_QSTR(MP_QSTR__8kHz), MP_OBJ_NEW_SMALL_INT(5) },
    { MP_OBJ_NEW_QSTR(MP_QSTR__6_667kHz), MP_OBJ_NEW_SMALL_INT(6) },
    { MP_OBJ_NEW_QSTR(MP_QSTR__5_714kHz), MP_OBJ_NEW_SMALL_INT(7) },
    { MP_OBJ_NEW_QSTR(MP_QSTR__5kHz), MP_OBJ_NEW_SMALL_INT(8) },
    { MP_OBJ_NEW_QSTR(MP_QSTR__4_444kHz), MP_OBJ_NEW_SMALL_INT(9) },
    { MP_OBJ_NEW_QSTR(MP_QSTR__4kHz), MP_OBJ_NEW_SMALL_INT(10) },
};

STATIC MP_DEFINE_CONST_DICT(module_globals, globals_table);

const mp_obj_module_t sound_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_sound,
    .globals = (mp_obj_dict_t*)&module_globals,
};


}


