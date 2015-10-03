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

#include "MicroBit.h"
#include "microbitobj.h"

extern "C" {

#include "py/runtime.h"
#include "modmicrobit.h"

typedef struct _microbit_const_image_obj_t {
    mp_obj_base_t base;
    uint8_t width;
    uint8_t height;
    const uint8_t *data;
} microbit_const_image_obj_t;

STATIC void microbit_const_image_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    microbit_const_image_obj_t *self = (microbit_const_image_obj_t*)self_in;
    for (int y = 0; y < self->height; ++y) {
        for (int x = 0; x < self->width; ++x) {
            if (x > 0) {
                mp_printf(print, ",");
            }
            mp_printf(print, "%u", self->data[y * self->width + x]);
        }
        mp_printf(print, "\n");
    }
}

mp_obj_t microbit_const_image_width(mp_obj_t self_in) {
    microbit_const_image_obj_t *self = (microbit_const_image_obj_t*)self_in;
    return MP_OBJ_NEW_SMALL_INT(self->width);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_const_image_width_obj, microbit_const_image_width);

mp_obj_t microbit_const_image_height(mp_obj_t self_in) {
    microbit_const_image_obj_t *self = (microbit_const_image_obj_t*)self_in;
    return MP_OBJ_NEW_SMALL_INT(self->height);
}
MP_DEFINE_CONST_FUN_OBJ_1(microbit_const_image_height_obj, microbit_const_image_height);

mp_obj_t microbit_const_image_get_pixel(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
    microbit_const_image_obj_t *self = (microbit_const_image_obj_t*)self_in;
    mp_int_t x = mp_obj_get_int(x_in);
    mp_int_t y = mp_obj_get_int(y_in);
    if (x >= 0 && x < self->width && y >= 0 && y < self->height) {
        return MP_OBJ_NEW_SMALL_INT(self->data[y * self->width + x]);
    } else {
        return MP_OBJ_NEW_SMALL_INT(MICROBIT_INVALID_VALUE);
    }
}
MP_DEFINE_CONST_FUN_OBJ_3(microbit_const_image_get_pixel_obj, microbit_const_image_get_pixel);

STATIC const uint8_t heart_data[] = {
    0,1,0,1,0,
    1,1,1,1,1,
    1,1,1,1,1,
    0,1,1,1,0,
    0,0,1,0,0,
};
const microbit_const_image_obj_t microbit_const_image_heart_obj =
    {{&microbit_const_image_type}, 5, 5, heart_data};

