
#ifndef __MICROPY_INCLUDED_MICROBIT_SOUND_H__
#define __MICROPY_INCLUDED_MICROBIT_SOUND_H__

#include "nrf.h"
#include "py/obj.h"
#include "py/runtime.h"

void sound_start(void);
void sound_stop(void);
mp_obj_t sound_init(void);
void sound_mute(void);
void sound_play_source(mp_obj_t src, bool wait);
mp_obj_t sound_set_pins(mp_obj_t pin0_obj, mp_obj_t pin1_obj);

#define LOG_SOUND_CHUNK_SIZE 5
#define SOUND_CHUNK_SIZE 32
#define SOUND_BUFFER_SIZE (SOUND_CHUNK_SIZE*2)
#define SOUND_CALLBACK_ID 0

typedef struct _microbit_sound_bytes_obj_t {
    mp_obj_base_t base;
    int8_t data[SOUND_CHUNK_SIZE];
} microbit_sound_bytes_obj_t;

extern const mp_obj_type_t microbit_sound_bytes_type;

microbit_sound_bytes_obj_t *new_microbit_sound_bytes(void);

typedef void (*fill_func_ptr)(struct _microbit_instrument_t *inst);

typedef struct _microbit_instrument_t {
    mp_obj_base_t base;
    bool pressed;
    bool active;
    int32_t phase;
    int32_t phase_delta;
    microbit_sound_bytes_obj_t *buffer;
    void *state;
    fill_func_ptr generator;

    inline void press() {
        this->pressed = true;
        this->phase = 0;
    }

    inline void release() {
        this->pressed = false;
    }

} microbit_instrument_t;

microbit_instrument_t *simple_organ(void);

#endif // __MICROPY_INCLUDED_MICROBIT_SOUND_H__
