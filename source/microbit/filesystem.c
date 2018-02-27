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
#include <errno.h>
#include <sys/stat.h>

#include "py/runtime.h"
#include "py/obj.h"
#include "py/gc.h"
#include "py/stream.h"
#include "microbit/filesystem.h"
#include "microbit/memory.h"

#define DEBUG_FILE 0
#if DEBUG_FILE
#define DEBUG(s) printf s
#else
#define DEBUG(s) (void)0
#endif

/**  How it works:
 * The File System consists of up to MAX_CHUNKS_IN_FILE_SYSTEM chunks of CHUNK_SIZE each,
 * plus one spare page which holds persistent configuration data and is used. for bulk erasing.
 * The spare page is either the first or the last page and will be switched by a bulk erase.
 * The exact number of chunks will depend on the amount of flash available.
 *
 * Each chunk consists of a one byte marker and a one byte tail
 * The marker shows whether this chunk is the start of a file, the midst of a file
 * (in which case it refers to the previous chunk in the file) or whether it is UNUSED
 * (and erased) or FREED (which means it is unused, but not erased).
 * Chunks are selected in a randomised round-robin fashion to even out wear on the flash
 * memory as much as possible.
 * A file consists of a linked list of chunks. The first chunk in a file contains its name
 * as well as the end chunk and offset.
 * Files are found by linear search of the chunks, this means that no meta-data needs to be stored
 * outside of the file, which prevents wear hot-spots. Since there are fewer than 250 chunks,
 * the search is fast enough.
 *
 * Chunks are numbered from 1 as we need to reserve 0 as the FREED marker.
 *
 * Writing to files relies on the persistent API which is high-level wrapper on top of the Nordic SDK.
 */

/** Page indexes count down from the end of ROM */
static uint8_t first_page_index;
static uint8_t last_page_index;
/** The number of useable chunks in the file system */
static uint8_t chunks_in_file_system;
/** Index of chunk to start searches. This is randomised to even out wear */
static uint8_t start_index;
static file_chunk *file_system_chunks;

STATIC_ASSERT((sizeof(file_chunk) == CHUNK_SIZE));


static inline void *first_page(void) {
    return microbit_end_of_rom() - persistent_page_size() * first_page_index;
}

static inline void *last_page(void) {
    return microbit_end_of_rom() - persistent_page_size() * last_page_index;
}

static void init_limits(void) {
    /* First determine where to end */
    char *end = (char*)microbit_compass_calibration_page() - persistent_page_size();
    last_page_index = (microbit_end_of_rom() - end)/persistent_page_size();
    /** Now find the start */
    char *start = roundup(end - CHUNK_SIZE*MAX_CHUNKS_IN_FILE_SYSTEM, persistent_page_size());
    while (start < microbit_end_of_code()) {
        start += persistent_page_size();
    }
    first_page_index = (microbit_end_of_rom() - start)/persistent_page_size();
    chunks_in_file_system = (end-start)>>LOG_CHUNK_SIZE;
}

static void randomise_start_index(void) {
    uint8_t new_index; // 0 based index.
    NRF_RNG->TASKS_START = 1;
    // Wait for valid number
    do {
        NRF_RNG->EVENTS_VALRDY = 0;
        while(NRF_RNG->EVENTS_VALRDY == 0);
        new_index = NRF_RNG->VALUE&255;
    } while (new_index >= chunks_in_file_system);
    start_index = new_index + 1;  // Adjust index to 1 based.
    NRF_RNG->TASKS_STOP = 1;
}

void microbit_filesystem_init(void) {
    init_limits();
    randomise_start_index();
    file_chunk *base = first_page();
    if (base->marker == PERSISTENT_DATA_MARKER) {
        file_system_chunks = &base[(persistent_page_size()>>LOG_CHUNK_SIZE)-1];
    } else if (((file_chunk *)last_page())->marker == PERSISTENT_DATA_MARKER) {
        file_system_chunks = &base[-1];
    } else {
        persistent_write_byte_unchecked(&((file_chunk *)last_page())->marker, PERSISTENT_DATA_MARKER);
        file_system_chunks = &base[-1];
    }
}

