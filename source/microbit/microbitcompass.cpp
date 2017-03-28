/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George, 2016-2017 Mark Shannon
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
 * OUT OF OR IN CONNread_commandECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

extern "C" {

#include "stdio.h"

#include "lib/ticker.h"
#include "py/runtime.h"
#include "modmicrobit.h"
#include "py/mphal.h"
#include "microbitdisplay.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "py/runtime0.h"
#include "filesystem.h"
#include "microbitobj.h"
#include "microbitmath.h"
#include "math.h"

#define MICROBIT_PIN_COMPASS_DATA_READY          P0_29

/**
  * MAG3110 Register map
  */
#define MAG_DR_STATUS 0x00
#define MAG_OUT_X_MSB 0x01
#define MAG_OUT_X_LSB 0x02
#define MAG_OUT_Y_MSB 0x03
#define MAG_OUT_Y_LSB 0x04
#define MAG_OUT_Z_MSB 0x05
#define MAG_OUT_Z_LSB 0x06
#define MAG_WHOAMI    0x07
#define MAG_SYSMOD    0x08
#define MAG_OFF_X_MSB 0x09
#define MAG_OFF_X_LSB 0x0A
#define MAG_OFF_Y_MSB 0x0B
#define MAG_OFF_Y_LSB 0x0C
#define MAG_OFF_Z_MSB 0x0D
#define MAG_OFF_Z_LSB 0x0E
#define MAG_DIE_TEMP  0x0F
#define MAG_CTRL_REG1 0x10
#define MAG_CTRL_REG2 0x11


typedef struct _persistent_compass_calibration {
    bool calibrated;
    vector values;
} persistent_compass_calibration;

typedef struct _compass_calibration_vector {
    vector coords;
    int16_t directional_sum;
} compass_calibration_vector;

typedef struct _compass_calibration_t {
    bool initialised;
    compass_calibration_vector vectors[27];
} compass_calibration_t;

typedef union _persistent_compass_data {
    /* Padding is to make sure that we fill an entire flash page, since that is the minimum we can erase.
     * The alternative would be to copy the rest of the page into RAM, but we can't rely on having sufficient RAM. */
    int8_t padding[1024];
    persistent_compass_calibration calibration;
} persistent_compass_data;

/* Must be aligned with and fill exactly one flash page. */
static const persistent_compass_data  __attribute__((aligned(1024))) data = {
    0
};

static inline volatile persistent_compass_calibration *get_calibration(void) {
    return (volatile persistent_compass_calibration *)&data.calibration;
}

typedef struct _microbit_compass_obj_t {
    mp_obj_base_t base;
    vector *sample;
} microbit_compass_obj_t;

mp_obj_t microbit_compass_is_calibrated(mp_obj_t self_in) {
    (void)self_in;
    return mp_obj_new_bool(get_calibration()->calibrated);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_is_calibrated_obj, microbit_compass_is_calibrated);

#define COMPASS_ADDRESS 0x1D


static int write_command(uint8_t reg, uint8_t value)
{
    uint8_t command[2];
    command[0] = reg;
    command[1] = value;
    return microbit_i2c_write(&microbit_i2c_obj, COMPASS_ADDRESS, (const char *)command, 2, false);
}

static int read_command(uint8_t reg, uint8_t* buffer, int length)
{
    int err = microbit_i2c_write(&microbit_i2c_obj, COMPASS_ADDRESS, (const char *)&reg, 1, true);
    if (err < 0)
        return err;
    return microbit_i2c_read(&microbit_i2c_obj, COMPASS_ADDRESS, (char *)buffer, length, false);
}

static int microbit_compass_config(void) {
    int err;

    // First, take the device offline, so it can be configured.
    err = write_command(MAG_CTRL_REG1, 0x00);
    if (err < 0)
        return err;

    // Wait for the part to enter standby mode
    while(1)
    {
        // Read the status of the part
        uint8_t result;
        err = read_command(MAG_SYSMOD, &result, 1);
        if (err < 0)
            return err;

        if((result & 0x03) == 0)
            break;

        // According data sheet turn on time is 25ms
        // Turn off time is probably quicker (but it isn't documented)
        nrf_delay_us(10000);
    }

    // Enable automatic reset after each sample;
    err = write_command(MAG_CTRL_REG2, 0xA0);
    if (err < 0)
        return err;

    // Select 10Hz update rate, with oversampling, and enable the device.
    return write_command(MAG_CTRL_REG1, 0x60 | 0x01);

}

void microbit_compass_init(void) {
    microbit_compass_config();
}

static inline int16_t bytes_to_int(uint8_t msb, uint8_t lsb) {
  return (msb << 8) | lsb;
}

static int update(const microbit_compass_obj_t *self) {
    uint8_t data[6];
    int err;
    if (!nrf_gpio_pin_read(MICROBIT_PIN_COMPASS_DATA_READY)) {
        // sample is up to date
        return -1;
    }
    err = read_command(MAG_OUT_X_MSB, data, 6);
    if (err < 0)
        return err;
    self->sample->x = bytes_to_int(data[0], data[1]);
    self->sample->y = bytes_to_int(data[2], data[3]);
    self->sample->z = bytes_to_int(data[4], data[5]);
    return 0;
}

#define calibration_data MP_STATE_PORT(compass_calibration_data)

void microbit_compass_tick(void) {
    /** If compass is calibrating see if sample is on periphery
     *  and record it if it is.
     * As we are an interrupt we can't call update(), so we
     * rely on it being called by whatever is doing the calibration
     */
    compass_calibration_t *data = calibration_data;
    if (data == NULL) {
        return;
    }
    int x = microbit_compass_obj.sample->x;
    int y =  microbit_compass_obj.sample->y;
    int z =  microbit_compass_obj.sample->z;
    if (!data->initialised) {
        // Set all vectors to initial value.
        for (int xscale=-1; xscale<2; xscale++) {
            for (int yscale=-1; yscale<2; yscale++) {
                for (int zscale=-1; zscale<2; zscale++) {
                    int val = x*xscale + y*yscale + z*zscale;
                    uint32_t index = xscale*9+yscale*3+zscale+13;
                    compass_calibration_vector *p = &data->vectors[index];
                    p->coords.x = x;
                    p->coords.y = y;
                    p->coords.z = z;
                    p->directional_sum = val;
                }
            }
        }
        data->initialised = true;
        return;
    }

    for (int xscale=-1; xscale<2; xscale++) {
        for (int yscale=-1; yscale<2; yscale++) {
            for (int zscale=-1; zscale<2; zscale++) {
                int val = x*xscale + y*yscale + z*zscale;
                uint32_t index = xscale*9+yscale*3+zscale+13;
                compass_calibration_vector *p = &data->vectors[index];
                if (val > p->directional_sum) {
                    p->coords.x = x;
                    p->coords.y = y;
                    p->coords.z = z;
                    p->directional_sum = val;
                }
            }
        }
    }
}

static mp_obj_t compass_start_calibrating(microbit_compass_obj_t *self) {
    (void)self;
    compass_calibration_t *data = m_new_obj(compass_calibration_t);
    data->initialised = false;
    calibration_data = data;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_start_calibrating_obj, compass_start_calibrating);


static int compute_mean(compass_calibration_t *data, int valid_samples, vector *result) {
    int sum_x = 0;
    int sum_y = 0;
    int sum_z = 0;
    int count = 0;
    for (int i = 0; i < 27; i++) {
        if ((valid_samples >> i) & 1) {
            vector *sample = &data->vectors[i].coords;
            sum_x += sample->x;
            sum_y += sample->y;
            sum_z += sample->z;
            count += 1;
        }
    }
    result->x = sum_x/count;
    result->y = sum_y/count;
    result->z = sum_z/count;
    return count;
}

static void microbit_compass_save_calibration(bool calibrated, vector *values) {
    persistent_erase_page((const void *)&data);
    persistent_compass_calibration calibration;
    calibration.calibrated = calibrated;
    calibration.values = *values;
    persistent_write_unchecked((const void *)&data, &calibration, sizeof(persistent_compass_calibration));
}

#define MIN_SEPARATION 60

// Returns the number of samples that were used for calibration
static mp_obj_t compass_stop_calibrating(microbit_compass_obj_t *self) {
    (void)self;
    compass_calibration_t *data = calibration_data;
    if (data == NULL) {
        return 0;
    }
    calibration_data = NULL;
    int valid_samples = (((1<<13)-1)<<14)|((1<<13)-1);
    if (!data->initialised) {
        return 0;
    }
    vector mean;
    int sample_count;
    // Discard values that are too close together.
    for (int index = 0; index < 13; index++) {
        if (((valid_samples >> index) & 1) == 0) {
            continue;
        }
        vector *sample = &data->vectors[index].coords;
        uint32_t opposed_index = 26-index;
        vector *opposed = &data->vectors[opposed_index].coords;
        if (vector_distance(sample, opposed) < MIN_SEPARATION) {
            valid_samples &= ~(1<<index);
            valid_samples &= ~(1<<opposed_index);
        }
    }
    // Iteratively discard values too close to mean (and opposing vector).
    float tolerance = 0.0;
    while (tolerance < 0.85) {
        // Take mean value to get (scaled) centre value.
        sample_count = compute_mean(data, valid_samples, &mean);
        for (int xscale=-1; xscale<2; xscale++) {
            for (int yscale=-1; yscale<2; yscale++) {
                for (int zscale=-1; zscale<2; zscale++) {
                    uint32_t index = xscale*9+yscale*3+zscale+13;
                    if (((valid_samples >> index) & 1) == 0) {
                        continue;
                    }
                    vector *sample = &data->vectors[index].coords;
                    uint32_t opposed_index = 26-index;
                    vector *opposed = &data->vectors[opposed_index].coords;
                    vector expected;
                    expected.x = xscale*2000;
                    expected.y = yscale*2000;
                    expected.z = zscale*2000;
                    vector diff;
                    vector_diff(sample, &mean, &diff);
                    vector opposed_diff;
                    vector_diff(opposed, &mean, &opposed_diff);
                    if (vector_cos_angle(&diff, &expected) < tolerance ||
                        vector_cos_angle(&opposed_diff, &expected) > -tolerance) {
                        valid_samples &= ~(1<<index);
                        valid_samples &= ~(1<<opposed_index);
                    }
                }
            }
        }
        tolerance += 0.1;
    }
    sample_count = compute_mean(data, valid_samples, &mean);
    //Save rescaled values
    microbit_compass_save_calibration(true, &mean);
    return mp_obj_new_int(sample_count);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_stop_calibrating_obj, compass_stop_calibrating);

#define PERIMETER_POINTS 12

struct Point
{
    uint8_t x;
    uint8_t y;
};

static const Point perimeter[PERIMETER_POINTS] = {{1,0}, {2,0}, {3,0},
                                                  {4,1}, {4,2}, {4,3},
                                                  {3,4}, {2,4}, {1,4},
                                                  {0,3}, {0,2}, {0,1}};

mp_obj_t microbit_compass_calibrate(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    if (!microbit_display_obj.active) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_RuntimeError, "display must be on to calibrate the compass"));
    }
    compass_start_calibrating(self);
    microbit_display_scroll(&microbit_display_obj, "DRAW A CIRCLE", false);
    while (microbit_display_active_animation()) {
        if (microbit_button_is_pressed(&microbit_button_a_obj) || microbit_button_is_pressed(&microbit_button_b_obj)) {
            microbit_display_clear();
            break;
        }
    }

    int samples = 0;
    int ticks = 0;
    microbit_image_obj_t *img = microbit_image_for_char(' ');
    Point cursor;
    cursor.x = cursor.y = 2;
    bool done[PERIMETER_POINTS];
    for (int i = 0; i < PERIMETER_POINTS; i++) {
        done[i] = false;
    }

    const int PIXEL1_THRESHOLD = 200;
    const int PIXEL2_THRESHOLD = 800;

    while(samples < PERIMETER_POINTS)
    {
        update(&microbit_compass_obj);

        // take a snapshot of the current accelerometer data.
        vector accel;
        microbit_accelerometer_get_values(&microbit_accelerometer_obj, &accel);

        img->greyscale.setPixelValue(cursor.x, cursor.y, 0);
        // Deterine the position of the user controlled pixel on the screen.
        if (accel.x < -PIXEL2_THRESHOLD)
            cursor.x = 0;
        else if (accel.x < -PIXEL1_THRESHOLD)
            cursor.x = 1;
        else if (accel.x > PIXEL2_THRESHOLD)
            cursor.x = 4;
        else if (accel.x > PIXEL1_THRESHOLD)
            cursor.x = 3;
        else
            cursor.x = 2;

        if (accel.y < -PIXEL2_THRESHOLD)
            cursor.y = 0;
        else if (accel.y < -PIXEL1_THRESHOLD)
            cursor.y = 1;
        else if (accel.y > PIXEL2_THRESHOLD)
            cursor.y = 4;
        else if (accel.y > PIXEL1_THRESHOLD)
            cursor.y = 3;
        else
            cursor.y = 2;
         // Flash the current pixel
        if ((ticks & 4) == 0) {
            img->greyscale.setPixelValue(cursor.x, cursor.y, 9);
        }

        for (int i=0; i<PERIMETER_POINTS; i++) {
            if (done[i]) {
                img->greyscale.setPixelValue(perimeter[i].x, perimeter[i].y, 9);
            } else {
                // Test if we need to update the state at the users position.
                if (cursor.x == perimeter[i].x && cursor.y == perimeter[i].y) {
                    done[i] = true;
                    samples++;
                    img->greyscale.setPixelValue(perimeter[i].x, perimeter[i].y, 9);
                }
            }
        }
        microbit_display_show(&microbit_display_obj, img);
        mp_hal_delay_ms(100);
        ticks += 1;
    }
    // Should have enough data by now.
    mp_obj_t res = compass_stop_calibrating(self);

    // Show a smiley to indicate that we're done.
    microbit_display_show(&microbit_display_obj, HAPPY_IMAGE);
    mp_hal_delay_ms(1000);

    microbit_display_clear();
    return res;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_calibrate_obj, microbit_compass_calibrate);

