/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2017 Damien P. George
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
#ifndef MICROPY_INCLUDED_MICROBIT_MODMICROBIT_H
#define MICROPY_INCLUDED_MICROBIT_MODMICROBIT_H

#include "py/obj.h"

/****************************************************************/
// microbit.pin objects

/* Leave 0 to mean default mode. */
#define MODE_UNUSED 1
#define MODE_READ_DIGITAL 2
#define MODE_WRITE_DIGITAL 3
#define MODE_DISPLAY 4
#define MODE_BUTTON 5
#define MODE_MUSIC 6
#define MODE_AUDIO_PLAY 7
#define MODE_TOUCH 8
#define MODE_I2C 9
#define MODE_SPI 10
#define MODE_WRITE_ANALOG 11

#define microbit_pin_mode_unused        (&microbit_pinmodes[MODE_UNUSED])
#define microbit_pin_mode_write_analog  (&microbit_pinmodes[MODE_WRITE_ANALOG])
#define microbit_pin_mode_read_digital  (&microbit_pinmodes[MODE_READ_DIGITAL])
#define microbit_pin_mode_write_digital (&microbit_pinmodes[MODE_WRITE_DIGITAL])
#define microbit_pin_mode_display       (&microbit_pinmodes[MODE_DISPLAY])
#define microbit_pin_mode_button        (&microbit_pinmodes[MODE_BUTTON])
#define microbit_pin_mode_music         (&microbit_pinmodes[MODE_MUSIC])
#define microbit_pin_mode_audio_play    (&microbit_pinmodes[MODE_AUDIO_PLAY])
#define microbit_pin_mode_touch         (&microbit_pinmodes[MODE_TOUCH])
#define microbit_pin_mode_i2c           (&microbit_pinmodes[MODE_I2C])
#define microbit_pin_mode_spi           (&microbit_pinmodes[MODE_SPI])

typedef struct _microbit_pin_obj_t {
    mp_obj_base_t base;
    uint8_t number; // The pin number on microbit board
    uint8_t name; // The pin number in the GPIO port.
    uint8_t initial_mode;
} microbit_pin_obj_t;

typedef void(*release_func)(const microbit_pin_obj_t *pin);

typedef struct _pinmode {
    qstr name;
    release_func release; /* Call this function to release pin */
} microbit_pinmode_t;

extern const microbit_pinmode_t microbit_pinmodes[];

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

void microbit_pin_init(void);

const microbit_pin_obj_t *microbit_obj_get_pin(mp_obj_t o);
uint8_t microbit_obj_get_pin_name(mp_obj_t o);

// Release pin for use by other modes. Safe to call in an interrupt.
// If pin is NULL or pin already unused, then this is a no-op
void microbit_obj_pin_free(const microbit_pin_obj_t *pin);

// Acquire pin (causing analog/digital modes to release) for mode.
// If pin is already in specified mode, this is a no-op and returns "false".
// Otherwise if the acquisition succeeds then it returns "true".
// Not safe to call in an interrupt as it may raise if pin can't be acquired.
bool microbit_obj_pin_acquire(const microbit_pin_obj_t *pin, const microbit_pinmode_t *mode);

const microbit_pinmode_t *microbit_pin_get_mode(const microbit_pin_obj_t *pin);
bool microbit_obj_pin_can_be_acquired(const microbit_pin_obj_t *pin);
void pinmode_error(const microbit_pin_obj_t *pin);

bool microbit_pin_high_debounced(microbit_pin_obj_t *pin);

/****************************************************************/
// microbit.Image class

typedef union _microbit_image_obj_t microbit_image_obj_t;

extern const mp_obj_type_t microbit_const_image_type;
extern const mp_obj_type_t microbit_image_type;

