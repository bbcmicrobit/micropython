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

#include "py/runtime.h"
#include "py/obj.h"
#include "py/stream.h"
#include "microbit/filesystem.h"

static mp_obj_t microbit_file_writable(mp_obj_t self) {
    return mp_obj_new_bool(((file_descriptor_obj *)self)->writable);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_file_writable_obj, microbit_file_writable);

MP_DEFINE_CONST_FUN_OBJ_1(microbit_file_name_obj, (mp_fun_1_t)microbit_file_name);

static mp_obj_t microbit_file_close_func(mp_obj_t self_in) {
    microbit_file_close((file_descriptor_obj *)self_in);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_file_close_obj, microbit_file_close_func);

STATIC mp_obj_t file___exit__(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    return microbit_file_close_func(args[0]);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(file___exit___obj, 4, 4, file___exit__);

static const mp_map_elem_t microbit_file_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_close), (mp_obj_t)&microbit_file_close_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_name), (mp_obj_t)&microbit_file_name_obj },
    { MP_ROM_QSTR(MP_QSTR___enter__), (mp_obj_t)&mp_identity_obj },
    { MP_ROM_QSTR(MP_QSTR___exit__), (mp_obj_t)&file___exit___obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_writable), (mp_obj_t)&microbit_file_writable_obj },
    /* Stream methods */
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&mp_stream_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_readinto), (mp_obj_t)&mp_stream_readinto_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_readline), (mp_obj_t)&mp_stream_unbuffered_readline_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&mp_stream_write_obj },
};
static MP_DEFINE_CONST_DICT(microbit_file_locals_dict, microbit_file_locals_dict_table);

STATIC const mp_stream_p_t bytesio_stream_p = {
    .read = microbit_file_read,
    .write = microbit_file_write,
};

const mp_obj_type_t microbit_bytesio_type = {
    { &mp_type_type },
    .name = MP_QSTR_BytesIO,
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
    .protocol = &bytesio_stream_p,
    .parent = NULL,
    .locals_dict = (mp_obj_dict_t*)&microbit_file_locals_dict,
};

STATIC const mp_stream_p_t textio_stream_p = {
    .read = microbit_file_read,
    .write = microbit_file_write,
    .is_text = true,
};

const mp_obj_type_t microbit_textio_type = {
    { &mp_type_type },
    .name = MP_QSTR_TextIO,
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
    .protocol = &textio_stream_p,
    .parent = NULL,
    .locals_dict = (mp_obj_dict_t*)&microbit_file_locals_dict,
};

static mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args) {
    /// -1 means default; 0 explicitly false; 1 explicitly true.
    int read = -1;
    int text = -1;
    if (n_args == 2) {
        mp_uint_t len;
        const char *mode = mp_obj_str_get_data(args[1], &len);
        for (mp_uint_t i = 0; i < len; i++) {
            if (mode[i] == 'r' || mode[i] == 'w') {
                if (read >= 0) {
                    goto mode_error;
                }
                read = (mode[i] == 'r');
            } else if (mode[i] == 'b' || mode[i] == 't') {
                if (text >= 0) {
                    goto mode_error;
                }
                text = (mode[i] == 't');
            } else {
                goto mode_error;
            }
        }
    }
    mp_uint_t name_len;
    const char *filename = mp_obj_str_get_data(args[0], &name_len);
    file_descriptor_obj *res = microbit_file_open(filename, name_len, read == 0, text == 0);
    if (res == NULL) {
        mp_raise_msg(&mp_type_OSError, "file not found");
    }
    return res;
mode_error:
    mp_raise_ValueError("illegal mode");
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_builtin_open_obj, 1, 2, mp_builtin_open);
