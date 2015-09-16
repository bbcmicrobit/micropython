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
    uBit.MessageBus.listen(MICROBIT_ID_DISPLAY,
        MICROBIT_DISPLAY_EVT_ANIMATION_COMPLETE, event_listener);
    while (1) {
        mp_run();
    }
}