extern const struct _monochrome_5by5_t microbit_const_image_heart_obj;
extern const struct _monochrome_5by5_t microbit_const_image_heart_small_obj;
extern const struct _monochrome_5by5_t microbit_const_image_happy_obj;
extern const struct _monochrome_5by5_t microbit_const_image_smile_obj;
extern const struct _monochrome_5by5_t microbit_const_image_sad_obj;
extern const struct _monochrome_5by5_t microbit_const_image_confused_obj;
extern const struct _monochrome_5by5_t microbit_const_image_angry_obj;
extern const struct _monochrome_5by5_t microbit_const_image_asleep_obj;
extern const struct _monochrome_5by5_t microbit_const_image_surprised_obj;
extern const struct _monochrome_5by5_t microbit_const_image_silly_obj;
extern const struct _monochrome_5by5_t microbit_const_image_fabulous_obj;
extern const struct _monochrome_5by5_t microbit_const_image_meh_obj;
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
extern const struct _monochrome_5by5_t microbit_const_image_triangle_obj;
extern const struct _monochrome_5by5_t microbit_const_image_triangle_left_obj;
extern const struct _monochrome_5by5_t microbit_const_image_chessboard_obj;
extern const struct _monochrome_5by5_t microbit_const_image_diamond_obj;
extern const struct _monochrome_5by5_t microbit_const_image_diamond_small_obj;
extern const struct _monochrome_5by5_t microbit_const_image_square_obj;
extern const struct _monochrome_5by5_t microbit_const_image_square_small_obj;
extern const struct _monochrome_5by5_t microbit_const_image_rabbit;
extern const struct _monochrome_5by5_t microbit_const_image_cow;
extern const struct _monochrome_5by5_t microbit_const_image_music_crotchet_obj;
extern const struct _monochrome_5by5_t microbit_const_image_music_quaver_obj;
extern const struct _monochrome_5by5_t microbit_const_image_music_quavers_obj;
extern const struct _monochrome_5by5_t microbit_const_image_pitchfork_obj;
extern const struct _monochrome_5by5_t microbit_const_image_xmas_obj;
extern const struct _monochrome_5by5_t microbit_const_image_pacman_obj;
extern const struct _monochrome_5by5_t microbit_const_image_target_obj;
extern const struct _mp_obj_tuple_t microbit_const_image_all_clocks_tuple_obj;
extern const struct _mp_obj_tuple_t microbit_const_image_all_arrows_tuple_obj;
extern const struct _monochrome_5by5_t microbit_const_image_tshirt_obj;
extern const struct _monochrome_5by5_t microbit_const_image_rollerskate_obj;
extern const struct _monochrome_5by5_t microbit_const_image_duck_obj;
extern const struct _monochrome_5by5_t microbit_const_image_house_obj;
extern const struct _monochrome_5by5_t microbit_const_image_tortoise_obj;
extern const struct _monochrome_5by5_t microbit_const_image_butterfly_obj;
extern const struct _monochrome_5by5_t microbit_const_image_stickfigure_obj;
extern const struct _monochrome_5by5_t microbit_const_image_ghost_obj;
extern const struct _monochrome_5by5_t microbit_const_image_sword_obj;
extern const struct _monochrome_5by5_t microbit_const_image_giraffe_obj;
extern const struct _monochrome_5by5_t microbit_const_image_skull_obj;
extern const struct _monochrome_5by5_t microbit_const_image_umbrella_obj;
extern const struct _monochrome_5by5_t microbit_const_image_snake_obj;

/****************************************************************/
// microbit.display object

typedef struct _microbit_display_obj_t microbit_display_obj_t;

extern microbit_display_obj_t microbit_display_obj;

void microbit_display_init(void);
void microbit_display_tick(void);

void microbit_display_clear(void);
mp_int_t microbit_display_get_pixel(microbit_display_obj_t *display, mp_int_t x, mp_int_t y);
void microbit_display_set_pixel(microbit_display_obj_t *display, mp_int_t x, mp_int_t y, mp_int_t val);
void microbit_display_show(microbit_display_obj_t *display, microbit_image_obj_t *image);
void microbit_display_animate(microbit_display_obj_t *display, mp_obj_t iterable, mp_int_t delay, bool clear, bool wait);
void microbit_display_scroll(microbit_display_obj_t *display, const char* str);

/****************************************************************/
// microbit.compass object

extern volatile bool compass_up_to_date;
extern volatile bool compass_updating;

extern const struct _microbit_compass_obj_t microbit_compass_obj;

void microbit_compass_init(void);

/****************************************************************/
// microbit.accelerometer object

extern volatile bool accelerometer_up_to_date;
extern volatile bool accelerometer_updating;

extern const mp_obj_type_t microbit_accelerometer_type;
extern const struct _microbit_accelerometer_obj_t microbit_accelerometer_obj;

//void microbit_accelerometer_event_handler(const MicroBitEvent *evt);

/****************************************************************/
// microbit.button objects

