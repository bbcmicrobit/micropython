#include "MicroBit.h"

extern "C" {
    void mp_run(void);

    void microbit_display_tick(void);
}

static void ticker(void) {
    /* Make sure we call the DAL ticker function */
    uBit.systemTick();
    /* Then update Python display */
    microbit_display_tick();
}

void app_main() {
    /*
    // debugging: print memory layout
    extern uint32_t __data_start__, __data_end__;
    extern uint32_t __bss_start__, __bss_end__;
    extern uint32_t __HeapLimit, __StackLimit, __StackTop;
    printf("__data_start__ = %p\r\n", &__data_start__);
    printf("__data_end__   = %p\r\n", &__data_end__);
    printf("__bss_start_ _ = %p\r\n", &__bss_start__);
    printf("__bss_end__    = %p\r\n", &__bss_end__);
    printf("__HeapLimit    = %p\r\n", &__HeapLimit);
    printf("__StackLimit   = %p\r\n", &__StackLimit);
    printf("__StackTop     = %p\r\n", &__StackTop);
    */

    currentFiber->flags |= MICROBIT_FIBER_FLAG_DO_NOT_PAGE;
    
    /* Hijack the DAL system ticker */
    uBit.systemTicker.attach(ticker, MICROBIT_DISPLAY_REFRESH_PERIOD);


    while (1) {
        mp_run();
    }
}
