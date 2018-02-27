/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "EventModel.h"
#include "microbit/microbitdal.h"
#include "microbit/modmicrobit.h"

extern "C" {
    extern void microbit_accelerometer_event_handler(const MicroBitEvent*);
}

class MicroPythonEventHandler : public EventModel {
public:
    MicroPythonEventHandler();

    virtual int send(MicroBitEvent evt);
};

// Create a static instance of our custom event handler
static MicroPythonEventHandler event_handler;

MicroPythonEventHandler::MicroPythonEventHandler() {
    // We take full control of the event bus
    EventModel::defaultEventBus = this;
}

int MicroPythonEventHandler::send(MicroBitEvent evt) {
    // Dispatch the event to the relevant component
    switch (evt.source) {
        case MICROBIT_ID_GESTURE:
            microbit_accelerometer_event_handler(&evt);
            break;

        case MICROBIT_ID_COMPASS:
            if (evt.value == MICROBIT_COMPASS_EVT_CALIBRATE) {
                ubit_compass_calibrator.calibrate(evt);
            }
            break;

        default:
            // Ignore this event
            break;
    }

    return MICROBIT_OK;
}
