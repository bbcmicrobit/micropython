/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George, 2017 Mark Shannon
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

#include "nrf_gpio.h"

extern "C" {

#include "py/runtime.h"
#include "modmicrobit.h"
#include "microbitobj.h"
#include "microbitmath.h"


enum Gesture
{
    GESTURE_NONE,
    GESTURE_UP,
    GESTURE_DOWN,
    GESTURE_LEFT,
    GESTURE_RIGHT,
    GESTURE_FACE_UP,
    GESTURE_FACE_DOWN,
    GESTURE_SHAKE,
    /** These gestures are never used and are not documented
     * They are retained for backwards compatibility */
    GESTURE_3G,
    GESTURE_6G,
    GESTURE_8G
};


#define MMA8653_DEFAULT_ADDR    0x3A

#define MICROBIT_PIN_ACCEL_DATA_READY 28

/*
 * MMA8653 Register map (partial)
 */
#define MMA8653_STATUS          0x00
#define MMA8653_OUT_X_MSB       0x01
#define MMA8653_WHOAMI          0x0D
#define MMA8653_XYZ_DATA_CFG    0x0E
#define MMA8653_CTRL_REG1       0x2A
#define MMA8653_CTRL_REG2       0x2B
#define MMA8653_CTRL_REG3       0x2C
#define MMA8653_CTRL_REG4       0x2D
#define MMA8653_CTRL_REG5       0x2E


#define GESTURE_LIST_SIZE (8)

typedef struct  _microbit_gestures_t {
    volatile uint16_t state;                    // 1 bit per gesture
    volatile uint8_t list_cur;                 // index into gesture_list
    volatile uint8_t list[GESTURE_LIST_SIZE]; // list of pending gestures, 4-bits per element
} microbit_gestures_t;

typedef struct  _microbit_accelerometer_data_t {
    microbit_vector3_t sample;
    uint8_t hysteresis; // the number of ticks that the instantaneous gesture has been stable.
    Gesture current_gesture;
    microbit_gestures_t gestures;
} microbit_accelerometer_data_t;

typedef struct _microbit_accelerometer_obj_t {
    mp_obj_base_t base;
    microbit_accelerometer_data_t *data;
} microbit_accelerometer_obj_t;



static int read_command(uint8_t reg, uint8_t* buffer, int length)
{
    int err = microbit_i2c_write(&microbit_i2c_obj, MMA8653_DEFAULT_ADDR, (const char *)&reg, 1, true);
    if (err < 0)
        return err;
    return microbit_i2c_read(&microbit_i2c_obj, MMA8653_DEFAULT_ADDR, (char *)buffer, length, false);
}

static int write_command(uint8_t reg, uint8_t value)
{
    uint8_t command[2];
    command[0] = reg;
    command[1] = value;

    return microbit_i2c_write(&microbit_i2c_obj, MMA8653_DEFAULT_ADDR, (const char *)command, 2, false);
}

static int16_t normalise(int8_t msb, int8_t lsb) {
    return ((msb<<2) + ((lsb>>6)&3))<<4;
}

static uint32_t integrated_jerk;

#define SHAKE_THRESHOLD 1000

#define TILT_THRESHOLD 800

static Gesture gesture_snapshot(const microbit_accelerometer_obj_t *self) {
    if (integrated_jerk > SHAKE_THRESHOLD)
        return GESTURE_SHAKE;


    // Determine our posture.
    if (microbit_accelerometer_obj.data->sample.x < -TILT_THRESHOLD)
        return GESTURE_LEFT;
    if (microbit_accelerometer_obj.data->sample.x > TILT_THRESHOLD)
        return GESTURE_RIGHT;
    if (microbit_accelerometer_obj.data->sample.y < -TILT_THRESHOLD)
        return GESTURE_DOWN;
    if (microbit_accelerometer_obj.data->sample.y > TILT_THRESHOLD)
        return GESTURE_UP;
    if (microbit_accelerometer_obj.data->sample.z < -TILT_THRESHOLD)
        return GESTURE_FACE_UP;
    if (microbit_accelerometer_obj.data->sample.z > TILT_THRESHOLD)
        return GESTURE_FACE_DOWN;
    return self->data->current_gesture;
}

static void update_gesture_list(const microbit_accelerometer_obj_t *self, Gesture latest) {
    microbit_gestures_t *gestures = &self->data->gestures;
    gestures->state |= 1 << latest;
    if (gestures->list_cur < 2 * GESTURE_LIST_SIZE) {
        gestures->list[gestures->list_cur >> 1] |= latest << (4 * (gestures->list_cur & 1));
        ++gestures->list_cur;
    }
}

#define GESTURE_HYSTERESIS 8

static int update(const microbit_accelerometer_obj_t *self) {
    int8_t data[6];
    int err;

    err = read_command(MMA8653_OUT_X_MSB, (uint8_t *)data, 6);
    if (err < 0) {
        return err;
    }
    microbit_vector3_t reading;
    reading.x = normalise(data[0], data[1]);
    reading.y = normalise(data[2], data[3]);
    reading.z = normalise(data[4], data[5]);

    /* When the window function of |jerk| exceeds a threshold, we take that
     * to signify shaking. This threshold should be such that it is not exceeded by a brief drop and catch.
     * https://en.wikipedia.org/wiki/Jerk_(physics)
     */
    uint32_t instanteous_jerk = microbit_vector3_distance(&reading, &self->data->sample);
    integrated_jerk = instanteous_jerk + ((integrated_jerk*7)>>8);
    self->data->sample = reading;

    Gesture now = gesture_snapshot(self);
    if (now == self->data->current_gesture) {
        self->data->hysteresis = GESTURE_HYSTERESIS;
    } else {
        self->data->hysteresis--;
        if (self->data->hysteresis == 0) {
            self->data->current_gesture = now;
            update_gesture_list(self, now);
            self->data->hysteresis = GESTURE_HYSTERESIS;
        }
    }
    return 0;
}

mp_obj_t microbit_accelerometer_read_register(mp_obj_t self_in, mp_obj_t reg_in) {
    (void)self_in;
    uint8_t data;
    int reg = mp_obj_get_int(reg_in);
    int err = read_command(reg, &data, 1);
    if (err < 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_OSError, "I2C read failed with error code %d", err));
    }
    return mp_obj_new_int(data);
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_accelerometer_read_register_obj, microbit_accelerometer_read_register);


