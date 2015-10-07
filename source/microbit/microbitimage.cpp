/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien George, Mark Shannon
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
#include "microbitobj.h"

extern "C" {

#include "py/runtime.h"
#include "modmicrobit.h"
#include "microbitimage.h"
#include "py/runtime0.h"

monochrome_5by5_t BLANK_IMAGE = {
    { &microbit_image_type },
    1, 0, 0, 0, 0,
    { 0, 0, 0 }
};

STATIC void microbit_image_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    mp_printf(print, "+");
    for (int x = 0; x < self->width(); ++x) {
        mp_printf(print, "-");
    }
    mp_printf(print, "+\n");
    for (int y = 0; y < self->height(); ++y) {
        mp_printf(print, "|");
        for (int x = 0; x < self->width(); ++x) {
            self->printPixel(x, y, print);
        }
        mp_printf(print, "|\n");
    }
    mp_printf(print, "+");
    for (int x = 0; x < self->width(); ++x) {
        mp_printf(print, "-");
    }
    mp_printf(print, "+\n");
}
    
    
void microbit_image_obj_t::printPixel(mp_int_t x, mp_int_t y, const mp_print_t *print) {
    if (this->base.five)
        this->monochrome_5by5.printPixel(x, y, print);
    else if (this->base.monochrome)
        this->monochrome.printPixel(x, y, print);
    else
        this->greyscale.printPixel(x, y, print);
}
    
void monochrome_5by5_t::printPixel(mp_int_t x, mp_int_t y, const mp_print_t *print) {
    if (this->getPixelValue(x, y)) {
        mp_printf(print, "X");
    } else {
        mp_printf(print, " ");
    }
}
    
void monochrome_t::printPixel(mp_int_t x, mp_int_t y, const mp_print_t *print) {
    if (this->getPixelValue(x, y)) {
        mp_printf(print, "X");
    } else {
        mp_printf(print, " ");
    }
}

void greyscale_t::printPixel(mp_int_t x, mp_int_t y, const mp_print_t *print) {
    mp_printf(print, "%c", " 123456789X"[this->getPixelValue(x, y)]);
}

int monochrome_5by5_t::getPixelValue(mp_int_t x, mp_int_t y) {
    unsigned int index = y*5+x;
    if (index == 24) 
        return this->pixel44;
    return (this->bits24[index>>3] >> (index&7))&1;
}

void monochrome_5by5_t::setPixelValue(mp_int_t x, mp_int_t y, mp_int_t val) {
    unsigned int index = y*5+x;
    if (index == 24) {
        this->pixel44 = (val!=0);
        return;
    }
    uint8_t mask = 1 << (index&7);
    if (val)
        this->bits24[index>>3] |= mask;
    else
        this->bits24[index>>3] &= ~mask;
}

int monochrome_t::getPixelValue(mp_int_t x, mp_int_t y) {
    unsigned int index = y*this->width+x;
    return (this->bit_data[index>>3] >> (index&7))&1;
}

void monochrome_t::setPixelValue(mp_int_t x, mp_int_t y, mp_int_t val) {
    unsigned int index = y*this->width+x;
    uint8_t mask = 1 << (index&7);
    if (val)
        this->bit_data[index>>3] |= mask;
    else
        this->bit_data[index>>3] &= ~mask;
}

int greyscale_t::getPixelValue(mp_int_t x, mp_int_t y) {
    unsigned int index = y*this->width+x;
    return this->byte_data[index];
}

void greyscale_t::setPixelValue(mp_int_t x, mp_int_t y, mp_int_t val) {
    unsigned int index = y*this->width+x;
    this->byte_data[index] = val;
}


mp_int_t microbit_image_obj_t::getPixelValue(mp_int_t x, mp_int_t y) {
    if (this->base.five)
        return this->monochrome_5by5.getPixelValue(x, y)*MAX_BRIGHTNESS;
    else if (this->base.monochrome)
        return this->monochrome.getPixelValue(x, y)*MAX_BRIGHTNESS;
    else
        return this->greyscale.getPixelValue(x, y);
}

void microbit_image_obj_t::setPixelValue(mp_int_t x, mp_int_t y, mp_int_t val) {
    if (this->base.five)
        this->monochrome_5by5.setPixelValue(x, y, val);
    else if (this->base.monochrome)
        this->monochrome.setPixelValue(x, y, val);
    else
        this->greyscale.setPixelValue(x, y, val);
}

mp_int_t microbit_image_obj_t::width() {
    if (this->base.five)
        return 5;
    else if (this->base.monochrome)
        return this->monochrome.width;
    else
        return this->greyscale.width;
}

