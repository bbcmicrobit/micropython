#include "MicroBit.h"
#include "microbitobj.h"
#include "microbitdisplay.h"
#include "microbitmusic.h"

extern "C" {
#include "lib/ticker.h"
#include "filesystem.h"
#include "lib/pwm.h"

    void mp_run(void);
    
    void microbit_button_init(void);
    void microbit_accelerometer_init(void);
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
    microbit_accelerometer_init();

    while (1) {
        mp_run();
    }
}

extern "C" {

void microbit_ticker(void) {

    /** Update compass if it is calibrating, but not if it is still
     *  updating as compass.idleTick() is not reentrant.
     */
    if (uBit.compass.isCalibrating() && !compass_updating) {
        uBit.compass.idleTick();
    }

    compass_up_to_date = false;
    accelerometer_up_to_date = false;

    //update any components in the DAL's systemComponents array
    for (int i = 0; i < MICROBIT_SYSTEM_COMPONENTS; i++) {
        if (uBit.systemTickComponents[i] != NULL) {
            uBit.systemTickComponents[i]->systemTick();
        }
    }

    // Update the display.
    microbit_display_tick();

    // Update the music
    microbit_music_tick();
}

// We need to override this function so that the linker does not pull in
// unnecessary code and static RAM usage for unused system exit functionality.
// There can be large static data structures to store the exit functions.
void __register_exitproc() {
}

void microbit_init(void) {
    uBit.display.disable();
    microbit_display_init();
    microbit_filesystem_init();

    // Start the ticker.
    uBit.systemTicker.detach();
    ticker_init(microbit_ticker);
    ticker_start();
    pwm_start();
}

}
