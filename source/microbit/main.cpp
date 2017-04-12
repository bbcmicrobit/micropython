
#include "microbitobj.h"
#include "microbitdisplay.h"
#include "microbitmusic.h"

extern "C" {
#include "lib/ticker.h"
#include "filesystem.h"
#include "lib/pwm.h"

    void mp_run(void);
    
    void microbit_button_init(void);
    void microbit_compass_init(void);
    void microbit_accelerometer_init(void);
    void microbit_button_tick(void);
    void pwm_init(void);
    void microbit_i2c_init(void);
    void MicroBit_seedRandom(void);
    void microbit_serial_init(void);
}

int main() {
    
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

    microbit_serial_init();
    microbit_i2c_init();
    microbit_button_init();
    microbit_accelerometer_init();
    microbit_compass_init();

    while (1) {
        mp_run();
    }
}

extern "C" {

extern void microbit_compass_tick(void);

void microbit_ticker(void) {

    // Update buttons and pins with touch.
    microbit_button_tick();

    // Update the display.
    microbit_display_tick();

    // Update the music
    microbit_music_tick();

    //Update compass (if calibrating)
    microbit_compass_tick();

}

// We need to override this function so that the linker does not pull in
// unnecessary code and static RAM usage for unused system exit functionality.
// There can be large static data structures to store the exit functions.
void __register_exitproc() {
}

void microbit_init(void) {
    microbit_display_init();
    microbit_filesystem_init();
    microbit_pin_init();
    pwm_init();

    // Start the ticker.
    ticker_init(microbit_ticker);
    ticker_start();
    pwm_start();
    MicroBit_seedRandom();
}

}