mp_int_t microbit_image_obj_t::height() {
    if (this->base.five)
        return 5;
    else if (this->base.monochrome)
        return this->monochrome.height;
    else
        return this->greyscale.height;
}

STATIC monochrome_5by5_t *monochrome_5by5_new() {
    monochrome_5by5_t *result = m_new_obj(monochrome_5by5_t);
    result->base.type = &microbit_image_type;
    result->five = 1;
    result->monochrome = 0;
    result->greyscale = 0;
    return result;
}

STATIC monochrome_t *monochrome_new(mp_int_t w, mp_int_t h) {
    monochrome_t *result = m_new_obj_var(monochrome_t, uint8_t, (w*h+7)>>3);
    result->base.type = &microbit_image_type;
    result->five = 0;
    result->monochrome = 1;
    result->greyscale = 0;
    result->width = w;
    result->height = h;
    return result;
}

STATIC greyscale_t *greyscale_new(mp_int_t w, mp_int_t h) {
    greyscale_t *result = m_new_obj_var(greyscale_t, uint8_t, w*h);
    result->base.type = &microbit_image_type;
    result->five = 0;
    result->monochrome = 0;
    result->greyscale = 1;
    result->width = w;
    result->height = h;
    return result;
}

microbit_image_obj_t *microbit_image_obj_t::partialCopy() {
    microbit_image_obj_t *result;
    if (this->base.five) {
        result = (microbit_image_obj_t *)monochrome_5by5_new();
    } else if (this->base.monochrome) {
        result = (microbit_image_obj_t *)monochrome_new(this->monochrome.width, this->monochrome.height);
    } else {
        result = (microbit_image_obj_t *)greyscale_new(this->greyscale.width, this->greyscale.height);
    }
    return result;
}

microbit_image_obj_t *microbit_image_obj_t::shiftLeft(mp_int_t n) {
    mp_int_t w = this->width();
    mp_int_t h = this->height();
    n = max(n, -w);
    n = min(n, w);
    mp_int_t src_start = max(n, 0);
    mp_int_t src_end = min(w+n,w);
    mp_int_t dest = max(0,-n);
    microbit_image_obj_t *result = this->partialCopy();
    for (mp_int_t x = 0; x < dest; ++x) {
        for (mp_int_t y = 0; y < h; y++) {
            result->setPixelValue(x, y, 0);
        }
    }
    for (mp_int_t x = src_start; x < src_end; ++x) {
        for (mp_int_t y = 0; y < h; y++) {
             result->setPixelValue(dest, y, this->getPixelValue(x, y));
        }
        ++dest;
    }
    for (mp_int_t x = dest; x < w; ++x) {
        for (mp_int_t y = 0; y < h; y++) {
            result->setPixelValue(x, y, 0);
        }
    }
    return result;
}


microbit_image_obj_t *microbit_image_obj_t::shiftUp(mp_int_t n) {
    mp_int_t w = this->width();
    mp_int_t h = this->height();
    n = max(n, -h);
    n = min(n, h);
    mp_int_t src_start = max(n, 0);
    mp_int_t src_end = min(h+n,h);
    mp_int_t dest = max(0,-n);
    microbit_image_obj_t *result = this->partialCopy();
    for (mp_int_t y = 0; y < dest; ++y) {
        for (mp_int_t x = 0; x < w; x++) {
            result->setPixelValue(x, y, 0);
        }
    }
    for (mp_int_t y = src_start; y < src_end; ++y) {
        for (mp_int_t x = 0; x < w; x++) {
             result->setPixelValue(x, dest, this->getPixelValue(x, y));
        }
        ++dest;
    }
    for (mp_int_t y = dest; y < h; ++y) {
        for (mp_int_t x = 0; x < w; x++) {
            result->setPixelValue(x, y, 0);
        }
    }
    return result;
}

STATIC monochrome_t *monochrome_from_str(const char *s, mp_int_t len) {
    monochrome_t *image = monochrome_new(len*5, MICROBIT_FONT_HEIGHT);
    MicroBitFont font = uBit.display.getFont();
    for (mp_int_t i = 0; i < len; ++i) {
        char c = s[i];
        if (c < MICROBIT_FONT_ASCII_START || c > font.asciiEnd)
            c = '?';
        
        /* The following logic belongs in MicroBitFont */
        int offset = (c-MICROBIT_FONT_ASCII_START) * 5;

        for (int row=0; row<MICROBIT_FONT_HEIGHT; row++)
        {
            unsigned char v = font.characters[offset+row];

            for (int col = 0; col < MICROBIT_FONT_WIDTH; col++)
            {
                image->setPixelValue(len*5+col, row, v & (0x10 >> col));
            }
        }
    }
    return image;
};


