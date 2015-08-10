#include "MicroBit.h"

extern "C" {
    void mp_run(void);
}

void app_main()
{
    while (1) {
        uBit.display.print('P');
        uBit.sleep(100);
        uBit.display.print(' ');
        mp_run();
    }
}
