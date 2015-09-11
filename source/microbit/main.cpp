#include "MicroBit.h"

extern "C" {
    void mp_run(void);
}

void app_main() {
    while (1) {
        mp_run();
    }
}
