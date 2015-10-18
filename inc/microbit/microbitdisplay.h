

typedef struct _microbit_display_obj_t {
    mp_obj_base_t base;
    /* boolean histogram of brightness in buffer */
    uint16_t brightnesses;
    uint8_t image_buffer[5][5];
} microbit_display_obj_t;


extern microbit_display_obj_t microbit_display_obj;

void microbit_display_print(microbit_display_obj_t *display, microbit_image_obj_t *image);

void microbit_display_animate(microbit_display_obj_t *display, mp_obj_t iterable, mp_int_t delay, bool wait, bool loop);

void microbit_display_scroll(microbit_display_obj_t *display, const char* str, mp_int_t len, bool wait);

mp_int_t microbit_display_get_pixel(microbit_display_obj_t *display, mp_int_t x, mp_int_t y);

void microbit_display_set_pixel(microbit_display_obj_t *display, mp_int_t x, mp_int_t y, mp_int_t val);


void microbit_display_clear(void);