extern const struct _microbit_button_obj_t microbit_button_a_obj;
extern const struct _microbit_button_obj_t microbit_button_b_obj;

void microbit_button_tick(void);

/****************************************************************/
// microbit.i2c, microbit.uart and microbit.spi objects

extern const struct _microbit_i2c_obj_t microbit_i2c_obj;
extern struct _microbit_uart_obj_t microbit_uart_obj;
extern struct _microbit_spi_obj_t microbit_spi_obj;

/****************************************************************/
// declarations of microbit functions and methods

MP_DECLARE_CONST_FUN_OBJ_0(microbit_reset_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_sleep_obj);
MP_DECLARE_CONST_FUN_OBJ_0(microbit_random_obj);
MP_DECLARE_CONST_FUN_OBJ_0(microbit_running_time_obj);
MP_DECLARE_CONST_FUN_OBJ_0(microbit_temperature_obj);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_panic_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_accelerometer_get_x_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_accelerometer_get_y_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_accelerometer_get_z_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_button_is_pressed_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_button_was_pressed_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_button_get_presses_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_compass_is_calibrated_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_compass_heading_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_compass_calibrate_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_compass_is_calibrating_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_compass_clear_calibration_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_compass_get_x_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_compass_get_y_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_compass_get_z_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_compass_get_field_strength_obj);
MP_DECLARE_CONST_FUN_OBJ_KW(microbit_display_show_obj);
MP_DECLARE_CONST_FUN_OBJ_KW(microbit_display_scroll_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_display_clear_obj);
MP_DECLARE_CONST_FUN_OBJ_3(microbit_display_get_pixel_obj);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_display_set_pixel_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_display_on_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_display_off_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_display_is_on_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_pin_read_digital_obj);
MP_DECLARE_CONST_FUN_OBJ_2(microbit_pin_write_digital_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_pin_read_analog_obj);
MP_DECLARE_CONST_FUN_OBJ_2(microbit_pin_write_analog_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_pin_is_touched_obj);
MP_DECLARE_CONST_FUN_OBJ_2(microbit_pin_set_analog_period_obj);
MP_DECLARE_CONST_FUN_OBJ_2(microbit_pin_set_analog_period_microseconds_obj);
MP_DECLARE_CONST_FUN_OBJ_KW(microbit_i2c_init_obj);
MP_DECLARE_CONST_FUN_OBJ_KW(microbit_i2c_read_obj);
MP_DECLARE_CONST_FUN_OBJ_KW(microbit_i2c_write_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_image_width_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_image_height_obj);
MP_DECLARE_CONST_FUN_OBJ_3(microbit_image_get_pixel_obj);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_image_set_pixel_obj);
MP_DECLARE_CONST_FUN_OBJ_2(microbit_image_shift_left_obj);
MP_DECLARE_CONST_FUN_OBJ_2(microbit_image_shift_right_obj);
MP_DECLARE_CONST_FUN_OBJ_2(microbit_image_shift_up_obj);
MP_DECLARE_CONST_FUN_OBJ_2(microbit_image_shift_down_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_image_copy_obj);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_image_crop_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_image_invert_obj);
MP_DECLARE_CONST_FUN_OBJ_KW(microbit_uart_init_obj);
MP_DECLARE_CONST_FUN_OBJ_1(microbit_uart_any_obj);
MP_DECLARE_CONST_FUN_OBJ_KW(microbit_spi_init_obj);
MP_DECLARE_CONST_FUN_OBJ_2(microbit_spi_write_obj);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_spi_read_obj);
MP_DECLARE_CONST_FUN_OBJ_3(microbit_spi_write_readinto_obj);
MP_DECLARE_CONST_FUN_OBJ_KW(microbit_music_set_tempo_obj);
MP_DECLARE_CONST_FUN_OBJ_KW(microbit_music_pitch_obj);
MP_DECLARE_CONST_FUN_OBJ_KW(microbit_music_play_obj);
MP_DECLARE_CONST_FUN_OBJ_0(microbit_music_get_tempo_obj);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(microbit_music_stop_obj);
MP_DECLARE_CONST_FUN_OBJ_0(microbit_music_reset_obj);
MP_DECLARE_CONST_FUN_OBJ_0(love_badaboom_obj);
MP_DECLARE_CONST_FUN_OBJ_0(this_authors_obj);

#endif // MICROPY_INCLUDED_MICROBIT_MODMICROBIT_H
