/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
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

#include "spi_api.h"
#include "MicroBit.h"

extern "C" {

#include "py/runtime.h"
#include "modmicrobit.h"
#include "microbitobj.h"

typedef struct _microbit_spi_obj_t {
    mp_obj_base_t base;
    spi_t spi;
} microbit_spi_obj_t;

STATIC mp_obj_t microbit_spi_init(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 1000000} },
        { MP_QSTR_bits,     MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_mode,     MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_sclk,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none } },
        { MP_QSTR_mosi,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none } },
        { MP_QSTR_miso,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none } },
    };

    // parse args
    microbit_spi_obj_t *self = (microbit_spi_obj_t*)pos_args[0];
    struct {
        mp_arg_val_t baudrate, bits, mode, sclk, mosi, miso;
    } args;
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);

    // get pins
    PinName p_sclk = MICROBIT_PIN_P13;
    PinName p_mosi = MICROBIT_PIN_P15;
    PinName p_miso = MICROBIT_PIN_P14;
    if (args.sclk.u_obj != mp_const_none) {
        p_sclk = microbit_obj_get_pin(args.sclk.u_obj)->name;
    }
    if (args.mosi.u_obj != mp_const_none) {
        p_mosi = microbit_obj_get_pin(args.mosi.u_obj)->name;
    }
    if (args.miso.u_obj != mp_const_none) {
        p_miso = microbit_obj_get_pin(args.miso.u_obj)->name;
    }

    // initialise the SPI
    spi_init(&self->spi, p_mosi, p_miso, p_sclk, NC);
    spi_format(&self->spi, args.bits.u_int, args.mode.u_int, 0);
    spi_frequency(&self->spi, args.baudrate.u_int);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(microbit_spi_init_obj, 1, microbit_spi_init);

STATIC mp_obj_t microbit_spi_write(mp_obj_t self_in, mp_obj_t buf_in) {
    microbit_spi_obj_t *self = (microbit_spi_obj_t*)self_in;
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);
    const byte *buf = (const byte*)bufinfo.buf;
    for (uint i = 0; i < bufinfo.len; ++i, ++buf) {
        spi_master_write(&self->spi, *buf);
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_spi_write_obj, microbit_spi_write);

STATIC mp_obj_t microbit_spi_read(size_t n_args, const mp_obj_t *args) {
    microbit_spi_obj_t *self = (microbit_spi_obj_t*)args[0];
    vstr_t vstr;
    vstr_init_len(&vstr, mp_obj_get_int(args[1]));
    mp_int_t byte_out = 0;
    if (n_args == 3) {
        byte_out = mp_obj_get_int(args[2]);
    }
    for (uint i = 0; i < vstr.len; ++i) {
        vstr.buf[i] = spi_master_write(&self->spi, byte_out);
    }
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_spi_read_obj, 2, 3, microbit_spi_read);

STATIC mp_obj_t microbit_spi_write_readinto(mp_obj_t self_in, mp_obj_t write_buf, mp_obj_t read_buf) {
    microbit_spi_obj_t *self = (microbit_spi_obj_t*)self_in;
    mp_buffer_info_t write_bufinfo;
    mp_get_buffer_raise(write_buf, &write_bufinfo, MP_BUFFER_READ);
    const byte *wr_buf = (const byte*)write_bufinfo.buf;
    mp_buffer_info_t read_bufinfo;
    mp_get_buffer_raise(read_buf, &read_bufinfo, MP_BUFFER_WRITE);
    byte *rd_buf = (byte*)read_bufinfo.buf;
    for (uint i = 0; i < write_bufinfo.len; ++i, ++wr_buf, ++rd_buf) {
        *rd_buf = spi_master_write(&self->spi, *wr_buf);
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(microbit_spi_write_readinto_obj, microbit_spi_write_readinto);

STATIC const mp_map_elem_t microbit_spi_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&microbit_spi_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&microbit_spi_write_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&microbit_spi_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write_readinto), (mp_obj_t)&microbit_spi_write_readinto_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_spi_locals_dict, microbit_spi_locals_dict_table);

const mp_obj_type_t microbit_spi_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitSPI,
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
    .locals_dict = (mp_obj_dict_t*)&microbit_spi_locals_dict,
};

microbit_spi_obj_t microbit_spi_obj = {
    {&microbit_spi_type},
    .spi = {},
};

}
