
#ifndef __MICROPY_INCLUDED_MICROBIT_MICROBITBUTTON_H__
#define __MICROPY_INCLUDED_MICROBIT_MICROBITBUTTON_H__

typedef struct _microbit_button_obj_t {
    mp_obj_base_t base;
    MicroBitButton *button;
    /* Stores pressed count in top 31 bits and was_pressed in the low bit */
    mp_uint_t pressed;
} microbit_button_obj_t;

microbit_button_obj_t *microbit_get_button_by_id(uint8_t);

#endif // __MICROPY_INCLUDED_MICROBIT_MICROBITBUTTON_H__
