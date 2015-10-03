/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George
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
#include "microbitobj.h"

extern "C" {

#include "py/runtime.h"
#include "py/objstr.h"
#include "modmicrobit.h"

typedef struct _music_tune_obj_t {
    mp_obj_base_t base;

    uint16_t bpm;
    uint16_t ticks;
} music_tune_obj_t;


STATIC void play_note(music_tune_obj_t *self, const char *note_str, size_t note_len, MicroBitPin *pin, bool async) {
    pin->setAnalogValue(128);

    // [NOTE](#|b)(octave)(:length)
    // technically, c4 is middle c, so we'll go with that...
    // if we define A as 0 and G as 7, then we can use the following
    // array of us periods

    // these are the periods of note4 (the octave ascending from middle c) from A->B then C->G
    STATIC uint16_t periods_us[] = {2273, 2025, 3822, 3405, 3034, 2863, 2551};
    // A#, -, C#, D#, -, F#, G#
    STATIC uint16_t periods_sharps_us[] = {2145, 0, 3608, 3214, 0, 2703, 2408};

    // we'll represent the note as an integer (A=0, G=6)
    // TODO: validate the note
    uint8_t note_index = (note_str[0] & 0x1f) - 1;

    // TODO: the duration and bpm should be persistent between notes
    uint8_t duration = 4;
    // uint8_t bpm = 120;
    // uint8_t ticks = 4; // that means that 4 ticks is one beat.

    uint8_t ms_per_tick = (60000/self->bpm)/self->ticks;

    int8_t octave = 4;
    bool sharp = false;

    uint8_t current_position = 1;

    // parse sharp or flat
    if (current_position < note_len && (note_str[current_position] == '#' || note_str[current_position] == 'b')) {
        if (note_str[current_position] == 'b') {
            // make sure we handle wrapping round gracefully
            if (note_index == 0) {
                note_index = 6;
            } else {
                note_index--;
            }

            // handle the unusual edge case of Cb
            if (note_index == 1) {
                octave--;
            }
        }

        sharp = true;
        current_position++;
    }

    // parse the octave
    if (current_position < note_len && note_str[current_position] != ':') {
        // currently this will only work with a one digit number
        // use +=, since the sharp/flat code changes octave to compensate.
        octave += (note_str[current_position] & 0xf) - 4;
        current_position++;
    }

    // parse the duration
    if (current_position < note_len && note_str[current_position] == ':') {
        // I'll make this handle up to two digits for the time being.
        current_position++;

        if (current_position < note_len) {
            duration += note_str[current_position] & 0xf;

            current_position++;
            if (current_position < note_len) {
                duration *= 10;
                duration += note_str[current_position] & 0xf;
            }
        } else {
            // technically, this should be a syntax error, since this means
            // that no duration has been specified. For the time being,
            // we'll let you off :D
        }
    }
    // play the note!

    // make the octave relative to octave 4
    octave -= 4;

    // printf("index: %d\n", note_index);
    // printf("sharp?: %s\n", sharp ? "yes" : "no");
    // printf("octave: %d\n", octave);
    // printf("duration: %d\n", duration);

    // 18 is 'r' or 'R'
    if (note_index < 10) {
        if (sharp) {
            // printf("period: %d\n", periods_sharps_us[note_index] >> octave);
            if (octave >= 0) {
                // printf("period: %d\n", periods_sharps_us[note_index] >> octave);
                pin->setAnalogPeriodUs(periods_sharps_us[note_index] >> octave);
            }
            else {
                // printf("period: %d\n", periods_sharps_us[note_index] << -octave);
                pin->setAnalogPeriodUs(periods_sharps_us[note_index] << -octave);
            }
        } else {
            // printf("period: %d\n", periods_sharps_us[note_index] >> octave);
            if (octave >= 0) {
                // printf("period: %d\n", periods_us[note_index] >> octave);
                pin->setAnalogPeriodUs(periods_us[note_index] >> octave);
            }
            else {
                // printf("period: %d\n", periods_us[note_index] >> -octave);
                pin->setAnalogPeriodUs(periods_us[note_index] << -octave);
            }
        }
    } else {
        // printf("rest!\r\n");
        pin->setAnalogValue(0);
    }

    uBit.sleep(ms_per_tick * duration);

    if (note_index >= 10) {
        pin->setAnalogValue(128);
    }

    pin->setAnalogValue(0);
}

STATIC mp_obj_t music_note(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_pin,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_note,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_async,  MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
    };

    // extract self.
    music_tune_obj_t *self = (music_tune_obj_t *)pos_args[0];

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    MicroBitPin *pin = microbit_obj_get_pin(args[0].u_obj);

    mp_uint_t note_len;
    const char * note_str = mp_obj_str_get_data(args[1].u_obj, &note_len);

    play_note(self, note_str, note_len, pin, args[2].u_bool);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(music_note_obj, 2, music_note);

STATIC mp_obj_t music_tune(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_pin,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_notes,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_async,  MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
    };

    // extract self.
    music_tune_obj_t *self = (music_tune_obj_t *)pos_args[0];

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    MicroBitPin *pin = microbit_obj_get_pin(args[0].u_obj);

    mp_uint_t len;
    mp_obj_t *items;
    mp_obj_get_array(args[1].u_obj, &len, &items);

    for (mp_uint_t i = 0; i < len; i++) {
        if (items[i] != mp_const_none) {
            mp_uint_t note_len;
            const char * note_str = mp_obj_str_get_data(items[i], &note_len);
            play_note(self, note_str, note_len, pin, args[2].u_bool);
        } else {
            uBit.sleep((60000/self->bpm));
        }
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(music_tune_obj, 2, music_tune);

STATIC mp_obj_t music_pitch(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_pin,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_frequency,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_async,  MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(music_pitch_obj, 2, music_pitch);

STATIC mp_obj_t music_set_tempo(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_ticks, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bpm, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };

    // get the self value
    music_tune_obj_t *self = (music_tune_obj_t *)pos_args[0];

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (args[0].u_int != 0) {
        // set ticks
        self->ticks = args[0].u_int;
    }

    if (args[1].u_int != 0) {
        self->bpm = args[1].u_int;
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(music_set_tempo_obj, 0, music_set_tempo);

STATIC mp_obj_t music_get_tempo(mp_obj_t self_in) {
    music_tune_obj_t *self = (music_tune_obj_t *)self_in;

    mp_obj_t tempo_tuple[2];

    tempo_tuple[0] = mp_obj_new_int(self->bpm);
    tempo_tuple[1] = mp_obj_new_int(self->ticks);

    return mp_obj_new_tuple(2, tempo_tuple);
}
MP_DEFINE_CONST_FUN_OBJ_1(music_get_tempo_obj, music_get_tempo);

STATIC mp_obj_t music_stop(mp_obj_t self_in) {
    // TODO: implement me!
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(music_stop_obj, music_stop);

STATIC const mp_map_elem_t music_tune_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_note), (mp_obj_t)&music_note_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_tune), (mp_obj_t)&music_tune_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pitch), (mp_obj_t)&music_pitch_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t)&music_stop_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_tempo), (mp_obj_t)&music_set_tempo_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_tempo), (mp_obj_t)&music_get_tempo_obj },
};

STATIC MP_DEFINE_CONST_DICT(music_tune_locals_dict, music_tune_locals_dict_table);

STATIC const mp_obj_type_t music_tune_type = {
    { &mp_type_type },
    .name = MP_QSTR_Tune,
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
    .bases_tuple = MP_OBJ_NULL,
    /* .locals_dict = */ (mp_obj_t)&music_tune_locals_dict,
};

music_tune_obj_t music_tune_singleton_obj {
    {&music_tune_type},
    120,
    4
};

}
