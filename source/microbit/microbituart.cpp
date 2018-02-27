/*
 * This file is part of the MicroPython project, http://micropython.org/
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

extern "C" {

#include <errno.h>
#include "pinmap.h"
#include "serial_api.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mphal.h"
#include "microbit/modmicrobit.h"

// There is only one UART peripheral and it's already used by stdio (and
// connected to USB serial).  So to access the UART we go through the
// mp_hal_stdio functions.  The init method can reconfigure the underlying
// UART peripheral to a different baudrate and/or pins.

typedef struct _microbit_uart_obj_t {
    mp_obj_base_t base;
} microbit_uart_obj_t;

// timeout (in ms) to wait between characters when reading
STATIC uint16_t microbit_uart_timeout_char = 0;

STATIC mp_obj_t microbit_uart_init(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 9600} },
        { MP_QSTR_bits,     MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_parity,   MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_stop,     MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_pins,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none } },
        { MP_QSTR_tx,       MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none } },
        { MP_QSTR_rx,       MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none } },
    };

    // parse args
    //microbit_uart_obj_t *self = (microbit_uart_obj_t*)pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    SerialParity parity;
    if (args[2].u_obj == mp_const_none) {
        parity = ParityNone;
    } else {
        parity = (SerialParity)mp_obj_get_int(args[2].u_obj);
    }

    // default pins are the internal USB-UART pins
    PinName p_tx = TGT_TX;
    PinName p_rx = TGT_RX;

    // set tx/rx pins if they are given
    if (args[5].u_obj != mp_const_none) {
        p_tx = (PinName)microbit_obj_get_pin_name(args[5].u_obj);
    }
    if (args[6].u_obj != mp_const_none) {
        p_rx = (PinName)microbit_obj_get_pin_name(args[6].u_obj);
    }

    // support for legacy "pins" argument
    if (args[4].u_obj != mp_const_none) {
        mp_obj_t *pins;
        mp_obj_get_array_fixed_n(args[4].u_obj, 2, &pins);
        p_tx = (PinName)microbit_obj_get_pin_name(pins[0]);
        p_rx = (PinName)microbit_obj_get_pin_name(pins[1]);
    }

    // initialise the uart
    // Note: we don't want to call serial_init() because it sends a single 0x00
    // character on the line during initialisation.  This function has anyway
    // already been called by the MicroBitSerial constructor so it's enough to
    // just reconfigure the pins here, as would have been done by serial_init().
    serial_t serial;
    serial.uart = NRF_UART0;
    NRF_GPIO->DIR |= (1 << p_tx);
    NRF_GPIO->DIR &= ~(1 << p_rx);
    NRF_UART0->PSELTXD = p_tx;
    NRF_UART0->PSELRXD = p_rx;
    pin_mode(p_tx, PullUp);
    pin_mode(p_rx, PullUp);
    serial_baud(&serial, args[0].u_int);
    serial_format(&serial, args[1].u_int, parity, args[3].u_int);

    // set the character read timeout based on the baudrate and 13 bits
    microbit_uart_timeout_char = 13000 / args[0].u_int + 1;

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(microbit_uart_init_obj, 1, microbit_uart_init);

STATIC mp_obj_t microbit_uart_any(mp_obj_t self_in) {
    (void)self_in;
    if (mp_hal_stdin_rx_any()) {
        return mp_const_true;
    } else {
        return mp_const_false;
    }
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_uart_any_obj, microbit_uart_any);

STATIC const mp_map_elem_t microbit_uart_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&microbit_uart_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_any), (mp_obj_t)&microbit_uart_any_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&mp_stream_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_readline), (mp_obj_t)&mp_stream_unbuffered_readline_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_readinto), (mp_obj_t)&mp_stream_readinto_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&mp_stream_write_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_ODD), MP_OBJ_NEW_SMALL_INT(ParityOdd) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_EVEN), MP_OBJ_NEW_SMALL_INT(ParityEven) },
};

STATIC MP_DEFINE_CONST_DICT(microbit_uart_locals_dict, microbit_uart_locals_dict_table);

// Waits at most timeout_ms for at least 1 char to become ready for reading.
// Returns true if something available, false if not.
STATIC bool microbit_uart_rx_wait(uint32_t timeout_ms) {
    uint32_t start = mp_hal_ticks_ms();
    for (;;) {
        if (mp_hal_stdin_rx_any()) {
            return true; // have at least 1 character waiting
        }
        if (mp_hal_ticks_ms() - start >= timeout_ms) {
            return false; // timeout
        }
    }
}

STATIC mp_uint_t microbit_uart_read(mp_obj_t self_in, void *buf_in, mp_uint_t size, int *errcode) {
    (void)self_in;
    byte *buf = (byte*)buf_in;
    (void)errcode;

    // make sure we want at least 1 char
    if (size == 0) {
        return 0;
    }

    // check there is at least 1 char available
    if (!mp_hal_stdin_rx_any()) {
        *errcode = EAGAIN;
        return MP_STREAM_ERROR;
    }

    // read the data
    byte *orig_buf = buf;
    for (;;) {
        *buf++ = mp_hal_stdin_rx_chr();
        if (--size == 0 || !microbit_uart_rx_wait(microbit_uart_timeout_char)) {
            // return number of bytes read
            return buf - orig_buf;
        }
    }
}

STATIC mp_uint_t microbit_uart_write(mp_obj_t self_in, const void *buf_in, mp_uint_t size, int *errcode) {
    (void)self_in;
    const char *buf = (const char*)buf_in;
    (void)errcode;
    mp_hal_stdout_tx_strn(buf, size);
    return size;
}

STATIC const mp_stream_p_t microbit_uart_stream_p = {
    .read = microbit_uart_read,
    .write = microbit_uart_write,
    .ioctl = NULL,
    .is_text = false,
};

const mp_obj_type_t microbit_uart_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitUART,
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
    .protocol = &microbit_uart_stream_p,
    .parent = NULL,
    .locals_dict = (mp_obj_dict_t*)&microbit_uart_locals_dict,
};

microbit_uart_obj_t microbit_uart_obj = {
    {&microbit_uart_type},
};

}