STATIC const uint8_t heart_small_data[] = {
    0,0,0,0,0,
    0,1,0,1,0,
    0,1,1,1,0,
    0,0,1,0,0,
    0,0,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_heart_small_obj =
    {{&microbit_const_image_type}, 5, 5, heart_small_data};

STATIC const uint8_t happy_data[] = {
    0,0,0,0,0,
    0,1,0,1,0,
    0,0,0,0,0,
    1,0,0,0,1,
    0,1,1,1,0,
};
const microbit_const_image_obj_t microbit_const_image_happy_obj =
    {{&microbit_const_image_type}, 5, 5, happy_data};

STATIC const uint8_t sad_data[] = {
    0,0,0,0,0,
    0,1,0,1,0,
    0,0,0,0,0,
    0,1,1,1,0,
    1,0,0,0,1,
};
const microbit_const_image_obj_t microbit_const_image_sad_obj =
    {{&microbit_const_image_type}, 5, 5, sad_data};

STATIC const uint8_t confused_data[] = {
    0,0,0,0,0,
    0,1,0,1,0,
    0,0,0,0,0,
    0,1,0,1,0,
    1,0,1,0,1,
};
const microbit_const_image_obj_t microbit_const_image_confused_obj =
    {{&microbit_const_image_type}, 5, 5, confused_data};

STATIC const uint8_t angry_data[] = {
    1,0,0,0,1,
    0,1,0,1,0,
    0,0,0,0,0,
    1,1,1,1,1,
    1,0,1,0,1,
};
const microbit_const_image_obj_t microbit_const_image_angry_obj =
    {{&microbit_const_image_type}, 5, 5, angry_data};

STATIC const uint8_t asleep_data[] = {
    0,0,0,0,0,
    1,1,0,1,1,
    0,0,0,0,0,
    0,1,1,1,0,
    0,0,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_asleep_obj =
    {{&microbit_const_image_type}, 5, 5, asleep_data};

STATIC const uint8_t surprised_data[] = {
    0,1,0,1,0,
    0,0,0,0,0,
    0,0,1,0,0,
    0,1,0,1,0,
    0,0,1,0,0,
};
const microbit_const_image_obj_t microbit_const_image_surprised_obj =
    {{&microbit_const_image_type}, 5, 5, surprised_data};

STATIC const uint8_t yes_data[] = {
    0,0,0,0,0,
    0,0,0,0,1,
    0,0,0,1,0,
    1,0,1,0,0,
    0,1,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_yes_obj =
    {{&microbit_const_image_type}, 5, 5, yes_data};

STATIC const uint8_t no_data[] = {
    1,0,0,0,1,
    0,1,0,1,0,
    0,0,1,0,0,
    0,1,0,1,0,
    1,0,0,0,1,
};
const microbit_const_image_obj_t microbit_const_image_no_obj =
    {{&microbit_const_image_type}, 5, 5, no_data};

STATIC const uint8_t clock12_data[] = {
    0,0,1,0,0,
    0,0,1,0,0,
    0,0,1,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_clock12_obj =
    {{&microbit_const_image_type}, 5, 5, clock12_data};

STATIC const uint8_t clock1_data[] = {
    0,0,0,1,0,
    0,0,0,1,0,
    0,0,1,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_clock1_obj =
    {{&microbit_const_image_type}, 5, 5, clock1_data};

STATIC const uint8_t clock2_data[] = {
    0,0,0,0,0,
    0,0,0,1,1,
    0,0,1,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_clock2_obj =
    {{&microbit_const_image_type}, 5, 5, clock2_data};

STATIC const uint8_t clock3_data[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,1,1,1,
    0,0,0,0,0,
    0,0,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_clock3_obj =
    {{&microbit_const_image_type}, 5, 5, clock3_data};

STATIC const uint8_t clock4_data[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,1,0,0,
    0,0,0,1,1,
    0,0,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_clock4_obj =
    {{&microbit_const_image_type}, 5, 5, clock4_data};

STATIC const uint8_t clock5_data[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,1,0,0,
    0,0,0,1,0,
    0,0,0,1,0,
};
const microbit_const_image_obj_t microbit_const_image_clock5_obj =
    {{&microbit_const_image_type}, 5, 5, clock5_data};

STATIC const uint8_t clock6_data[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,1,0,0,
    0,0,1,0,0,
    0,0,1,0,0,
};
const microbit_const_image_obj_t microbit_const_image_clock6_obj =
    {{&microbit_const_image_type}, 5, 5, clock6_data};

STATIC const uint8_t clock7_data[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,1,0,0,
    0,1,0,0,0,
    0,1,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_clock7_obj =
    {{&microbit_const_image_type}, 5, 5, clock7_data};

STATIC const uint8_t clock8_data[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    0,0,1,0,0,
    1,1,0,0,0,
    0,0,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_clock8_obj =
    {{&microbit_const_image_type}, 5, 5, clock8_data};

STATIC const uint8_t clock9_data[] = {
    0,0,0,0,0,
    0,0,0,0,0,
    1,1,1,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_clock9_obj =
    {{&microbit_const_image_type}, 5, 5, clock9_data};

STATIC const uint8_t clock10_data[] = {
    0,0,0,0,0,
    1,1,0,0,0,
    0,0,1,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_clock10_obj =
    {{&microbit_const_image_type}, 5, 5, clock10_data};

STATIC const uint8_t clock11_data[] = {
    0,1,0,0,0,
    0,1,0,0,0,
    0,0,1,0,0,
    0,0,0,0,0,
    0,0,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_clock11_obj =
    {{&microbit_const_image_type}, 5, 5, clock11_data};

STATIC const uint8_t arrow_n_data[] = {
    0,0,1,0,0,
    0,1,1,1,0,
    1,0,1,0,1,
    0,0,1,0,0,
    0,0,1,0,0,
};
const microbit_const_image_obj_t microbit_const_image_arrow_n_obj =
    {{&microbit_const_image_type}, 5, 5, arrow_n_data};

STATIC const uint8_t arrow_ne_data[] = {
    0,0,1,1,1,
    0,0,0,1,1,
    0,0,1,0,1,
    0,1,0,0,0,
    1,0,0,0,0,
};
const microbit_const_image_obj_t microbit_const_image_arrow_ne_obj =
    {{&microbit_const_image_type}, 5, 5, arrow_ne_data};

STATIC const uint8_t arrow_e_data[] = {
    0,0,1,0,0,
    0,0,0,1,0,
    1,1,1,1,1,
    0,0,0,1,0,
    0,0,1,0,0,
};
const microbit_const_image_obj_t microbit_const_image_arrow_e_obj =
    {{&microbit_const_image_type}, 5, 5, arrow_e_data};

STATIC const uint8_t arrow_se_data[] = {
    1,0,0,0,0,
    0,1,0,0,0,
    0,0,1,0,1,
    0,0,0,1,1,
    0,0,1,1,1,
};
const microbit_const_image_obj_t microbit_const_image_arrow_se_obj =
    {{&microbit_const_image_type}, 5, 5, arrow_se_data};

STATIC const uint8_t arrow_s_data[] = {
    0,0,1,0,0,
    0,0,1,0,0,
    1,0,1,0,1,
    0,1,1,1,0,
    0,0,1,0,0,
};
const microbit_const_image_obj_t microbit_const_image_arrow_s_obj =
    {{&microbit_const_image_type}, 5, 5, arrow_s_data};

STATIC const uint8_t arrow_sw_data[] = {
    0,0,0,0,1,
    0,0,0,1,0,
    1,0,1,0,0,
    1,1,0,0,0,
    1,1,1,0,0,
};
const microbit_const_image_obj_t microbit_const_image_arrow_sw_obj =
    {{&microbit_const_image_type}, 5, 5, arrow_sw_data};

STATIC const uint8_t arrow_w_data[] = {
    0,0,1,0,0,
    0,1,0,0,0,
    1,1,1,1,1,
    0,1,0,0,0,
    0,0,1,0,0,
};
const microbit_const_image_obj_t microbit_const_image_arrow_w_obj =
    {{&microbit_const_image_type}, 5, 5, arrow_w_data};

STATIC const uint8_t arrow_nw_data[] = {
    1,1,1,0,0,
    1,1,0,0,0,
    1,0,1,0,0,
    0,0,0,1,0,
    0,0,0,0,1,
};
const microbit_const_image_obj_t microbit_const_image_arrow_nw_obj =
    {{&microbit_const_image_type}, 5, 5, arrow_nw_data};

STATIC const mp_map_elem_t microbit_const_image_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_width), (mp_obj_t)&microbit_const_image_width_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_height), (mp_obj_t)&microbit_const_image_height_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_pixel), (mp_obj_t)&microbit_const_image_get_pixel_obj },
};

STATIC MP_DEFINE_CONST_DICT(microbit_const_image_locals_dict, microbit_const_image_locals_dict_table);

const mp_obj_type_t microbit_const_image_type = {
    { &mp_type_type },
    .name = MP_QSTR_MicroBitImage, // MicroBitConstImage?
    .print = microbit_const_image_print,
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
    .bases_tuple = MP_OBJ_NULL,
    .locals_dict = (mp_obj_t)&microbit_const_image_locals_dict,
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

MicroBitImage *microbit_obj_get_const_image(mp_obj_t o) {
    if (mp_obj_get_type(o) == &microbit_const_image_type) {
        microbit_const_image_obj_t *img = (microbit_const_image_obj_t*)o;
        fake_image_class.width = img->width;
        fake_image_class.height = img->height;
        fake_image_class.ref = &fake_image_ref;
        fake_image_class.bitmap = img->data;
        fake_image_ref = 10; // just to make sure the data is never freed
        return (MicroBitImage*)&fake_image_class;
    } else {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "expecting an image"));
    }
}

}