static void copy_page(void *dest, void *src) {
    DEBUG(("FILE DEBUG: Copying page from %lx to %lx.\r\n", (uint32_t)src, (uint32_t)dest));
    persistent_erase_page(dest);
    file_chunk *src_chunk = src;
    file_chunk *dest_chunk = dest;
    uint32_t chunks = persistent_page_size()>>LOG_CHUNK_SIZE;
    for (uint32_t i = 0; i < chunks; i++) {
        if (src_chunk[i].marker != FREED_CHUNK) {
            persistent_write_unchecked(&dest_chunk[i], &src_chunk[i], CHUNK_SIZE);
        }
    }
}

/* Move entire file system up or down one page, copying all used chunks
 * Freed chunks are not copied, so become erased.
 * There should be no erased chunks before the sweep (or it would be unnecessary)
 * but if there are this should work correctly.
 *
 * The direction of the sweep depends on whether the persistent data is in the first or last page
 * The persistent data is copied to RAM, leaving its page unused.
 * Then all the pages are copied, one by one, into the adjacent newly unused page.
 * Finally, the persistent data is saved back to the opposite end of the filesystem from whence it came.
 */
void filesystem_sweep(void) {
    persistent_config_t config;
    uint8_t *page;
    uint8_t *end_page;
    int step;
    uint32_t page_size = persistent_page_size();
    DEBUG(("FILE DEBUG: Sweeping file system\r\n"));
    if (((file_chunk *)first_page())->marker == PERSISTENT_DATA_MARKER) {
        config = *(persistent_config_t *)first_page();
        page = first_page();
        end_page = last_page();
        step = page_size;
    } else {
        config = *(persistent_config_t *)last_page();
        page = last_page();
        end_page = first_page();
        step = -page_size;
    }
    while (page != end_page) {
        uint8_t *next_page = page+step;
        persistent_erase_page(page);
        copy_page(page, next_page);
        page = next_page;
    }
    persistent_erase_page(end_page);
    persistent_write_unchecked(end_page, &config, sizeof(config));
    microbit_filesystem_init();
}


static inline char *seek_address(file_descriptor_obj *self) {
    return (char *)&(file_system_chunks[self->seek_chunk].data[self->seek_offset]);
}

uint8_t microbit_find_file(const char *name, int name_len) {
    for (uint8_t index = 1; index <= chunks_in_file_system; index++) {
        const file_chunk *p = &file_system_chunks[index];
        if (p->marker != FILE_START)
            continue;
        if (p->header.name_len != name_len)
            continue;
        if (memcmp(name, &p->header.filename[0], name_len) == 0) {
            DEBUG(("FILE DEBUG: File found. index %d\r\n", index));
            return index;
        }
    }
    DEBUG(("FILE DEBUG: File not found.\r\n"));
    return FILE_NOT_FOUND;
}

/** Return a free, erased chunk.
 * Search the chunks:
 * 1  If an UNUSED chunk is found, then return that.
 * 2. If an entire page of FREED chunks is found, then erase the page and return the first chunk
 * 3. If the number of FREED chunks is >= MIN_FREE_CHUNKS_FOR_SWEEP, then
 * 3a. Sweep the filesystem and restart.
 * 3b. Fail and return FILE_NOT_FOUND
 */
static uint8_t find_chunk_and_erase(void) {
    // Start search at a random chunk to spread the wear more evenly.
    // Search for unused chunk
    uint8_t index = start_index;
    do {
        const file_chunk *p = &file_system_chunks[index];
        if (p->marker == UNUSED_CHUNK) {
            DEBUG(("FILE DEBUG: Unused chunk found: %d\r\n", index));
            return index;
        }
        index++;
        if (index == chunks_in_file_system+1) index = 1;
    } while (index != start_index);

    // Search for FREED page, and total up FREED chunks
    uint32_t freed_chunks = 0;
    index = start_index;
    uint32_t chunks_per_page = persistent_page_size()>>LOG_CHUNK_SIZE;
    do {
        const file_chunk *p = &file_system_chunks[index];
        if (p->marker == FREED_CHUNK) {
            freed_chunks++;
        }
        if (is_persistent_page_aligned(p)) {
            uint32_t i;
            for (i = 0; i < chunks_per_page; i++) {
                if (p[i].marker != FREED_CHUNK)
                    break;
            }
            if (i == chunks_per_page) {
                DEBUG(("FILE DEBUG: Found freed page of chunks: %d\r\n", index));
                persistent_erase_page(&file_system_chunks[index]);
                return index;
            }
        }
        index++;
        if (index == chunks_in_file_system+1) index = 1;
    } while (index != start_index);
    DEBUG(("FILE DEBUG: %lu free chunks\r\n", freed_chunks));
    if (freed_chunks < MIN_CHUNKS_FOR_SWEEP) {
        return FILE_NOT_FOUND;
    }
    // No freed pages, so sweep file system.
    filesystem_sweep();
    // This is guaranteed to succeed.
    return find_chunk_and_erase();
}