mp_obj_t microbit_compass_clear_calibration(mp_obj_t self_in) {
    (void)self_in;
    vector zero;
    zero.x = zero.y = zero.z = 0;
    microbit_compass_save_calibration(false, &zero);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_clear_calibration_obj, microbit_compass_clear_calibration);

/* Factor to normalize to SI units */
#define SCALE_FACTOR 100

static int get_x(microbit_compass_obj_t *self) {
    return self->sample->x - get_calibration()->values.x;
}

static int get_y(microbit_compass_obj_t *self) {
    return self->sample->y - get_calibration()->values.y;
}

static int get_z(microbit_compass_obj_t *self) {
    return self->sample->z - get_calibration()->values.z;
}


mp_obj_t microbit_compass_heading(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    if (!get_calibration()->calibrated) {
        microbit_compass_calibrate(self_in);
    }
    float x, y;
    vector raw;
    update(self);
    raw.x = get_x(self);
    raw.y = get_y(self);
    raw.z = get_z(self);
    vector acceleration;
    microbit_accelerometer_get_values(&microbit_accelerometer_obj, &acceleration);

    /* Do some vector maths to get horizontal compass vector */
    vector horizontal_x;
    horizontal_x.x = -acceleration.z;
    horizontal_x.y = 0;
    horizontal_x.z = -acceleration.x;
    vector horizontal_y;
    horizontal_y.x = 0;
    horizontal_y.y = -acceleration.z;
    horizontal_y.z = -acceleration.y;
    x = vector_dot_product(&raw, &horizontal_x)/vector_length(&horizontal_x);
    y = vector_dot_product(&raw, &horizontal_y)/vector_length(&horizontal_y);

    int bearing = (360*atan2(-x, -y)) / (2*M_PI);
    if (bearing < 0) {
        bearing += 360;
    }
    return mp_obj_new_int(bearing);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_heading_obj, microbit_compass_heading);

