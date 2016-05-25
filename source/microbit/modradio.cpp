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

extern "C" {

#include <stdio.h>
#include <string.h>

#include "py/runtime0.h"
#include "py/runtime.h"
#include "microbitobj.h"

static uint8_t *buf_start = NULL; // XXX root pointer
static uint8_t *buf_end = NULL;
static uint8_t *rx_buf = NULL;

void RADIO_IRQHandler(void) {
    if (NRF_RADIO->EVENTS_READY) {
        NRF_RADIO->EVENTS_READY = 0;
        NRF_RADIO->TASKS_START = 1;
    }

    if (NRF_RADIO->EVENTS_END) {
        NRF_RADIO->EVENTS_END = 0;

        size_t max_len = NRF_RADIO->PCNF1 & 0xff;
        size_t len = rx_buf[0];
        if (len > max_len) {
            len = max_len;
            rx_buf[0] = len;
        }

        //printf("radio end pos=%d len=%d [%d %d %d %d]\r\n", rx_buf - buf_start, len, rx_buf[0], rx_buf[1], rx_buf[2], rx_buf[3]);

        /*
        if (NRF_RADIO->CRCSTATUS == 1) {
            printf("rssi: %d\r\n", -NRF_RADIO->RSSISAMPLE);
        }
        */

        if (rx_buf + 1 + len + 1 + max_len <= buf_end) {
            rx_buf += 1 + len;
            NRF_RADIO->PACKETPTR = (uint32_t)rx_buf;
        }

        NRF_RADIO->TASKS_START = 1;
    }
}

static void radio_disable(void) {
    NVIC_DisableIRQ(RADIO_IRQn);
    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_DISABLE = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0);
}

static void radio_enable(size_t max_payload, size_t queue_len) {
    radio_disable();

    // free any old buffers
    if (buf_start != NULL) {
        m_del(uint8_t, buf_start, buf_end - buf_start);
    }

    // allocate tx and rx buffers
    queue_len += 1; // one extra for tx buffer
    buf_start = m_new(uint8_t, (max_payload + 1) * queue_len);
    buf_end = buf_start + (max_payload + 1) * queue_len;
    rx_buf = buf_start + max_payload + 1; // start is tx buffer

    // Enable the High Frequency clock on the processor. This is a pre-requisite for
    // the RADIO module. Without this clock, no communication is possible.
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);

    // power should be one off: -30, -20, -16, -12, -8, -4, 0, 4
    NRF_RADIO->TXPOWER = 0;

    // should be between 0 and 100 inclusive (actual physical freq is 2400MHz + this register)
    NRF_RADIO->FREQUENCY = 7;

    // Configure for 1Mbps throughput.
    // This may sound excessive, but running a high data rates reduces the chances of collisions...
    NRF_RADIO->MODE = RADIO_MODE_MODE_Nrf_1Mbit;

    // Configure the addresses we use for this protocol. We run ANONYMOUSLY at the core.
    // A 40 bit addresses is used. The first 32 bits match the ASCII character code for "uBit".
    // We also map the assigned 8-bit GROUP id into the PREFIX field. This allows the RADIO hardware to perform
    // address matching for us, and only generate an interrupt when a packet matching our group is received.
    NRF_RADIO->BASE0 = 0x75626974;

    // Join the default group. This will configure the remaining byte in the RADIO hardware module.
    NRF_RADIO->PREFIX0 = 0;

    // The RADIO hardware module supports the use of multiple addresses, but as we're running anonymously, we only need one.
    // Configure the RADIO module to use the default address (address 0) for both send and receive operations.
    NRF_RADIO->TXADDRESS = 0;
    NRF_RADIO->RXADDRESSES = 1;

    // LFLEN=8 bits, S0LEN=0, S1LEN=0
    NRF_RADIO->PCNF0 = 0x00000008;
    // STATLEN=0, BALEN=4, ENDIAN=0 (little), WHITEEN=1
    NRF_RADIO->PCNF1 = 0x02040000 | max_payload;

    // Enable automatic 16bit CRC generation and checking, and configure how the CRC is calculated.
    NRF_RADIO->CRCCNF = RADIO_CRCCNF_LEN_Two;
    NRF_RADIO->CRCINIT = 0xFFFF;
    NRF_RADIO->CRCPOLY = 0x11021;

    // Set the start random value of the data whitening algorithm. This can be any non zero number.
    NRF_RADIO->DATAWHITEIV = 0x18;

    // set receive buffer
    NRF_RADIO->PACKETPTR = (uint32_t)rx_buf;

    // configure interrupts
    NRF_RADIO->INTENSET = 0x00000008;
    NVIC_ClearPendingIRQ(RADIO_IRQn);
    NVIC_EnableIRQ(RADIO_IRQn);

    NRF_RADIO->SHORTS |= RADIO_SHORTS_ADDRESS_RSSISTART_Msk;

    // enable receiver
    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->TASKS_RXEN = 1;
    while (NRF_RADIO->EVENTS_READY == 0);

    NRF_RADIO->EVENTS_END = 0;
    NRF_RADIO->TASKS_START = 1;
}