STATIC microbit_image_obj_t *image_from_parsed_str(const char *s, mp_int_t len) {
    mp_int_t w = 0;
    mp_int_t h = 0;
    mp_int_t line_len = 0;
    bool old_style = len > 1 && s[1] == ',';
    bool seen_one = false;
    bool monochrome = true;
    microbit_image_obj_t *result;
    /*First pass -- Establish metadata */
    for (int i = 0; i < len; i++) {
        char c = s[i];
        if (c == '\n') {
            w = max(line_len, w);
            line_len = 0;
            ++h;
        } else if (c == ',') {
            /* Ignore commas */
        } else if (c == '0') {
            ++line_len;
        } else if (c == '1') {
            ++line_len;
            seen_one = true;
        } else if ('c' > '1' && c <= '9') {
            ++line_len;
            monochrome = false;
        } else {
            old_style = false;
            ++line_len;
        }
    }
    if (line_len) {
        // Omitted trainling newline
        ++h;
        w = max(line_len, w); 
    }
    if (!old_style && seen_one)
        monochrome = false;
    //Create new image
    if (monochrome) {
        if (w == 5 && h == 5) {
            result = (microbit_image_obj_t *)monochrome_5by5_new();
        } else {
            result = (microbit_image_obj_t *)monochrome_new(w, h);
        }
    } else {
        result = (microbit_image_obj_t *)greyscale_new(w, h);
    }
    mp_int_t x = 0;
    mp_int_t y = 0;
    /* Second pass -- Fill in data */
    for (int i = 0; i < len; i++) {
        char c = s[i];
        if (c == '\n') {
            while (x < w) {
                result->setPixelValue(x, y, 0);
                x++;
            }
            ++y;
            x = 0;
        } else if (c == ',') {
            /* Ignore commas */
        } else if (c == ' ') {
            /* Treat spaces as 0 */
            result->setPixelValue(x, y, 0);
            ++x;
        } else if ('c' >= '0' && c <= '9') {
            result->setPixelValue(x, y, c - '0');
            ++x;
        } else {
            result->setPixelValue(x, y, MAX_BRIGHTNESS);
            ++x;
        }
    }
    if (y < h) {
        while (x < w) {
            result->setPixelValue(x, y, 0);
            x++;
        }
    }
    return result;
}


STATIC mp_obj_t microbit_image_make_new(mp_obj_t type_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    (void)type_in;
    mp_arg_check_num(n_args, n_kw, 0, 3, false);

    switch (n_args) {
        case 0: {
            return &BLANK_IMAGE;
        }

        case 1: {
            // make image from string
            mp_int_t len = mp_obj_get_int(mp_obj_len(args[0]));
            return image_from_parsed_str(mp_obj_str_get_str(args[0]), len);
        }

        case 2: {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError,
                "Image() takes 0, 1 or 3 arguments"));
        }

        case 3:
        default: {
            mp_int_t w = mp_obj_get_int(args[0]);
            mp_int_t h = mp_obj_get_int(args[1]);
            mp_buffer_info_t bufinfo;
            mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);

            if (w < 0 || h < 0 || (size_t)(w * h) != bufinfo.len) {
                nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError,
                    "image data is incorrect size"));
            }
            greyscale_t *image = greyscale_new(w, h);
            for (mp_int_t i = 0; i < w*h; ++i) {
                image->byte_data[i] = ((const uint8_t*)bufinfo.buf)[i];
            }
            return image;
        }
    }
}

mp_obj_t microbit_image_width(mp_obj_t self_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    return MP_OBJ_NEW_SMALL_INT(self->width());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_image_width_obj, microbit_image_width);

mp_obj_t microbit_image_height(mp_obj_t self_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    return MP_OBJ_NEW_SMALL_INT(self->height());
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_image_height_obj, microbit_image_height);

mp_obj_t microbit_image_get_pixel(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    mp_int_t x = mp_obj_get_int(x_in);
    mp_int_t y = mp_obj_get_int(y_in);
    if (x < 0 || y < 0) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError,
            "index cannot be negative"));
    }
    if (x < self->width() && y < self->height()) {
        return MP_OBJ_NEW_SMALL_INT(self->getPixelValue(x, y));
    }
    nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "index too large"));
}
MP_DEFINE_CONST_FUN_OBJ_3(microbit_image_get_pixel_obj, microbit_image_get_pixel);