mp_obj_t microbit_compass_get_x(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(get_x(self)*SCALE_FACTOR);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_x_obj, microbit_compass_get_x);

mp_obj_t microbit_compass_get_y(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(get_y(self)*SCALE_FACTOR);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_y_obj, microbit_compass_get_y);

mp_obj_t microbit_compass_get_z(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(get_z(self)*SCALE_FACTOR);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_z_obj, microbit_compass_get_z);

mp_obj_t microbit_compass_get_values(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    update(self);
    mp_obj_tuple_t *tuple = (mp_obj_tuple_t *)mp_obj_new_tuple(3, NULL);
    tuple->items[0] = mp_obj_new_int(get_x(self)*SCALE_FACTOR);
    tuple->items[1] = mp_obj_new_int(get_y(self)*SCALE_FACTOR);
    tuple->items[2] = mp_obj_new_int(get_z(self)*SCALE_FACTOR);
    return tuple;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_values_obj, microbit_compass_get_values);


mp_obj_t microbit_compass_get_calibration(mp_obj_t self_in) {
    (void)self_in;
    mp_obj_tuple_t *tuple = (mp_obj_tuple_t *)mp_obj_new_tuple(3, NULL);
    tuple->items[0] = mp_obj_new_int(get_calibration()->values.x*SCALE_FACTOR);
    tuple->items[1] = mp_obj_new_int(get_calibration()->values.y*SCALE_FACTOR);
    tuple->items[2] = mp_obj_new_int(get_calibration()->values.z*SCALE_FACTOR);
    return tuple;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_calibration_obj, microbit_compass_get_calibration);