void radio_send(const uint8_t *buf, size_t len) {
    // construct the packet
    // note: we must send from RAM
    size_t max_len = NRF_RADIO->PCNF1 & 0xff;
    if (len > max_len) {
        len = max_len;
    }
    buf_start[0] = len;
    memcpy(buf_start + 1, buf, len);

    // transmission will occur synchronously
    NVIC_DisableIRQ(RADIO_IRQn);

    // Turn off the transceiver.
    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_DISABLE = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0);

    // Configure the radio to send the buffer provided.
    NRF_RADIO->PACKETPTR = (uint32_t)buf_start;

    // Turn on the transmitter, and wait for it to signal that it's ready to use.
    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->TASKS_TXEN = 1;
    while (NRF_RADIO->EVENTS_READY == 0);

    // Start transmission and wait for end of packet.
    NRF_RADIO->TASKS_START = 1;
    NRF_RADIO->EVENTS_END = 0;
    while (NRF_RADIO->EVENTS_END == 0);

    // Return the radio to using the default receive buffer
    NRF_RADIO->PACKETPTR = (uint32_t)rx_buf;

    // Turn off the transmitter.
    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_DISABLE = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0);

    // Start listening for the next packet
    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->TASKS_RXEN = 1;
    while (NRF_RADIO->EVENTS_READY == 0);

    NRF_RADIO->EVENTS_END = 0;
    NRF_RADIO->TASKS_START = 1;

    NVIC_ClearPendingIRQ(RADIO_IRQn);
    NVIC_EnableIRQ(RADIO_IRQn);
}

/*****************************************************************************/
// MicroPython bindings and module

STATIC mp_obj_t mod_radio_enable(mp_obj_t max_payload_in, mp_obj_t queue_len_in) {
    mp_int_t max_payload = mp_obj_get_int(max_payload_in);
    mp_int_t queue_len = mp_obj_get_int(queue_len_in);
    radio_enable(max_payload, queue_len);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mod_radio_enable_obj, mod_radio_enable);

STATIC mp_obj_t mod_radio_disable(void) {
    radio_disable();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(mod_radio_disable_obj, mod_radio_disable);

STATIC mp_obj_t mod_radio_send(mp_obj_t buf_in) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);
    radio_send((const uint8_t*)bufinfo.buf, bufinfo.len);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mod_radio_send_obj, mod_radio_send);

STATIC mp_obj_t mod_radio_recv(void) {
    NVIC_DisableIRQ(RADIO_IRQn);
    uint8_t *buf = buf_start + (NRF_RADIO->PCNF1 & 0xff) + 1; // skip tx buf
    if (rx_buf == buf) {
        NVIC_EnableIRQ(RADIO_IRQn);
        return mp_const_none;
    }
    size_t len = buf[0];
    mp_obj_t ret = mp_obj_new_bytes(buf + 1, len); // if it raises the radio irq remains disabled...
    memmove(buf, buf + 1 + len, rx_buf - (buf + 1 + len));
    rx_buf -= 1 + len;
    NRF_RADIO->PACKETPTR = (uint32_t)rx_buf;
    NRF_RADIO->EVENTS_END = 0;
    NRF_RADIO->TASKS_START = 1;
    NVIC_EnableIRQ(RADIO_IRQn);
    return ret;
}
MP_DEFINE_CONST_FUN_OBJ_0(mod_radio_recv_obj, mod_radio_recv);

STATIC const mp_map_elem_t radio_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_radio) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_enable), (mp_obj_t)&mod_radio_enable_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_disable), (mp_obj_t)&mod_radio_disable_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_send), (mp_obj_t)&mod_radio_send_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_recv), (mp_obj_t)&mod_radio_recv_obj },
};

STATIC MP_DEFINE_CONST_DICT(radio_module_globals, radio_module_globals_table);

const mp_obj_module_t radio_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_radio,
    .globals = (mp_obj_dict_t*)&radio_module_globals,
};

}
