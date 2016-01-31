
#include "nrf.h"

void ticker_init(void);
void ticker_start(void);
void ticker_stop(void);

typedef int32_t (*sound_func_ptr)(int32_t previous_value);
typedef int32_t (*ticker_callback_ptr)(void);

void set_sound_callback(sound_func_ptr func);
void set_display_callback(ticker_callback_ptr func, int32_t initial_delay);
void set_other_callback(ticker_callback_ptr func, int32_t initial_delay);

void clear_display_callback(void);
void clear_other_callback(void);

#define MICROSECONDS_PER_TICK 25
#define CYCLE_PER_TICK (16*MICROSECONDS_PER_TICK)
#define TICKS_PER_MACRO_TICK (6000/MICROSECONDS_PER_TICK)

