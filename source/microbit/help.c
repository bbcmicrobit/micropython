/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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

#include "py/builtin.h"
#include "py/stream.h"
#include "microbit/modmicrobit.h"

const char *microbit_help_text =
"Welcome to MicroPython on the micro:bit!\n"
"\n"
"Try these commands:\n"
"  display.scroll('Hello')\n"
"  running_time()\n"
"  sleep(1000)\n"
"  button_a.is_pressed()\n"
"What do these commands do? Can you improve them? HINT: use the up and down\n"
"arrow keys to get your command history. Press the TAB key to auto-complete\n"
"unfinished words (so 'di' becomes 'display' after you press TAB). These\n"
"tricks save a lot of typing and look cool!\n"
"\n"
"Explore:\n"
"Type 'help(something)' to find out about it. Type 'dir(something)' to see what\n"
"it can do. Type 'dir()' to see what stuff is available. For goodness sake,\n"
"don't type 'import this'.\n"
"\n"
"Control commands:\n"
"  CTRL-C        -- stop a running program\n"
"  CTRL-D        -- on a blank line, do a soft reset of the micro:bit\n"
"  CTRL-E        -- enter paste mode, turning off auto-indent\n"
"\n"
"For a list of available modules, type help('modules')\n"
"\n"
"For more information about Python, visit: http://python.org/\n"
"To find out about MicroPython, visit: http://micropython.org/\n"
"Python/micro:bit documentation is here: https://microbit-micropython.readthedocs.io/\n"
;

typedef struct _mp_doc_t {
    mp_const_obj_t obj;
    const char *doc;
} mp_doc_t;

STATIC const mp_doc_t help_table_types[] = {
    {&microbit_accelerometer_type, "MicroBitAccelerometer type\n"},
};

