
#ifndef __MICROPY_INCLUDED_MICROBIT_SOUND_H__
#define __MICROPY_INCLUDED_MICROBIT_SOUND_H__

#include "nrf.h"
#include "py/obj.h"
#include "py/runtime.h"

void sound_start(void);
void sound_stop(void);
void sound_play_source(mp_obj_t src, bool wait);

#define SOUND_CHUNK_SIZE 32
#define SOUND_BUFFER_SIZE (SOUND_CHUNK_SIZE*2)
#define SOUND_CALLBACK_ID 0

typedef struct _microbit_sound_bytes_obj_t {
    mp_obj_base_t base;
    int8_t data[SOUND_CHUNK_SIZE];
} microbit_sound_bytes_obj_t;

extern const mp_obj_type_t microbit_sound_bytes_type;

microbit_sound_bytes_obj_t *new_microbit_sound_bytes(void);

#endif // __MICROPY_INCLUDED_MICROBIT_SOUND_H__
