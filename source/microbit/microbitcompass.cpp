/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George, 2016 Mark Shannon
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

#include "MicroBit.h"

extern "C" {

#include "lib/ticker.h"
#include "py/runtime.h"
#include "modmicrobit.h"
#include "py/mphal.h"
#include "microbitdisplay.h"

#include "py/runtime0.h"
#include "filesystem.h"
#include "microbitobj.h"

typedef struct _persistent_compass_calibration {
    bool calibrated;
    int x;
    int y;
    int z;
} persistent_compass_calibration;

typedef struct _point {
    int16_t x;
    int16_t y;
    int16_t z;
} point;

typedef struct _compass_calibration_point {
    point coords;
    int16_t directional_sum;
} compass_calibration_point;

typedef struct _compass_calibration_t {
    bool initialised;
    compass_calibration_point points[27];
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
    MicroBitCompass *compass;
} microbit_compass_obj_t;

mp_obj_t microbit_compass_is_calibrated(mp_obj_t self_in) {
    (void)self_in;
    return mp_obj_new_bool(get_calibration()->calibrated);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_is_calibrated_obj, microbit_compass_is_calibrated);

static int samples_taken = 0;

static void update(const microbit_compass_obj_t *self) {
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
        samples_taken++;
    }
}

#define calibration_data MP_STATE_PORT(compass_calibration_data)

