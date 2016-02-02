#ifndef __MICROPY_INCLUDED_LIB_TICKER_H__
#define __MICROPY_INCLUDED_LIB_TICKER_H__


#include "nrf.h"

void ticker_init(void);
void ticker_start(void);
void ticker_stop(void);

typedef void (*sound_func_ptr)(void);
typedef int32_t (*ticker_callback_ptr)(void);

void set_sound_callback(sound_func_ptr func);
void set_display_callback(ticker_callback_ptr func, int32_t initial_delay);
void set_other_callback(ticker_callback_ptr func, int32_t initial_delay);

void clear_sound_callback(void);
void clear_display_callback(void);
void clear_other_callback(void);

int set_low_priority_callback(sound_func_ptr callback, int id);

extern int8_t sound_callback_pending;
void sound_callback(void);

#define MICROSECONDS_PER_TICK 25
#define CYCLES_PER_TICK (16*MICROSECONDS_PER_TICK)
#define MILLISECONDS_PER_MACRO_TICK 6
#define MICROTICKS_PER_MACROTICK 240

#endif // __MICROPY_INCLUDED_LIB_TICKER_H__