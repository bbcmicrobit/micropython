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
#ifndef __MICROPY_INCLUDED_FILESYSTEM_H__
#define __MICROPY_INCLUDED_FILESYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf51.h"
#include "nrf_nvmc.h"
#include "py/lexer.h"

static inline uint32_t persistent_page_size(void) {
    return NRF_FICR->CODEPAGESIZE;
}

bool is_persistent_page_aligned(const void *ptr);

/** WARNING: This function may require 1k of heap space in order to hold data when erasing a page.
 * Returns -1 if it cannot allocate sufficient memory.
 */
int persistent_write(const void *dest, const void *src, uint32_t byte_count);
/** WARNING: This function may require 1k of heap space in order to hold data when erasing a page.
 * Returns -1 if it cannot allocate sufficient memory.
 */
int persistent_write_byte(const uint8_t *dest, const uint8_t val);

void persistent_write_unchecked(const void *dest, const void *src, uint32_t byte_count);
void persistent_write_byte_unchecked(const uint8_t *dest, const uint8_t val);

void persistent_erase_page(const void *page);

typedef struct _file_descriptor_obj {
    mp_obj_base_t base;
    uint8_t start_chunk;
    uint8_t seek_chunk;
    uint8_t seek_offset;
    bool writable;
    bool open;
    bool binary;
} file_descriptor_obj;

#define LOG_CHUNK_SIZE 7
#define CHUNK_SIZE (1<<LOG_CHUNK_SIZE)
#define DATA_PER_CHUNK (CHUNK_SIZE-2)

#define UNUSED_CHUNK 255
#define FREED_CHUNK  0
#define FILE_START 254
#define PERSISTENT_DATA_MARKER 253

/** Must be such that sizeof(file_header) < DATA_PER_CHUNK */
#define MAX_FILENAME_LENGTH 120

//Minimum number of free chunks to justify sweeping.
//If this is too low it may cause excessive wear
#define MIN_CHUNKS_FOR_SWEEP 8

typedef struct _file_header {
    uint8_t end_offset;
    uint8_t name_len;
    char filename[MAX_FILENAME_LENGTH];
} file_header;

typedef struct _file_chunk {
    uint8_t marker;
    union {
        char data[DATA_PER_CHUNK];
        file_header header;
    };
    uint8_t next_chunk;
} file_chunk;

typedef struct _persistent_config_t {
    // Must start with a marker, so that we can identify it.
    uint8_t marker; // Should always be PERSISTENT_DATA_MARKER
} persistent_config_t;

#define FILE_NOT_FOUND ((uint8_t)-1)

/** Maximum number of chunks allowed in filesystem. 240 chunks is 30kb */
#define MAX_CHUNKS_IN_FILE_SYSTEM 240

#define STATIC_ASSERT(e) extern char static_assert_failed[(e) ? 1 : -1]

uint8_t microbit_find_file(const char *name, int name_len);
file_descriptor_obj *microbit_file_open(const char *name, uint32_t name_len, bool write, bool binary);
void microbit_file_close(file_descriptor_obj *fd);
mp_uint_t microbit_file_read(mp_obj_t obj, void *buf, mp_uint_t size, int *errcode);
mp_uint_t microbit_file_write(mp_obj_t obj, const void *buf, mp_uint_t size, int *errcode);
mp_obj_t microbit_file_name(file_descriptor_obj *fd);

mp_obj_t microbit_remove(mp_obj_t filename);
mp_obj_t microbit_file_list(void);
mp_obj_t microbit_file_size(mp_obj_t filename);

mp_lexer_t *microbit_file_lexer(qstr src_name, file_descriptor_obj *fd);

void microbit_filesystem_init(void);

extern const mp_obj_type_t microbit_bytesio_type;
extern const mp_obj_type_t microbit_textio_type;

#define min(a,b) (((a)<(b))?(a):(b))

#ifdef __cplusplus
}
#endif

#endif // __MICROPY_INCLUDED_FILESYSTEM_H__
