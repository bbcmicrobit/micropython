#include "MicroBit.h"
#include "microbitobj.h"
#include "microbitdisplay.h"
#include "microbitmusic.h"

extern "C" {
    void mp_run(void);
    
    void microbit_button_init(void);
}

void app_main() {
    
    // debugging: print memory layout
    /*
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

    
    microbit_button_init();

    while (1) {
        mp_run();
    }
}

static void ticker(void) {

    // increment our real-time counter.
    ticks += FIBER_TICK_PERIOD_MS;

    /** Update compass if it is calibrating, but not if it is still
     *  updating as compass.idleTick() is not reentrant.
     */
    if (uBit.compass.isCalibrating() && !compass_updating) {
        uBit.compass.idleTick();
    }

    compass_up_to_date = false;
    accelerometer_up_to_date = false;

    // Update buttons
    uBit.buttonA.systemTick();
    uBit.buttonB.systemTick();

    // Update the display.
    microbit_display_tick();

    // Update the music
    microbit_music_tick();
}

extern "C" {

void microbit_init(void) {
    microbit_display_init();

    // Hijack the DAL system ticker.
    uBit.systemTicker.attach(ticker, MICROBIT_DISPLAY_REFRESH_PERIOD);
}

}
