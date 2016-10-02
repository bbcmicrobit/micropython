
#ifndef __MICROPY_INCLUDED_MICROBIT_AUDIO_H__
#define __MICROPY_INCLUDED_MICROBIT_AUDIO_H__

#include "nrf.h"
#include "py/obj.h"
#include "py/runtime.h"

void audio_play_source(mp_obj_t src, mp_obj_t pin1, mp_obj_t pin2, bool wait);
void audio_stop(void);

#define LOG_AUDIO_CHUNK_SIZE 5
#define AUDIO_CHUNK_SIZE (1<<LOG_AUDIO_CHUNK_SIZE)
#define AUDIO_BUFFER_SIZE (AUDIO_CHUNK_SIZE*2)
#define AUDIO_CALLBACK_ID 0

typedef struct _microbit_audio_frame_obj_t {
    mp_obj_base_t base;
    uint8_t data[AUDIO_CHUNK_SIZE];
} microbit_audio_frame_obj_t;

extern const mp_obj_type_t microbit_audio_frame_type;

bool microbit_audio_is_playing(void);

microbit_audio_frame_obj_t *new_microbit_audio_frame(void);

#endif // __MICROPY_INCLUDED_MICROBIT_AUDIO_H__
