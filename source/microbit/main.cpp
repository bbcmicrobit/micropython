#include "MicroBit.h"

extern "C" {
    void mp_run(void);

    void microbit_display_event(void);
}

static void event_listener(MicroBitEvent evt) {
    if (evt.value == MICROBIT_DISPLAY_EVT_ANIMATION_COMPLETE) {
        microbit_display_event();
    }
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

    uBit.MessageBus.listen(MICROBIT_ID_DISPLAY,
        MICROBIT_DISPLAY_EVT_ANIMATION_COMPLETE, event_listener,
        MESSAGE_BUS_LISTENER_REENTRANT | MESSAGE_BUS_LISTENER_NONBLOCKING);

    while (1) {
        mp_run();
    }
}