mp_obj_t microbit_compass_set_calibration(mp_uint_t n_args, const mp_obj_t *args) {
    (void)n_args;
    vector calibration;
    calibration.x = mp_obj_get_int(args[1])/100;
    calibration.y = mp_obj_get_int(args[2])/100;
    calibration.z = mp_obj_get_int(args[3])/100;
    microbit_compass_save_calibration(true, &calibration);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_compass_set_calibration_obj, 4, 4, microbit_compass_set_calibration);

mp_obj_t microbit_compass_get_field_strength(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(vector_distance(self->sample, (vector *)&get_calibration()->values));
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
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_calibration), (mp_obj_t)&microbit_compass_get_calibration_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_calibration), (mp_obj_t)&microbit_compass_set_calibration_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start_calibrating), (mp_obj_t)&microbit_compass_start_calibrating_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop_calibrating), (mp_obj_t)&microbit_compass_stop_calibrating_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_values), (mp_obj_t)&microbit_compass_get_values_obj },
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
    .stream_p = NULL,
    .bases_tuple = NULL,
    .locals_dict = (mp_obj_dict_t*)&microbit_compass_locals_dict,
};

static vector compass_sample;

const microbit_compass_obj_t microbit_compass_obj = {
    {&microbit_compass_type},
    .sample = &compass_sample
};

}