mp_obj_t microbit_image_shift_left(mp_obj_t self_in, mp_obj_t n_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    mp_int_t n = mp_obj_get_int(n_in);
    if (n == 0)
        return self;
    return self->shiftLeft(n);
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_image_shift_left_obj, microbit_image_shift_left);

mp_obj_t microbit_image_shift_right(mp_obj_t self_in, mp_obj_t n_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    mp_int_t n = mp_obj_get_int(n_in);
    if (n == 0)
        return self;
    return self->shiftLeft(-n);
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_image_shift_right_obj, microbit_image_shift_right);

mp_obj_t microbit_image_shift_up(mp_obj_t self_in, mp_obj_t n_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    mp_int_t n = mp_obj_get_int(n_in);
    if (n == 0)
        return self;
    return self->shiftUp(n);
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_image_shift_up_obj, microbit_image_shift_up);

mp_obj_t microbit_image_shift_down(mp_obj_t self_in, mp_obj_t n_in) {
    microbit_image_obj_t *self = (microbit_image_obj_t*)self_in;
    mp_int_t n = mp_obj_get_int(n_in);
    if (n == 0)
        return self;
    return self->shiftUp(-n);
}
MP_DEFINE_CONST_FUN_OBJ_2(microbit_image_shift_down_obj, microbit_image_shift_down);

STATIC const mp_map_elem_t microbit_image_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_width), (mp_obj_t)&microbit_image_width_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_height), (mp_obj_t)&microbit_image_height_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_pixel), (mp_obj_t)&microbit_image_get_pixel_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shift_left), (mp_obj_t)&microbit_image_shift_left_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shift_right), (mp_obj_t)&microbit_image_shift_right_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shift_up), (mp_obj_t)&microbit_image_shift_up_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_shift_down), (mp_obj_t)&microbit_image_shift_down_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_HEART), (mp_obj_t)&microbit_const_image_heart_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_HEART_SMALL), (mp_obj_t)&microbit_const_image_heart_small_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_HAPPY), (mp_obj_t)&microbit_const_image_happy_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SMILE), (mp_obj_t)&microbit_const_image_smile_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SAD), (mp_obj_t)&microbit_const_image_sad_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CONFUSED), (mp_obj_t)&microbit_const_image_confused_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ANGRY), (mp_obj_t)&microbit_const_image_angry_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ASLEEP), (mp_obj_t)&microbit_const_image_asleep_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_SURPRISED), (mp_obj_t)&microbit_const_image_surprised_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_YES), (mp_obj_t)&microbit_const_image_yes_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_NO), (mp_obj_t)&microbit_const_image_no_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK12), (mp_obj_t)&microbit_const_image_clock12_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK1), (mp_obj_t)&microbit_const_image_clock1_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK2), (mp_obj_t)&microbit_const_image_clock2_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK3), (mp_obj_t)&microbit_const_image_clock3_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK4), (mp_obj_t)&microbit_const_image_clock4_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK5), (mp_obj_t)&microbit_const_image_clock5_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK6), (mp_obj_t)&microbit_const_image_clock6_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK7), (mp_obj_t)&microbit_const_image_clock7_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK8), (mp_obj_t)&microbit_const_image_clock8_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK9), (mp_obj_t)&microbit_const_image_clock9_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK10), (mp_obj_t)&microbit_const_image_clock10_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CLOCK11), (mp_obj_t)&microbit_const_image_clock11_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_N), (mp_obj_t)&microbit_const_image_arrow_n_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_NE), (mp_obj_t)&microbit_const_image_arrow_ne_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_E), (mp_obj_t)&microbit_const_image_arrow_e_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_SE), (mp_obj_t)&microbit_const_image_arrow_se_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_S), (mp_obj_t)&microbit_const_image_arrow_s_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_SW), (mp_obj_t)&microbit_const_image_arrow_sw_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_W), (mp_obj_t)&microbit_const_image_arrow_w_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ARROW_NW), (mp_obj_t)&microbit_const_image_arrow_nw_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_image_locals_dict, microbit_image_locals_dict_table);

