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
#include <string.h>
#include <stdio.h>

#include "py/nlr.h"
#include "py/obj.h"
#include "py/gc.h"
#include "microbit/filesystem.h"
#include "lib/ticker.h"

#define DEBUG_PERSISTENT 0
#if DEBUG_PERSISTENT
#define DEBUG(s) printf s
#else
#define DEBUG(s) (void)0
#endif



void persistent_write_byte_unchecked(const uint8_t *dest, const uint8_t val) {
#if DEBUG_PERSISTENT
    if (((~(*dest)) & val) != 0) {
        DEBUG(("PERSISTENCE DEBUG: ERROR: Unchecked write of byte %u to %lx which contains %u\r\n", val, (uint32_t)dest, *dest));
        mp_raise_msg(&mp_type_Exception, "Internal error: Attempting illegal write.");
    }
#endif
    DEBUG(("PERSISTENCE DEBUG: Write unchecked byte %u to %lx, previous value %u\r\n", val, (uint32_t)dest, *dest));
    // Writing to flash will stop the CPU, so we stop the ticker to minimise odd behaviour.
    ticker_stop();
    nrf_nvmc_write_byte((uint32_t)dest, val);
    ticker_start();
}

void persistent_write_unchecked(const void *dest, const void *src, uint32_t len) {
    DEBUG(("PERSISTENCE DEBUG: Write unchecked %lu bytes from %lx to %lx\r\n", len, (uint32_t)src, (uint32_t)dest));
    int8_t *address = (int8_t *)dest;
    int8_t *data = (int8_t *)src;
#if DEBUG_PERSISTENT
    for(uint32_t i = 0; i < len; i++) {
        if ((~address[i] & data[i]) != 0) {
            DEBUG(("PERSISTENCE DEBUG: ERROR: Unchecked write of byte %u to %lx which contains %u\r\n", data[i], (uint32_t)&address[i], address[i]));
            mp_raise_msg(&mp_type_Exception, "Internal error: Attempting illegal write.");
        }
    }
#endif
    // Writing to flash will stop the CPU, so we stop the ticker to minimise odd behaviour.
    ticker_stop();
    // Aligned word writes are over 4 times as fast per byte, so use those if we can.
    if ((((uint32_t)address) & 3) == 0 && (((uint32_t)data) & 3) == 0 && len >= 4) {
        nrf_nvmc_write_words((uint32_t)address, (const uint32_t *)data, len>>2);
        address += (len>>2)<<2;
        data += (len>>2)<<2;
        len &= 3;
    }
    if (len) {
        nrf_nvmc_write_bytes((uint32_t)address, (const uint8_t *)data, len);
    }
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    ticker_start();
}

static inline bool can_write(const int8_t *dest, const int8_t *src, uint32_t len) {
    const int8_t *end = dest + len;
    while (dest < end) {
        if (((~(*dest)) & *src) != 0) {
            DEBUG(("PERSISTENCE DEBUG: %lu bytes from %lx need to be erased\r\n", len, (uint32_t)dest));
            return false;
        }
        dest++;
        src++;
    }
    DEBUG(("PERSISTENCE DEBUG: %lu bytes from %lx do not need to be erased\r\n", len, (uint32_t)dest));
    return true;
}

void persistent_erase_page(const void *page) {
    DEBUG(("PERSISTENCE DEBUG: Erasing page %lx\r\n", (uint32_t)page));
    // Writing to flash will stop the CPU, so we stop the ticker to minimise odd behaviour.
    ticker_stop();
    nrf_nvmc_page_erase((uint32_t)page);
    ticker_start();
}

bool is_persistent_page_aligned(const void *ptr) {
    return (((uint32_t)ptr) & (persistent_page_size()-1)) == 0;
}

int persistent_write(const void *dst, const void *src, uint32_t len) {
    DEBUG(("PERSISTENCE DEBUG: Write persistent %lu bytes from %lx to %lx\r\n", len, (uint32_t)src, (uint32_t)dst));
    const int8_t *dest = dst;
    const int8_t *addr = src;
    const int8_t *end_data = src+len;
    const uint32_t page_size = persistent_page_size();
    int8_t *page = (void *)(((uint32_t)dest)&(-page_size));
    int8_t *tmp_storage = NULL;
    while (addr < end_data) {
        int8_t *next_page = page + page_size;
        uint32_t data_in_page = min(end_data-addr, next_page-dest);
        if (can_write(dest, addr, data_in_page)) {
            persistent_write_unchecked(dest, addr, data_in_page);
        } else {
            if (tmp_storage == NULL) {
                tmp_storage = m_new(int8_t, page_size);
                if (tmp_storage == NULL) {
                    return -1;
                }
            }
            memcpy(tmp_storage, page, page_size);
            memcpy(tmp_storage+(dest-page), addr, data_in_page);
            persistent_erase_page(page);
            persistent_write_unchecked(page, tmp_storage, page_size);
        }
        dest = page = next_page;
        addr += data_in_page;
    }
    if (tmp_storage) {
        gc_free(tmp_storage);
    }
    return 0;
}

int persistent_write_byte(const uint8_t *dest, const uint8_t val) {
    DEBUG(("PERSISTENCE DEBUG: Write persistent byte %u to %lx\r\n", val, (uint32_t)dest));
    if (((~(*dest)) & val) == 0) {
        // Writing to flash will stop the CPU, so we stop the ticker to minimise odd behaviour.
        ticker_stop();
        nrf_nvmc_write_byte((uint32_t)dest, val);
        ticker_start();
        return 0;
    } else {
        return persistent_write(dest, &val, 1);
    }
}
