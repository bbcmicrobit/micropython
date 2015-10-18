

typedef struct _microbit_display_obj_t {
    mp_obj_base_t base;
    uint8_t image_buffer[5][5];
    uint8_t row_brightness[MICROBIT_DISPLAY_COLUMN_COUNT];
    uint8_t previous_brightness;
    /* Current row for strobing */
    uint8_t strobe_row;
    /* boolean histogram of brightness in buffer */
    uint16_t brightnesses;
    uint16_t strobe_mask;
    
    void advanceRow();
    void renderRow();
    inline void setPinsForRow(uint8_t brightness);
    
} microbit_display_obj_t;

#define ASYNC_MODE_STOPPED 0
#define ASYNC_MODE_ANIMATION 1
#define ASYNC_MODE_CLEAR 2

extern microbit_display_obj_t microbit_display_obj;

void microbit_display_print(microbit_display_obj_t *display, microbit_image_obj_t *image);

void microbit_display_animate(microbit_display_obj_t *display, mp_obj_t iterable, mp_int_t delay, bool wait, bool loop);

void microbit_display_scroll(microbit_display_obj_t *display, const char* str, mp_int_t len, bool wait);

mp_int_t microbit_display_get_pixel(microbit_display_obj_t *display, mp_int_t x, mp_int_t y);

void microbit_display_set_pixel(microbit_display_obj_t *display, mp_int_t x, mp_int_t y, mp_int_t val);


void microbit_display_clear(void);