void microbit_accelerometer_init(void) {

    // First place the device into standby mode, so it can be configured.
    int err = write_command(MMA8653_CTRL_REG1, 0x00);
    if (err < 0)
        return;

    // Enable high precision mode. This consumes a bit more power, but still only 184 uA!
    err = write_command(MMA8653_CTRL_REG2, 0x10);
    if (err < 0)
        return;

    // Enable the INT1 interrupt pin.
    err = write_command(MMA8653_CTRL_REG4, 0x01);
    if (err < 0)
        return;

    // Select the DATA_READY event source to be routed to INT1
    err = write_command(MMA8653_CTRL_REG5, 0x01);
    if (err < 0)
        return;

    // Configure for 8g range.
    err = write_command(MMA8653_XYZ_DATA_CFG, 2);
    if (err < 0)
        return;

    // Bring the device back online, with 10bit wide samples at 50Hz.
    err = write_command(MMA8653_CTRL_REG1, 0x20 | 0x01);
    if (err < 0)
        return;

    microbit_accelerometer_obj.data->hysteresis = GESTURE_HYSTERESIS;

}

static mp_obj_t microbit_accelerometer_get_x(mp_obj_t self_in) {
    microbit_accelerometer_obj_t *self = (microbit_accelerometer_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(self->data->sample.x);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_accelerometer_get_x_obj, microbit_accelerometer_get_x);

static mp_obj_t microbit_accelerometer_get_y(mp_obj_t self_in) {
    microbit_accelerometer_obj_t *self = (microbit_accelerometer_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(self->data->sample.y);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_accelerometer_get_y_obj, microbit_accelerometer_get_y);

static mp_obj_t microbit_accelerometer_get_z(mp_obj_t self_in) {
    microbit_accelerometer_obj_t *self = (microbit_accelerometer_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(self->data->sample.z);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_accelerometer_get_z_obj, microbit_accelerometer_get_z);

void microbit_accelerometer_get_values(const microbit_accelerometer_obj_t *self, microbit_vector3_t *values) {
    update(self);
    /* Make sure that we get a consistent set of values */
    __disable_irq();
    values[0] = self->data->sample;
    __enable_irq();
}

static mp_obj_t microbit_accelerometer_get_values_func(mp_obj_t self_in) {
    microbit_accelerometer_obj_t *self = (microbit_accelerometer_obj_t*)self_in;
    microbit_vector3_t vals;
    microbit_accelerometer_get_values(self, &vals);
    mp_obj_tuple_t *tuple = (mp_obj_tuple_t *)mp_obj_new_tuple(3, NULL);
    tuple->items[0] = mp_obj_new_int(vals.x);
    tuple->items[1] = mp_obj_new_int(vals.y);
    tuple->items[2] = mp_obj_new_int(vals.z);
    return tuple;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_accelerometer_get_values_obj, microbit_accelerometer_get_values_func);


STATIC const qstr gesture_name_map[] = {
    [GESTURE_NONE] = MP_QSTR_NULL,
    [GESTURE_UP] = MP_QSTR_up,
    [GESTURE_DOWN] = MP_QSTR_down,
    [GESTURE_LEFT] = MP_QSTR_left,
    [GESTURE_RIGHT] = MP_QSTR_right,
    [GESTURE_FACE_UP] = MP_QSTR_face_space_up,
    [GESTURE_FACE_DOWN] = MP_QSTR_face_space_down,
    [GESTURE_SHAKE] = MP_QSTR_shake,
    /** These gestures are never used and are not documented.
     * They are retained for backwards compatibility */
    [GESTURE_3G] = MP_QSTR_3g,
    [GESTURE_6G] = MP_QSTR_6g,
    [GESTURE_8G] = MP_QSTR_8g
};



static Gesture gesture_from_obj(mp_obj_t gesture_in) {
    qstr gesture = mp_obj_str_get_qstr(gesture_in);
    for (uint i = 0; i < MP_ARRAY_SIZE(gesture_name_map); ++i) {
        if (gesture == gesture_name_map[i]) {
            return (Gesture)i;
        }
    }
    nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "invalid gesture"));
}

mp_obj_t microbit_accelerometer_current_gesture(mp_obj_t self_in) {
    microbit_accelerometer_obj_t *self = (microbit_accelerometer_obj_t*)self_in;
    update(self);
    return MP_OBJ_NEW_QSTR(gesture_name_map[self->data->current_gesture]);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_accelerometer_current_gesture_obj, microbit_accelerometer_current_gesture);

mp_obj_t microbit_accelerometer_is_gesture(mp_obj_t self_in, mp_obj_t gesture_in) {
    microbit_accelerometer_obj_t *self = (microbit_accelerometer_obj_t*)self_in;
    update(self);
    Gesture gesture = gesture_from_obj(gesture_in);
    return mp_obj_new_bool(self->data->current_gesture == gesture);
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_accelerometer_is_gesture_obj, microbit_accelerometer_is_gesture);

mp_obj_t microbit_accelerometer_was_gesture(mp_obj_t self_in, mp_obj_t gesture_in) {
    microbit_accelerometer_obj_t *self = (microbit_accelerometer_obj_t*)self_in;
    update(self);
    Gesture gesture = gesture_from_obj(gesture_in);
    microbit_gestures_t *gestures = &self->data->gestures;
    int result = gestures->state & (1 << gesture);
    gestures->state &= (~(1 << gesture));
    gestures->list_cur = 0;
    return mp_obj_new_bool(result);
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_accelerometer_was_gesture_obj, microbit_accelerometer_was_gesture);

mp_obj_t microbit_accelerometer_get_gestures(mp_obj_t self_in) {
    microbit_accelerometer_obj_t *self = (microbit_accelerometer_obj_t*)self_in;
    update(self);
    if (self->data->gestures.list_cur == 0) {
        return mp_const_empty_tuple;
    }
    mp_obj_tuple_t *o = (mp_obj_tuple_t*)mp_obj_new_tuple(self->data->gestures.list_cur, NULL);
    for (uint i = 0; i < self->data->gestures.list_cur; ++i) {
        uint gesture = (self->data->gestures.list[i >> 1] >> (4 * (i & 1))) & 0x0f;
        o->items[i] = MP_OBJ_NEW_QSTR(gesture_name_map[gesture]);
    }
    self->data->gestures.list_cur = 0;
    return o;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_accelerometer_get_gestures_obj, microbit_accelerometer_get_gestures);


STATIC const mp_map_elem_t microbit_accelerometer_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_x), (mp_obj_t)&microbit_accelerometer_get_x_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_y), (mp_obj_t)&microbit_accelerometer_get_y_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_z), (mp_obj_t)&microbit_accelerometer_get_z_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_values), (mp_obj_t)&microbit_accelerometer_get_values_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read_register), (mp_obj_t)&microbit_accelerometer_read_register_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_current_gesture), (mp_obj_t)&microbit_accelerometer_current_gesture_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_gesture), (mp_obj_t)&microbit_accelerometer_is_gesture_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_was_gesture), (mp_obj_t)&microbit_accelerometer_was_gesture_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_gestures), (mp_obj_t)&microbit_accelerometer_get_gestures_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_accelerometer_locals_dict, microbit_accelerometer_locals_dict_table);

const mp_obj_type_t microbit_accelerometer_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitAccelerometer,
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
    .stream_p = NULL,
    .bases_tuple = NULL,
    .locals_dict = (mp_obj_dict_t*)&microbit_accelerometer_locals_dict,
};

static microbit_accelerometer_data_t _data;

const microbit_accelerometer_obj_t microbit_accelerometer_obj = {
    {&microbit_accelerometer_type},
    .data = &_data
};

}
