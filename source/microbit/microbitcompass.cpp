/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George
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

#include "microbit/memory.h"
#include "microbit/microbitdal.h"

extern "C" {

#include "py/runtime.h"
#include "lib/ticker.h"
#include "microbit/modmicrobit.h"

#define COMPASS_CALIBRATION_MAGIC (0xc011ba55)

typedef struct _microbit_compass_obj_t {
    mp_obj_base_t base;
    MicroBitCompass *compass;
} microbit_compass_obj_t;

void microbit_compass_init(void) {
    // load any peristent calibration data if it exists
    uint32_t *persist = (uint32_t*)microbit_compass_calibration_page();
    if (persist[0] == COMPASS_CALIBRATION_MAGIC) {
        CompassSample samp = {(int)persist[1], (int)persist[2], (int)persist[3]};
        ubit_compass.setCalibration(samp);
    }
}

mp_obj_t microbit_compass_is_calibrated(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    return mp_obj_new_bool(self->compass->isCalibrated());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_is_calibrated_obj, microbit_compass_is_calibrated);

mp_obj_t microbit_compass_calibrate(mp_obj_t self_in) {
    // Calibration requires to pass control over to the DAL so it
    // can use the display to collect samples for the calibration.
    // It will do the calibration and then return here.
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    ticker_stop();
    //uBit.systemTicker.attach_us(&uBit, &MicroBit::systemTick, MICROBIT_DEFAULT_TICK_PERIOD * 1000); TODO what to replace with?
    ubit_display.enable();
    self->compass->calibrate();
    ubit_display.disable();
    //uBit.systemTicker.detach(); TODO what to replace with?
    ticker_start();
    microbit_display_init();

    // store the calibration data
    uint32_t *persist = (uint32_t*)microbit_compass_calibration_page();
    CompassSample samp = self->compass->getCalibration();
    uint32_t data[4] = {COMPASS_CALIBRATION_MAGIC, (uint32_t)samp.x, (uint32_t)samp.y, (uint32_t)samp.z};
    persistent_erase_page(persist);
    persistent_write_unchecked(persist, data, sizeof(data));

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_calibrate_obj, microbit_compass_calibrate);

mp_obj_t microbit_compass_clear_calibration(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    self->compass->clearCalibration();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_clear_calibration_obj, microbit_compass_clear_calibration);

volatile bool compass_up_to_date = false;
volatile bool compass_updating = false;

static void update(microbit_compass_obj_t *self) {
    /* The only time it is possible for compass_updating to be true here
     * is if this is called in an interrupt when it is already updating in
     * the main execution thread. This is extremely unlikely, so we just
     * accept that a slightly out-of-date result will be returned
     */
    if (!compass_up_to_date && !compass_updating) {
        compass_updating = true;
        self->compass->idleTick();
        compass_updating = false;
        compass_up_to_date = true;
    }
}

mp_obj_t microbit_compass_heading(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    // Upon calling heading(), the DAL will automatically calibrate the compass
    // if it's not already calibrated.  Since we need to first enable the display
    // for calibration to work, we must check for non-calibration here and call
    // our own calibration function.
    if (!self->compass->isCalibrated()) {
        microbit_compass_calibrate(self_in);
    }
    update(self);
    return mp_obj_new_int(self->compass->heading());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_heading_obj, microbit_compass_heading);

mp_obj_t microbit_compass_get_x(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(self->compass->getX());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_x_obj, microbit_compass_get_x);

mp_obj_t microbit_compass_get_y(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(self->compass->getY());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_y_obj, microbit_compass_get_y);

mp_obj_t microbit_compass_get_z(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(self->compass->getZ());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_z_obj, microbit_compass_get_z);

mp_obj_t microbit_compass_get_field_strength(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(self->compass->getFieldStrength());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_field_strength_obj, microbit_compass_get_field_strength);

STATIC const mp_map_elem_t microbit_compass_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_heading), (mp_obj_t)&microbit_compass_heading_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_calibrated), (mp_obj_t)&microbit_compass_is_calibrated_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_calibrate), (mp_obj_t)&microbit_compass_calibrate_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_clear_calibration), (mp_obj_t)&microbit_compass_clear_calibration_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_x), (mp_obj_t)&microbit_compass_get_x_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_y), (mp_obj_t)&microbit_compass_get_y_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_z), (mp_obj_t)&microbit_compass_get_z_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_field_strength), (mp_obj_t)&microbit_compass_get_field_strength_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_compass_locals_dict, microbit_compass_locals_dict_table);

STATIC const mp_obj_type_t microbit_compass_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitCompass,
    .print = NULL,
    .make_new = NULL,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = NULL,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .protocol = NULL,
    .parent = NULL,
    .locals_dict = (mp_obj_dict_t*)&microbit_compass_locals_dict,
};

const microbit_compass_obj_t microbit_compass_obj = {
    {&microbit_compass_type},
    .compass = &ubit_compass,
};

}
