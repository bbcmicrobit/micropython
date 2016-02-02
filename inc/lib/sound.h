#ifndef __MICROPY_INCLUDED_LIB_SOUND_H__
#define __MICROPY_INCLUDED_LIB_SOUND_H__


#include "nrf.h"
#include "py/obj.h"

void play_test_sample(void);
void sound_init(void);
void sound_start(void);
void sound_stop(void);
int32_t sound_get_rate(void);
int  sound_set_rate(int32_t rate);
void reset_buffer(void);
void sound_play_source(mp_obj_t src, bool wait);

#define SOUND_CHUNK_SIZE 32
#define SOUND_BUFFER_SIZE (SOUND_CHUNK_SIZE*2)
#define SOUND_CALLBACK_ID 0

extern int8_t *sound_buffer_ptr;


#endif // __MICROPY_INCLUDED_LIB_SOUND_H__