mp_obj_t microbit_file_name(file_descriptor_obj *fd) {
    return mp_obj_new_str(&(file_system_chunks[fd->start_chunk].header.filename[0]), file_system_chunks[fd->start_chunk].header.name_len, false);
}

static file_descriptor_obj *microbit_file_descriptor_new(uint8_t start_chunk, bool write, bool binary);

static void clear_file(uint8_t chunk) {
    do {
        persistent_write_byte_unchecked(&(file_system_chunks[chunk].marker), FREED_CHUNK);
        DEBUG(("FILE DEBUG: Freeing chunk %d.\n", chunk));
        chunk = file_system_chunks[chunk].next_chunk;
    } while (chunk <= chunks_in_file_system);
}

file_descriptor_obj *microbit_file_open(const char *name, uint32_t name_len, bool write, bool binary) {
    if (name_len > MAX_FILENAME_LENGTH) {
        return NULL;
    }
    uint8_t index = microbit_find_file(name, name_len);
    if (write) {
        if (index != FILE_NOT_FOUND) {
            // Free old file
            clear_file(index);
        }
        index = find_chunk_and_erase();
        if (index == FILE_NOT_FOUND) {
            mp_raise_msg(&mp_type_OSError, "no more storage space");
        }
        persistent_write_byte_unchecked(&(file_system_chunks[index].marker), FILE_START);
        persistent_write_byte_unchecked(&(file_system_chunks[index].header.name_len), name_len);
        persistent_write_unchecked(&(file_system_chunks[index].header.filename[0]), name, name_len);
    } else {
        if (index == FILE_NOT_FOUND) {
            return NULL;
        }
    }
    return microbit_file_descriptor_new(index, write, binary);
}

static file_descriptor_obj *microbit_file_descriptor_new(uint8_t start_chunk, bool write, bool binary) {
    file_descriptor_obj *res = m_new_obj(file_descriptor_obj);
    if (binary) {
        res->base.type = &microbit_bytesio_type;
    } else {
        res->base.type = &microbit_textio_type;
    }
    res->start_chunk = start_chunk;
    res->seek_chunk = start_chunk;
    res->seek_offset = file_system_chunks[start_chunk].header.name_len+2;
    res->writable = write;
    res->open = true;
    res->binary = binary;
    return res;
}

mp_obj_t microbit_remove(mp_obj_t filename) {
    mp_uint_t name_len;
    const char *name = mp_obj_str_get_data(filename, &name_len);
    mp_uint_t index = microbit_find_file(name, name_len);
    if (index == 255) {
        mp_raise_msg(&mp_type_OSError, "file not found");
    }
    clear_file(index);
    return mp_const_none;
}

static void check_file_open(file_descriptor_obj *self) {
    if (!self->open) {
        mp_raise_ValueError("I/O operation on closed file");
    }
}

static int advance(file_descriptor_obj *self, uint32_t n, bool write) {
    DEBUG(("FILE DEBUG: Advancing from chunk %d, offset %d.\r\n", self->seek_chunk, self->seek_offset));
    self->seek_offset += n;
    if (self->seek_offset == DATA_PER_CHUNK) {
        self->seek_offset = 0;
        if (write) {
            uint8_t next_chunk = find_chunk_and_erase();
            if (next_chunk == FILE_NOT_FOUND) {
                clear_file(self->start_chunk);
                self->open = false;
                return ENOSPC;
            }
            /* Link next chunk to this one */
            persistent_write_byte_unchecked(&(file_system_chunks[self->seek_chunk].next_chunk), next_chunk);
            persistent_write_byte_unchecked(&(file_system_chunks[next_chunk].marker), self->seek_chunk);
        }
        self->seek_chunk = file_system_chunks[self->seek_chunk].next_chunk;
    }
    DEBUG(("FILE DEBUG: Advanced to chunk %d, offset %d.\r\n", self->seek_chunk, self->seek_offset));
    return 0;
}