STATIC mp_obj_t microbit_image_dim(mp_obj_t lhs_in, mp_obj_t rhs_in) {
    if (mp_obj_get_type(lhs_in) != &microbit_image_type)
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "Must be an image"));
    microbit_image_obj_t *lhs = (microbit_image_obj_t *)lhs_in;
    mp_float_t fval = mp_obj_float_get(rhs_in);
    if (fval < 0) 
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Brightness multiplier must not be negative."));
    greyscale_t *result = greyscale_new(lhs->width(), lhs->height());
    for (int x = 0; x < lhs->width(); ++x) {
        for (int y = 0; y < lhs->width(); ++y) {
            int val = min((int)lhs->getPixelValue(x,y)*fval+0.5, MAX_BRIGHTNESS);
            result->setPixelValue(x, y, val);
        }
    }
    return result;
    
}

STATIC mp_obj_t image_binary_op(mp_uint_t op, mp_obj_t lhs_in, mp_obj_t rhs_in) {
    switch(op) {
    case MP_BINARY_OP_ADD:
    case MP_BINARY_OP_SUBTRACT:
        break;
    case MP_BINARY_OP_MULTIPLY:
        return microbit_image_dim(lhs_in, rhs_in);
    default:
        return MP_OBJ_NULL; // op not supported
    }
    if (mp_obj_get_type(lhs_in) != &microbit_image_type || mp_obj_get_type(rhs_in) != &microbit_image_type) {
        const char *msg;
        if (op == MP_BINARY_OP_ADD) {
            msg = "Can only add other images to images.";
        } else {
            msg = "Can only subtract other images from images.";
        }
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, msg));
    }
    microbit_image_obj_t *lhs = (microbit_image_obj_t *)lhs_in;
    microbit_image_obj_t *rhs = (microbit_image_obj_t *)rhs_in;
    mp_int_t h = lhs->height();
    mp_int_t w = lhs->width();
    if (rhs->height() != h || rhs->width() != w) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Images must be the same size."));
    }
    microbit_image_obj_t *result;
    /* Make sure that we have a result image that can handle greyscale if necessary */
    if (lhs->base.greyscale)
        result = lhs->partialCopy();
    else 
        result = rhs->partialCopy();
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            int val;
            if (op == MP_BINARY_OP_ADD) {
                val = min(lhs->getPixelValue(x,y) + rhs->getPixelValue(x,y), MAX_BRIGHTNESS);
            } else {
                val = max(0, lhs->getPixelValue(x,y) - rhs->getPixelValue(x,y));
            }
            result->setPixelValue(x, y, val);
        }
    }
    return result;
}

const mp_obj_type_t microbit_image_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitImage,
    .print = microbit_image_print,
    .make_new = microbit_image_make_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = image_binary_op,
    .attr = NULL,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .stream_p = NULL,
    .bases_tuple = MP_OBJ_NULL,
    /* .locals_dict = */ (mp_obj_t)&microbit_image_locals_dict,
};

int SCALE_BRIGHTNESS[] = { 
   /* 0 */ 0, 
   /* 1 */ 1, 
   /* 2 */ 2, 
   /* 3 */ 4,
   /* 4 */ 7,
   /* 5 */ 13,
   /* 6 */ 24,
   /* 7 */ 43,
   /* 8 */ 78,
   /* 9 */ 141,
   /* 10 */ 255
};

// this is a very big hack: the class layout should match MicroBitImage
class MBCI {
public:
    int16_t width;
    int16_t height;
    int16_t *ref;
    const uint8_t *bitmap;
};

// only 1 const image can fake a MicroBitImage at a time
static int16_t fake_image_ref;
static MBCI fake_image_class;
static uint8_t fake_data[25];

/** Temporary  -- This WILL be deleted */
MicroBitImage *microbit_obj_get_image(mp_obj_t o) {
    if (mp_obj_get_type(o) == &microbit_image_type) {
        microbit_image_obj_t *img = (microbit_image_obj_t*)o;
        if (img->base.monochrome) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "Cannot convert a non 5x5 monchrome image into a MicroBitImage. Sorry."));
        }
        fake_image_class.ref = &fake_image_ref;
        fake_image_ref = 10; // just to make sure the data is never freed
        if (img->base.five) {
            fake_image_class.width = 5;
            fake_image_class.height = 5;
            for (int y = 0; y < 5; ++y) {
                for (int x = 0; x < 5; ++x) {
                    if (img->monochrome_5by5.getPixelValue(x, y)) {
                        fake_data[y*5+x] = 255;
                    } else {
                        fake_data[y*5+x] = 0;
                    }
                }
            }
            fake_image_class.bitmap = fake_data;
        } else {
            fake_image_class.width = img->width();
            fake_image_class.height = img->height();
            fake_image_class.bitmap = img->greyscale.byte_data;
        }
        return (MicroBitImage*)&fake_image_class;
    } else {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "expecting an image"));
    }
}

}