void compass_tick(void) {
    /** If compass is calibrating see if sample is on periphery
     *  and record it if it is.
     */
    compass_calibration_t *data = calibration_data;
    if (data == NULL) {
        return;
    }
    update(&microbit_compass_obj);
    // Scale values down to make sure that they fit into 16 bits.
    int x = uBit.compass.getX(RAW) >> 4;
    int y = uBit.compass.getY(RAW) >> 4;
    int z = uBit.compass.getZ(RAW) >> 4;
    if (!data->initialised) {
        // Set all points to initial value.
        for (int xscale=-1; xscale<2; xscale++) {
            for (int yscale=-1; yscale<2; yscale++) {
                for (int zscale=-1; zscale<2; zscale++) {
                    int val = x*xscale + y*yscale + z*zscale;
                    uint32_t index = xscale*9+yscale*3+zscale+13;
                    compass_calibration_point *p = &data->points[index];
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
                compass_calibration_point *p = &data->points[index];
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


static int compute_mean(compass_calibration_t *data, int valid_samples, point *result) {
    int sum_x = 0;
    int sum_y = 0;
    int sum_z = 0;
    int count = 0;
    for (int i = 0; i < 27; i++) {
        if ((valid_samples >> i) & 1) {
            point *sample = &data->points[i].coords;
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

/* Avoid using floating point or division */
static int distance(point *a, point *b) {
    int x = a->x-b->x;
    int y = a->y-b->y;
    int z = a->z-b->z;
    int len_sq = x*x+y*y+z*z;
    int bit = 15;
    for(; bit; bit--) {
        if (1<<(bit<<1) <= len_sq) {
            break;
        }
    }
    int estimate = 1<<bit;
    while (bit) {
        bit--;
        int bigger = estimate + (1<<bit);
        if (bigger*bigger <= len_sq) {
            estimate = bigger;
        }
    }
    return estimate;
}

static int dot_product(point *a, point *b, point *c) {
    int a_x = a->x-c->x;
    int a_y = a->y-c->y;
    int a_z = a->z-c->z;
    int b_x = b->x-c->x;
    int b_y = b->y-c->y;
    int b_z = b->z-c->z;
    return a_x*b_x + a_y*b_y + a_z*b_z;
}

static float cos_angle(point *a, point *b, point *common) {
    int dot = dot_product(a, b, common);
    int alen = distance(a, common);
    int blen = distance(b, common);
    float res = ((float)dot)/alen/blen;
    return res;
}

static void microbit_compass_save_calibration(bool calibrated, int x, int y, int z) {
    persistent_erase_page((const void *)&data);
    persistent_compass_calibration calibration;
    calibration.calibrated = calibrated;
    calibration.x = x;
    calibration.y = y;
    calibration.z = z;
    persistent_write_unchecked((const void *)&data, &calibration, sizeof(persistent_compass_calibration));
}

#define MIN_SEPARATION (6000>>4)

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
    point mean;
    int sample_count;
    // Discard values that are too close together.
    for (int index = 0; index < 13; index++) {
        if (((valid_samples >> index) & 1) == 0) {
            continue;
        }
        point *sample = &data->points[index].coords;
        uint32_t opposed_index = 26-index;
        point *opposed = &data->points[opposed_index].coords;
        if (distance(sample, opposed) < MIN_SEPARATION) {
            valid_samples &= ~(1<<index);
            valid_samples &= ~(1<<opposed_index);
        }
    }
    // Iteratively discard values too close to mean (and opposing point).
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
                    point *sample = &data->points[index].coords;
                    uint32_t opposed_index = 26-index;
                    point *opposed = &data->points[opposed_index].coords;
                    point expected;
                    expected.x = mean.x + xscale*2000;
                    expected.y = mean.y + yscale*2000;
                    expected.z = mean.z + zscale*2000;
                    if (cos_angle(sample, &expected, &mean) < tolerance ||
                        cos_angle(opposed, &expected, &mean) > -tolerance) {
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
    microbit_compass_save_calibration(true, mean.x<<4, mean.y<<4, mean.z<<4);
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

        // take a snapshot of the current accelerometer data.
        int x;
        int y;
        int z;
        microbit_accelerometer_get_values(&microbit_accelerometer_obj, &x, &y, &z);

        img->greyscale.setPixelValue(cursor.x, cursor.y, 0);
        // Deterine the position of the user controlled pixel on the screen.
        if (x < -PIXEL2_THRESHOLD)
            cursor.x = 0;
        else if (x < -PIXEL1_THRESHOLD)
            cursor.x = 1;
        else if (x > PIXEL2_THRESHOLD)
            cursor.x = 4;
        else if (x > PIXEL1_THRESHOLD)
            cursor.x = 3;
        else
            cursor.x = 2;

        if (y < -PIXEL2_THRESHOLD)
            cursor.y = 0;
        else if (y < -PIXEL1_THRESHOLD)
            cursor.y = 1;
        else if (y > PIXEL2_THRESHOLD)
            cursor.y = 4;
        else if (y > PIXEL1_THRESHOLD)
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

    // Show a smiley to indicate that we're done.
    microbit_display_show(&microbit_display_obj, HAPPY_IMAGE);
    mp_hal_delay_ms(1000);

    // Should have enough data by now.
    mp_obj_t res = compass_stop_calibrating(self);
    microbit_display_clear();
    return res;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_calibrate_obj, microbit_compass_calibrate);

mp_obj_t microbit_compass_clear_calibration(mp_obj_t self_in) {
    (void)self_in;
    microbit_compass_save_calibration(false, 0, 0, 0);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_clear_calibration_obj, microbit_compass_clear_calibration);

volatile bool compass_up_to_date = false;
volatile bool compass_updating = false;

static int get_x(microbit_compass_obj_t *self) {
    return self->compass->getX(RAW) - get_calibration()->x;
}

static int get_y(microbit_compass_obj_t *self) {
    return self->compass->getY(RAW) - get_calibration()->y;
}

static int get_z(microbit_compass_obj_t *self) {
    return self->compass->getZ(RAW) - get_calibration()->z;
}

#define PITCH_ADJUST true

mp_obj_t microbit_compass_heading(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    if (!get_calibration()->calibrated) {
        microbit_compass_calibrate(self_in);
    }
    float x, y;
    update(self);
    if (PITCH_ADJUST) {
        uBit.accelerometer.update();
        float raw_x = get_x(self);
        float raw_y = get_y(self);
        float raw_z = get_z(self);
        float roll = uBit.accelerometer.getRollRadians();
        float pitch = uBit.accelerometer.getPitchRadians();

        // Precompute cos and sin of pitch and roll angles to make the calculation a little more efficient.
        float sinroll = sin(roll);
        float cosroll = cos(roll);
        float sinpitch = sin(pitch);
        float cospitch = cos(pitch);

        x = raw_z*sinroll + raw_x*cosroll;
        y = raw_y*cospitch + raw_z*sinpitch;
    } else {
        x = get_x(self);
        y = get_y(self);
    }

    int bearing = (360*atan2(-x, -y)) / (2*PI);
    if (bearing < 0) {
        bearing += 360;
    }
    return mp_obj_new_int(bearing);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_heading_obj, microbit_compass_heading);

mp_obj_t microbit_compass_get_x(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(get_x(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_x_obj, microbit_compass_get_x);

mp_obj_t microbit_compass_get_y(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(get_y(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_y_obj, microbit_compass_get_y);

mp_obj_t microbit_compass_get_z(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    update(self);
    return mp_obj_new_int(get_z(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_z_obj, microbit_compass_get_z);

mp_obj_t microbit_compass_get_values(mp_obj_t self_in) {
    microbit_compass_obj_t *self = (microbit_compass_obj_t*)self_in;
    mp_obj_tuple_t *tuple = (mp_obj_tuple_t *)mp_obj_new_tuple(3, NULL);
    update(self);
    tuple->items[0] = mp_obj_new_int(get_x(self));
    tuple->items[1] = mp_obj_new_int(get_y(self));
    tuple->items[2] = mp_obj_new_int(get_z(self));
    return tuple;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_values_obj, microbit_compass_get_values);


mp_obj_t microbit_compass_get_calibration(mp_obj_t self_in) {
    (void)self_in;
    mp_obj_tuple_t *tuple = (mp_obj_tuple_t *)mp_obj_new_tuple(3, NULL);
    tuple->items[0] = mp_obj_new_int(get_calibration()->x);
    tuple->items[1] = mp_obj_new_int(get_calibration()->y);
    tuple->items[2] = mp_obj_new_int(get_calibration()->z);
    return tuple;
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_compass_get_calibration_obj, microbit_compass_get_calibration);

mp_obj_t microbit_compass_set_calibration(mp_uint_t n_args, const mp_obj_t *args) {
    (void)n_args;
    microbit_compass_save_calibration(true, mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_compass_set_calibration_obj, 4, 4, microbit_compass_set_calibration);

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

const microbit_compass_obj_t microbit_compass_obj = {
    {&microbit_compass_type},
    .compass = &uBit.compass,
};

}