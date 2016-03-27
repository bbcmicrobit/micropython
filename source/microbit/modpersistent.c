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
#include <string.h>

#include "py/nlr.h"
#include "nrf51.h"
#include "nrf_nvmc.h"
#include "py/obj.h"
#include "py/gc.h"
#include "persistence.h"
#include "lib/ticker.h"

//#define DEBUG(s) printf s
#define DEBUG(s) (void)0

#define min(a,b) (((a)<(b))?(a):(b))

typedef struct _persistent_data_obj {
    mp_obj_base_t base;
    const int8_t *start;
    uint32_t len;
} persistent_data_obj;

STATIC void write_unchecked(const void *dest, const void *src, uint32_t len) {
    DEBUG(("Write unchecked %d bytes from %x to %x\r\n", len, src, dest));
    int8_t *address = (int8_t *)dest;
    int8_t *data = (int8_t *)src;
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

static bool is_erased(const int8_t *start, uint32_t len) {
    const int8_t *addr = start;
    const int8_t *end = addr + len;
    while ((((uint32_t)addr)&3) && addr < end) {
        if (*addr != -1) {
            DEBUG(("%d bytes from %x are not erased\r\n"));
            return false;
        }
        addr++;
    }
    while (addr + 4 <= end) {
        if (*((const int32_t *)addr) != -1) {
            DEBUG(("%d bytes from %x are not erased\r\n"));
            return false;
        }
        addr += 4;
    }
    while (addr < end) {
        if (*addr != -1) {
            DEBUG(("%d bytes from %x are not erased\r\n"));
            return false;
        }
        addr++;
    }
    DEBUG(("%d bytes from %x are erased\r\n"));
    return true;
}

STATIC void erase_page(const void *page) {
    DEBUG(("Erasing page %x\r\n", page));
    nrf_nvmc_page_erase((uint32_t)page);
}

void write_persistent(const int8_t *dest, const int8_t *src, uint32_t len) {
    DEBUG(("Write persistent %d bytes from %x to %x\r\n", len, src, dest));
    const int8_t *addr = src;
    const int8_t *end_data = src+len;
    const uint32_t page_size = NRF_FICR->CODEPAGESIZE;
    int8_t *page = (void *)(((uint32_t)dest)&(-page_size));
    int8_t *tmp = NULL;
    while (addr < end_data) {
        int8_t *next_page = page + page_size;
        uint32_t data_in_page = min(end_data-addr, next_page-dest);
        if (!is_erased(dest, data_in_page)) {
            if (tmp == NULL) {
                tmp = gc_alloc(page_size, false);
            }
            memcpy(tmp, page, page_size);
            memcpy(tmp+(dest-page), addr, data_in_page);
            erase_page(page);
            write_unchecked(page, tmp, page_size);
        } else {
            write_unchecked(dest, addr, data_in_page);
        }
        dest = page = next_page;
        addr += data_in_page;
    }
    if (tmp) {
        gc_free(tmp);
    }
}

STATIC mp_obj_t write_persistent_func(mp_obj_t self_in, mp_obj_t offset_obj, mp_obj_t data_obj) {
    persistent_data_obj *self = (persistent_data_obj *)self_in;
    int32_t offset = mp_obj_get_int(offset_obj);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data_obj, &bufinfo, MP_BUFFER_READ);
    if (((uint32_t)offset) >= self->len) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid offset"));
    }
    if (bufinfo.len + offset >= self->len) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "too much data"));
    }
    write_persistent(self->start + offset, bufinfo.buf, bufinfo.len);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(write_persistent_obj, write_persistent_func);


STATIC mp_obj_t read_persistent(mp_obj_t self_in, mp_obj_t offset_obj, mp_obj_t len_obj) {
    persistent_data_obj *self = (persistent_data_obj *)self_in;
    int32_t offset = mp_obj_get_int(offset_obj);
    uint32_t len = mp_obj_get_int(len_obj);
    if (((uint32_t)offset) >= self->len) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid offset"));
    }
    if (len + offset >= self->len) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "too much data"));
    }
    return mp_obj_new_bytes((const byte *)(self->start + offset), len);
}
MP_DEFINE_CONST_FUN_OBJ_3(read_persistent_obj, read_persistent);


STATIC void erase_pages(const persistent_data_obj *data, uint32_t len) {
    const int8_t *page = data->start;
    const int8_t *end = page + len;
    uint32_t page_size = NRF_FICR->CODEPAGESIZE;
    while (page < end) {
        erase_page(page);
        page += page_size;
    }
}

STATIC mp_obj_t erase_func(mp_obj_t self_in) {
    persistent_data_obj *self = (persistent_data_obj *)self_in;
    erase_pages(self, self->len);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(erase_obj, erase_func);

STATIC mp_obj_t page_size_func() {
    return mp_obj_new_int(NRF_FICR->CODEPAGESIZE);
}
MP_DEFINE_CONST_FUN_OBJ_0(page_size_obj, page_size_func);

STATIC const mp_map_elem_t persistent_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&read_persistent_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&write_persistent_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_erase), (mp_obj_t)&erase_obj },
};
STATIC MP_DEFINE_CONST_DICT(persistent_locals_dict, persistent_locals_dict_table);

STATIC void persistent_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    persistent_data_obj *self = (persistent_data_obj *)self_in;
    mp_printf(print, "<PersistentData@0x%08x>", (uint32_t)self->start);
}

const mp_obj_type_t persistent_data_type = {
    { &mp_type_type },
    .name = MP_QSTR_PersistentData,
    .print = persistent_print,
    .make_new = NULL,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = NULL,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = { NULL },
    .stream_p = NULL,
    .bases_tuple = NULL,
    .locals_dict = (mp_obj_dict_t*)&persistent_locals_dict,
};

// Want to initialize all data to -1 as that is the erased state of the flash EEPROM.

#define EIGHT_K (8*1024)

#define END_OF_ROM (256*1024)

#define USER_DATA_ADDRESS (END_OF_ROM-2*EIGHT_K)
#define APPENDED_SCRIPT_ADDRESS (END_OF_ROM-EIGHT_K)

persistent_data_obj user_data_obj = {
    .base = { &persistent_data_type },
    .start = (const int8_t *)USER_DATA_ADDRESS,
    .len = EIGHT_K,
};

STATIC const persistent_data_obj appended_script_obj = {
    .base = { &persistent_data_type },
    .start = (const int8_t *)APPENDED_SCRIPT_ADDRESS,
    .len = EIGHT_K,
};

STATIC mp_obj_t save_script(mp_obj_t script) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(script, &bufinfo, MP_BUFFER_READ);
    if (bufinfo.len >= EIGHT_K-4) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "script too long"));
    }
    uint16_t len = bufinfo.len;
    erase_pages(&appended_script_obj, len+4);
    write_unchecked(appended_script_obj.start, (const uint8_t *)"MP", 2);
    write_unchecked(appended_script_obj.start+2, (const uint8_t *)&len, 2);
    write_unchecked(appended_script_obj.start+4, bufinfo.buf, len);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(save_script_obj, save_script);

STATIC const mp_map_elem_t _globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_data), (mp_obj_t)&user_data_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_script), (mp_obj_t)&appended_script_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_page_size), (mp_obj_t)&page_size_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_save_script), (mp_obj_t)&save_script_obj },
};

STATIC MP_DEFINE_CONST_DICT(_globals, _globals_table);

const mp_obj_module_t persistent_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_persistent,
    .globals = (mp_obj_dict_t*)&_globals,
};