mp_uint_t microbit_file_read(mp_obj_t obj, void *buf, mp_uint_t size, int *errcode) {
    file_descriptor_obj *self = (file_descriptor_obj *)obj;
    check_file_open(self);
    if (self->writable || file_system_chunks[self->start_chunk].marker == FREED_CHUNK) {
        *errcode = EBADF;
        return MP_STREAM_ERROR;
    }
    uint32_t bytes_read = 0;
    uint8_t *data = buf;
    while (1) {
        mp_uint_t to_read = DATA_PER_CHUNK - self->seek_offset;
        if (file_system_chunks[self->seek_chunk].next_chunk == UNUSED_CHUNK) {
            uint8_t end_offset = file_system_chunks[self->start_chunk].header.end_offset;
            if (end_offset == UNUSED_CHUNK) {
                to_read = 0;
            } else {
                to_read = min(to_read, (mp_uint_t)end_offset-self->seek_offset);
            }
        }
        to_read = min(to_read, size-bytes_read);
        if (to_read == 0) {
            break;
        }
        memcpy(data+bytes_read, seek_address(self), to_read);
        advance(self, to_read, false);
        bytes_read += to_read;
    }
    return bytes_read;
}

mp_uint_t microbit_file_write(mp_obj_t obj, const void *buf, mp_uint_t size, int *errcode) {
    file_descriptor_obj *self = (file_descriptor_obj *)obj;
    check_file_open(self);
    if (!self->writable || file_system_chunks[self->start_chunk].marker == FREED_CHUNK) {
        *errcode = EBADF;
        return MP_STREAM_ERROR;
    }
    uint32_t len = size;
    const uint8_t *data = buf;
    while (len) {
        uint32_t to_write = min(((uint32_t)(DATA_PER_CHUNK - self->seek_offset)), len);
        persistent_write_unchecked(seek_address(self), data, to_write);
        int err = advance(self, to_write, true);
        if (err) {
            *errcode = err;
            return MP_STREAM_ERROR;
        }
        data += to_write;
        len -= to_write;
    }
    return size;
}

void microbit_file_close(file_descriptor_obj *fd) {
    if (fd->writable) {
        persistent_write_byte_unchecked(&(file_system_chunks[fd->start_chunk].header.end_offset), fd->seek_offset);
    }
    fd->open = false;
}

mp_obj_t microbit_file_list(void) {
    mp_obj_t res = mp_obj_new_list(0, NULL);
    for (uint8_t index = 1; index <= chunks_in_file_system; index++) {
        if (file_system_chunks[index].marker == FILE_START) {
            mp_obj_t name = mp_obj_new_str(&file_system_chunks[index].header.filename[0], file_system_chunks[index].header.name_len, false);
            mp_obj_list_append(res, name);
        }
    }
    return res;
}

mp_obj_t microbit_file_size(mp_obj_t filename) {
    mp_uint_t name_len;
    const char *name = mp_obj_str_get_data(filename, &name_len);
    uint8_t chunk = microbit_find_file(name, name_len);
    if (chunk == 255) {
        mp_raise_msg(&mp_type_OSError, "file not found");
    }
    mp_uint_t len = 0;
    uint8_t end_offset = file_system_chunks[chunk].header.end_offset;
    uint8_t offset = file_system_chunks[chunk].header.name_len+2;
    while (file_system_chunks[chunk].next_chunk != UNUSED_CHUNK) {
        len += DATA_PER_CHUNK - offset;
        chunk = file_system_chunks[chunk].next_chunk;
        offset = 0;
    }
    len += end_offset - offset;
    return mp_obj_new_int(len);
}

static mp_uint_t file_read_byte(void *fd_in) {
    file_descriptor_obj *fd = fd_in;
    if (file_system_chunks[fd->seek_chunk].next_chunk == UNUSED_CHUNK) {
        uint8_t end_offset = file_system_chunks[fd->start_chunk].header.end_offset;
        if (end_offset == UNUSED_CHUNK || fd->seek_offset == end_offset) {
            return (mp_uint_t)-1;
        }
    }
    mp_uint_t res = file_system_chunks[fd->seek_chunk].data[fd->seek_offset];
    advance(fd, 1, false);
    return res;
}

mp_lexer_t *microbit_file_lexer(qstr src_name, file_descriptor_obj *fd) {
    mp_reader_t reader = {fd, file_read_byte, (void(*)(void*))microbit_file_close};
    return mp_lexer_new(src_name, reader);
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    file_descriptor_obj *fd = microbit_file_open(filename, strlen(filename), false, false);
    if (fd == NULL)
        return NULL;
    return microbit_file_lexer(qstr_from_str(filename), fd);
}
