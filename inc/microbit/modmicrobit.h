/*
 * This file is part of the Micro Python project, http://micropython.org/
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
#ifndef __MICROPY_INCLUDED_MICROBIT_MODMICROBIT_H__
#define __MICROPY_INCLUDED_MICROBIT_MODMICROBIT_H__

#include "py/objtuple.h"

extern const mp_obj_type_t microbit_ad_pin_type;
extern const mp_obj_type_t microbit_dig_pin_type;
extern const mp_obj_type_t microbit_touch_pin_type;

extern const struct _microbit_pin_obj_t microbit_p0_obj;
extern const struct _microbit_pin_obj_t microbit_p1_obj;
extern const struct _microbit_pin_obj_t microbit_p2_obj;
extern const struct _microbit_pin_obj_t microbit_p3_obj;
extern const struct _microbit_pin_obj_t microbit_p4_obj;
extern const struct _microbit_pin_obj_t microbit_p5_obj;
extern const struct _microbit_pin_obj_t microbit_p6_obj;
extern const struct _microbit_pin_obj_t microbit_p7_obj;
extern const struct _microbit_pin_obj_t microbit_p8_obj;
extern const struct _microbit_pin_obj_t microbit_p9_obj;
extern const struct _microbit_pin_obj_t microbit_p10_obj;
extern const struct _microbit_pin_obj_t microbit_p11_obj;
extern const struct _microbit_pin_obj_t microbit_p12_obj;
extern const struct _microbit_pin_obj_t microbit_p13_obj;
extern const struct _microbit_pin_obj_t microbit_p14_obj;
extern const struct _microbit_pin_obj_t microbit_p15_obj;
extern const struct _microbit_pin_obj_t microbit_p16_obj;
extern const struct _microbit_pin_obj_t microbit_p19_obj;
extern const struct _microbit_pin_obj_t microbit_p20_obj;

extern const mp_obj_type_t microbit_const_image_type;
extern const struct _monochrome_5by5_t microbit_const_image_heart_obj;
extern const struct _monochrome_5by5_t microbit_const_image_heart_small_obj;
extern const struct _monochrome_5by5_t microbit_const_image_happy_obj;
extern const struct _monochrome_5by5_t microbit_const_image_smile_obj;
extern const struct _monochrome_5by5_t microbit_const_image_sad_obj;
extern const struct _monochrome_5by5_t microbit_const_image_confused_obj;
extern const struct _monochrome_5by5_t microbit_const_image_angry_obj;
extern const struct _monochrome_5by5_t microbit_const_image_asleep_obj;
extern const struct _monochrome_5by5_t microbit_const_image_surprised_obj;
extern const struct _monochrome_5by5_t microbit_const_image_yes_obj;
extern const struct _monochrome_5by5_t microbit_const_image_no_obj;
extern const struct _monochrome_5by5_t microbit_const_image_clock12_obj;
extern const struct _monochrome_5by5_t microbit_const_image_clock1_obj;
extern const struct _monochrome_5by5_t microbit_const_image_clock2_obj;
extern const struct _monochrome_5by5_t microbit_const_image_clock3_obj;
extern const struct _monochrome_5by5_t microbit_const_image_clock4_obj;
extern const struct _monochrome_5by5_t microbit_const_image_clock5_obj;
extern const struct _monochrome_5by5_t microbit_const_image_clock6_obj;
extern const struct _monochrome_5by5_t microbit_const_image_clock7_obj;
extern const struct _monochrome_5by5_t microbit_const_image_clock8_obj;
extern const struct _monochrome_5by5_t microbit_const_image_clock9_obj;
extern const struct _monochrome_5by5_t microbit_const_image_clock10_obj;
extern const struct _monochrome_5by5_t microbit_const_image_clock11_obj;
extern const struct _monochrome_5by5_t microbit_const_image_arrow_n_obj;
extern const struct _monochrome_5by5_t microbit_const_image_arrow_ne_obj;
extern const struct _monochrome_5by5_t microbit_const_image_arrow_e_obj;
extern const struct _monochrome_5by5_t microbit_const_image_arrow_se_obj;
extern const struct _monochrome_5by5_t microbit_const_image_arrow_s_obj;
extern const struct _monochrome_5by5_t microbit_const_image_arrow_sw_obj;
extern const struct _monochrome_5by5_t microbit_const_image_arrow_w_obj;
extern const struct _monochrome_5by5_t microbit_const_image_arrow_nw_obj;

extern const mp_obj_type_t microbit_image_type;

extern const mp_obj_type_t microbit_accelerometer_type;
extern const struct _microbit_accelerometer_obj_t microbit_accelerometer_obj;

extern const struct _microbit_display_obj_t microbit_display_obj;
extern const struct _microbit_button_obj_t microbit_button_a_obj;
extern const struct _microbit_button_obj_t microbit_button_b_obj;
extern const struct _microbit_compass_obj_t microbit_compass_obj;
extern const struct _microbit_i2c_obj_t microbit_i2c_obj;
extern struct _microbit_uart_obj_t microbit_uart_obj;
extern struct _microbit_music_obj_t microbit_music_obj;

MP_DECLARE_CONST_FUN_OBJ(microbit_reset_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_sleep_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_random_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_running_time_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_panic_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_accelerometer_get_x_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_accelerometer_get_y_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_accelerometer_get_z_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_button_is_pressed_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_compass_is_calibrated_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_compass_calibrate_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_compass_is_calibrating_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_compass_clear_calibration_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_compass_get_x_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_compass_get_y_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_compass_get_z_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_display_print_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_display_scroll_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_display_clear_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_display_animate_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_display_get_pixel_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_display_get_pixel_raw_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_display_set_pixel_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_display_set_pixel_raw_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_pin_read_digital_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_pin_write_digital_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_pin_read_analog_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_pin_write_analog_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_pin_is_touched_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_pin_set_analog_period_obj);
MP_DECLARE_CONST_FUN_OBJ(microbit_pin_set_analog_period_microseconds_obj);

extern const mp_obj_module_t microbit_module;

#endif // __MICROPY_INCLUDED_MICROBIT_MODMICROBIT_H__
