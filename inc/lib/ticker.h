#ifndef __MICROPY_INCLUDED_LIB_TICKER_H__
#define __MICROPY_INCLUDED_LIB_TICKER_H__

/*************************************
 * 33.333kHz (30µs cycle time) ticker.
 ************************************/

#include "nrf.h"

void ticker_init(void);
void ticker_start(void);
void ticker_stop(void);

typedef void (*callback_ptr)(void);
typedef int32_t (*ticker_callback_ptr)(void);

void clear_ticker_callback(uint32_t index);
void set_ticker_callback(uint32_t index, ticker_callback_ptr func, int32_t initial_delay_us);

int set_low_priority_callback(callback_ptr callback, int id);

extern int8_t sound_callback_pending;
void sound_callback(void);

#define CYCLES_PER_MICROSECONDS 16

#define MICROSECONDS_PER_TICK 16
#define CYCLES_PER_TICK (CYCLES_PER_MICROSECONDS*MICROSECONDS_PER_TICK)
// This must be an integer multiple of MICROSECONDS_PER_TICK
#define MICROSECONDS_PER_MACRO_TICK 6000
#define MILLISECONDS_PER_MACRO_TICK 6

#endif // __MICROPY_INCLUDED_LIB_TICKER_H__