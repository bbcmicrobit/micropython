#include "microbitdal.h"
#include "microbitobj.h"
#include "microbitdisplay.h"
#include "microbitbutton.h"
#include "modmusic.h"

extern "C" {
#include "lib/ticker.h"
#include "filesystem.h"
#include "lib/pwm.h"

    void mp_run(void);
    void pwm_init(void);
}

MicroBitDisplay ubit_display;
MicroPythonI2C ubit_i2c(I2C_SDA0, I2C_SCL0);
MicroBitAccelerometer ubit_accelerometer(ubit_i2c);
MicroBitCompass ubit_compass(ubit_i2c, ubit_accelerometer);
MicroBitCompassCalibrator ubit_compass_calibrator(ubit_compass, ubit_accelerometer, ubit_display);

int main(void) {
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

    while (1) {
        mp_run();
    }
}

extern "C" {

void microbit_ticker(void) {
    /** Update compass if it is calibrating, but not if it is still
     *  updating as compass.idleTick() is not reentrant.
     */
    if (ubit_compass.isCalibrating() && !compass_updating) {
        ubit_compass.idleTick();
    }

    compass_up_to_date = false;
    accelerometer_up_to_date = false;

    // Update buttons and pins with touch.
    microbit_button_tick();

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
    ubit_display.disable();
    microbit_display_init();
    microbit_filesystem_init();
    microbit_pin_init();
    pwm_init();

    // Start the ticker.
    //uBit.systemTicker.detach(); TODO what to replace with?
    ticker_init(microbit_ticker);
    ticker_start();
    pwm_start();
}

}