// Consistency between messages and minimal jargon improves help text.
STATIC const mp_doc_t help_table_instances[] = {
    {&microbit_module, "Useful stuff to control the micro:bit hardware.\n"},
    // System state objects
    {&microbit_panic_obj, "Put micro:bit in panic() mode and display an unhappy face.\nPress reset button to exit panic() mode.\n"},
    {&microbit_sleep_obj, "Put micro:bit to sleep(time) for some milliseconds (1 second = 1000 ms) of time.\nsleep(2000) gives micro:bit a 2 second nap.\n"},
    {&microbit_running_time_obj, "Return running_time() in milliseconds since micro:bit's last reset.\n"},
    {&microbit_temperature_obj, "Return micro:bit's temperature in degrees Celcius.\n"},
    // Accelerometer 3D orientation
    {&microbit_accelerometer_obj, "Detect micro:bit's movement in 3D.\nIt measures tilt (X and Y) and up-down (Z) motion.\n"},
    {&microbit_accelerometer_get_x_obj, "Return micro:bit's tilt (X acceleration) in milli-g's.\n"},
    {&microbit_accelerometer_get_y_obj, "Return micro:bit's tilt (Y acceleration) in milli-g's.\n"},
    {&microbit_accelerometer_get_z_obj, "Return micro:bit's up-down motion (Z acceleration) in milli-g's.\nZ is a positive number when moving up. Moving down, Z is a negative number.\n"},
    // Pushbutton
    {&microbit_button_a_obj, "micro:bit's 'A' button. When button is pressed down, is_pressed() is True.\n"},
    {&microbit_button_b_obj, "micro:bit's 'B' button. When button is pressed down, is_pressed() is True.\n"},
    {&microbit_button_is_pressed_obj, "If the button is pressed down, is_pressed() is True, else False.\n"},
    {&microbit_button_was_pressed_obj, "Use was_pressed() to learn if the button was pressed since the last time\nwas_pressed() was called. Returns True or False.\n"},
    {&microbit_button_get_presses_obj, "Use get_presses() to get the running total of button presses, and also\nreset this counter to zero.\n"},
    // Compass 3D direction heading
    {&microbit_compass_heading_obj, "Gives a compass heading between 0-360 with 0 as north.\n"},
    {&microbit_compass_obj, "Use micro:bit's compass to detect the direction it is heading in.\nThe compass can detect magnetic fields.\nIt uses the Earth's magnetic field to detect direction.\n"},
    {&microbit_compass_is_calibrated_obj, "If micro:bit's compass is_calibrated() and adjusted for accuracy, return True.\nIf compass hasn't been adjusted for accuracy, return False.\n"},
    {&microbit_compass_calibrate_obj, "If micro:bit is confused, calibrate() the compass to adjust the its accuracy.\nIt will ask you to rotate the device to draw a circle on the display.\n"},
    {&microbit_compass_clear_calibration_obj, "Reset micro:bit's compass using clear_calibration() command.\nRun calibrate() to improve accuracy.\n"},
    {&microbit_compass_get_x_obj, "Return magnetic field detected along micro:bit's X axis.\nUsually, the compass returns the earth's magnetic field in micro-Tesla units.\nUnless...a strong magnet is nearby!\n"},
    {&microbit_compass_get_y_obj, "Return magnetic field detected along micro:bit's Y axis.\nUsually, the compass returns the earth's magnetic field in micro-Tesla units.\nUnless...a strong magnet is nearby!\n"},
    {&microbit_compass_get_z_obj, "Return magnetic field detected along micro:bit's Z axis.\nUsually, the compass returns the earth's magnetic field in micro-Tesla units.\nUnless...a strong magnet is nearby!\n"},
    {&microbit_compass_get_field_strength_obj, "Return strength of magnetic field around micro:bit.\n"},
    // Display 5x5 LED grid
    {&microbit_display_obj, "micro:bit's 5x5 LED display.\n"},
    {&microbit_display_show_obj, "Use show(x) to print the string or images 'x' to the display. Try show('Hi!').\nUse show(s, i) to show string 's', one character at a time with a delay of\n'i' milliseconds.\n"},
    {&microbit_display_scroll_obj, "Use scroll(s) to scroll the string 's' across the display.\nUse scroll(s, i) to scroll string 's' with a delay of 'i' milliseconds after\neach character.\n"},
    {&microbit_display_clear_obj, "Use clear() to clear micro:bit's display.\n"},
    {&microbit_display_get_pixel_obj, "Use get_pixel(x, y) to return the display's brightness at LED pixel (x,y).\nBrightness can be from 0 (LED is off) to 9 (maximum LED brightness).\n"},
    {&microbit_display_set_pixel_obj, "Use set_pixel(x, y, b) to set the display at LED pixel (x,y) to brightness 'b'\nwhich can be set between 0 (off) to 9 (full brightness).\n"},
    {&microbit_display_on_obj, "Use on() to turn on the display.\n"},
    {&microbit_display_off_obj, "Use off() to turn off the display.\n"},
    {&microbit_display_is_on_obj, "Use is_on() to query if the micro:bit's display is on (True) or off (False).\n"},
    // Pins
    {&microbit_p0_obj, "micro:bit's pin 0 on the gold edge connector.\n"},
    {&microbit_p1_obj, "micro:bit's pin 1 on the gold edge connector.\n"},
    {&microbit_p2_obj, "micro:bit's pin 2 on the gold edge connector.\n"},
    {&microbit_p3_obj, "micro:bit's pin 3 on the gold edge connector.\n"},
    {&microbit_p4_obj, "micro:bit's pin 4 on the gold edge connector.\n"},
    {&microbit_p5_obj, "micro:bit's pin 5 on the gold edge connector.\n"},
    {&microbit_p6_obj, "micro:bit's pin 6 on the gold edge connector.\n"},
    {&microbit_p7_obj, "micro:bit's pin 7 on the gold edge connector.\n"},
    {&microbit_p8_obj, "micro:bit's pin 8 on the gold edge connector.\n"},
    {&microbit_p9_obj, "micro:bit's pin 9 on the gold edge connector.\n"},
    {&microbit_p10_obj, "micro:bit's pin 10 on the gold edge connector.\n"},
    {&microbit_p11_obj, "micro:bit's pin 11 on the gold edge connector.\n"},
    {&microbit_p12_obj, "micro:bit's pin 12 on the gold edge connector.\n"},
    {&microbit_p13_obj, "micro:bit's pin 13 on the gold edge connector.\n"},
    {&microbit_p14_obj, "micro:bit's pin 14 on the gold edge connector.\n"},
    {&microbit_p15_obj, "micro:bit's pin 15 on the gold edge connector.\n"},
    {&microbit_p16_obj, "micro:bit's pin 16 on the gold edge connector.\n"},
    {&microbit_p19_obj, "micro:bit's pin 19 on the gold edge connector.\n"},
    {&microbit_p20_obj, "micro:bit's pin 20 on the gold edge connector.\n"},
    {&microbit_pin_write_digital_obj, "micro:bit, write_digital(choice) to the pin. You have two 'choice' values,\n0 (lo) or 1 (hi).\n"},
    {&microbit_pin_read_digital_obj, "micro:bit, read_digital() value from the pin as either 0 (lo) or 1 (hi).\n"},
    {&microbit_pin_write_analog_obj, "micro:bit, write_analog(value) to the pin. You can use any value between\n0 and 1023.\n"},
    {&microbit_pin_read_analog_obj, "micro:bit, read_analog() value from the pin. Wow, analog has lots of values\n(0 - 65535). Digital has only 0 and 1.\n"},
    {&microbit_pin_is_touched_obj, "If pin is_touched() on micro:bit, return True. If nothing is touching the pin,\nreturn False.\n"},
    // I2C
    {&microbit_i2c_obj, "Communicate with one or more named devices connected to micro:bit. Each named\ndevice has an 'address', communicates using I2C, and connects to the I/O pins.\n"},
    {&microbit_i2c_read_obj, "Use read(address, n) to read 'n' bytes from the device with this address.\n"},
    {&microbit_i2c_write_obj, "Use write(address, buffer) to write to the 'buffer' of the device at this 'address'.\n"},
    {&microbit_i2c_init_obj, "Use init(frequency, scl, sda) to set the bus frequency and pins.\n"},
    // Image
    {&microbit_image_type, "Create and use built-in IMAGES to show on the display. Use:\nImage(\n  '09090:'\n  '99999:'\n  '99999:'\n  '09990:'\n  '00900:')\n...to make a new 5x5 heart image. Numbers go from 0 (off) to 9 (brightest). Note\nthe colon ':' to set the end of a row.\n"},
    {&microbit_image_width_obj, "Return the width of the image in pixels.\n"},
    {&microbit_image_height_obj, "Return the height of the image in pixels.\n"},
    {&microbit_image_get_pixel_obj, "Use get_pixel(x, y) to return the image's brightness at LED pixel (x,y).\nBrightness can be from 0 (LED is off) to 9 (maximum LED brightness).\n"},
    {&microbit_image_set_pixel_obj, "Use set_pixel(x, y, b) to set the LED pixel (x,y) in the image to brightness\n'b' which can be set between 0 (off) to 9 (full brightness).\n"},
    {&microbit_image_shift_left_obj, "Use shift_left(i) to make a copy of the image but moved 'i' pixels to the left.\n"},
    {&microbit_image_shift_right_obj, "Use shift_right(i) to make a copy of the image but moved 'i' pixels to\nthe right.\n"},
    {&microbit_image_shift_up_obj, "Use shift_up(i) to make a copy of the image but moved 'i' pixels up.\n"},
    {&microbit_image_shift_down_obj, "Use shift_down(i) to make a copy of the image but moved 'i' pixels down.\n"},
    {&microbit_image_copy_obj, "Use copy() to make a new exact copy of the image.\n"},
    {&microbit_image_crop_obj, "Use crop(x1, y1, x2, y2) to make a cut-out copy of the image where coordinate\n(x1,y1) is the top left corner of the cut-out area and coordinate (x2,y2) is the\nbottom right corner.\n"},
    {&microbit_image_invert_obj, "Use invert() to make a negative copy of the image. Where a pixel was bright or\non in the original, it is dim or off in the negative copy.\n"},
    // uart
    {&microbit_uart_obj, "Communicate with a serial device connected to micro:bit's I/O pins.\n"},
    {&microbit_uart_init_obj, "Use init() to set up communication. Use pins 0 (TX) and 1 (RX) with a baud\nrate of 9600.\nOverride the defaults for 'baudrate', 'parity' and 'pins'.\n"},
    {&microbit_uart_any_obj, "If there are incoming characters waiting to be read, any() will return True.\nOtherwise, returns False.\n"},
    {&mp_stream_read_obj, "Use read() to read characters.\nUse read(n) to read, at most, 'n' bytes of data.\n"},
    {&mp_stream_unbuffered_readline_obj, "Use readline() to read a line that ends with a newline character.\n"},
    {&mp_stream_readinto_obj, "Use readinto(buf) to read bytes into the buffer 'buf'.\nUse readinto(buff, n) to read, at most, 'n' number of bytes into 'buf'.\n"},
    {&mp_stream_write_obj, "Use write(buf) to write the bytes in buffer 'buf' to the connected device.\n"},
    // SPI
    {&microbit_spi_obj, "Communicate using a serial peripheral interface (SPI) device connected to\nmicro:bit's I/O pins.\n"},
    {&microbit_spi_init_obj, "Use init() to set up communication. Override the defaults for baudrate, mode,\nSCLK, MOSI and MISO. The default connections are pin13 for SCLK, pin15 for\nMOSI and pin14 for MISO.\n"},
    {&microbit_spi_write_obj, "Use write(buf) to write bytes in buffer 'buf' to the connected device.\n"},
    {&microbit_spi_read_obj, "Use read(n) to read 'n' bytes of data.\n"},
    {&microbit_spi_write_readinto_obj, "Use write_readinto(out, in) to write the 'out' buffer to the connected device\nand read any response into the 'in' buffer. The length of the buffers should\nbe the same. The buffers can be the same object.\n"},
    // Music module
    {&music_module, "Plug in a speaker with crocodile clips and make micro:bit go bleep and bloop.\n"},
    {&microbit_music_set_tempo_obj, "Use set_tempo(number, bpm) to make a beat last a 'number' of ticks long and\nplayed at 'bpm' beats per minute.\n"},
    {&microbit_music_pitch_obj, "Use pitch(freq, length) to make micro:bit play a note at 'freq' frequency for\n'length' milliseconds. E.g. pitch(440, 1000) will play concert 'A' for 1 second.\n"},
    {&microbit_music_play_obj, "Use play(music) to make micro:bit play 'music' list of notes. Try out the\nbuilt in music to see how it works. E.g. music.play(music.PUNCHLINE).\n"},
    {&microbit_music_get_tempo_obj, "Use get_tempo() to return the number of ticks in a beat and number of beats\nper minute.\n"},
    {&microbit_music_stop_obj, "Use to stop() the music that is playing.\n"},
    {&microbit_music_reset_obj, "If things go wrong, reset() the music to its default settings.\n"},
    // Antigravity
    {&antigravity_module, "See: http://xkcd.com/353/\n"},
    // This module
    {&this_module, "The Zen of Python defines what it is to be Pythonic. It wouldn't fit on this\ndevice so we've written a Zen of MicroPython instead.\n"},
    {&this_authors_obj, "Use authors() to reveal the names of the people who created this software.\n"},
    // Love module
    {&love_module, "All you need. Use love.badaboom() to repeat the effect.\n"},
    {&love_badaboom_obj, "Hear my soul speak:\nThe very instant that I saw you, did\nMy heart fly to your service.\n"},
};

bool mp_plat_specific_help(mp_obj_t args0) {
    mp_obj_type_t *args0_type = mp_obj_get_type(args0);

    // see if we have specific help info for this instance
    for (size_t i = 0; i < MP_ARRAY_SIZE(help_table_instances); i++) {
        if (args0 == help_table_instances[i].obj) {
            mp_print_str(&mp_plat_print, help_table_instances[i].doc);
            //if (args0_type == &mp_type_module) {
            //TODO here we can list the things inside the module
            //}
            return true;
        }
    }

    // see if we have specific help info for this type
    for (size_t i = 0; i < MP_ARRAY_SIZE(help_table_types); i++) {
        if (args0 == help_table_types[i].obj || args0_type == help_table_types[i].obj) {
            mp_print_str(&mp_plat_print, help_table_types[i].doc);
            return true;
        }
    }

    return false;
}
