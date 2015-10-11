   
#define MAX_BRIGHTNESS 9

/** Monochrome images are immutable, which means that 
 * we only need one bit per pixel which saves quite a lot
 * of memory */

/* we reserve a couple of bits, so we won't need to modify the
 * layout if we need to add more functionality or subtypes. */ 
#define TYPE_AND_FLAGS \
    mp_obj_base_t base; \
    uint8_t five:1; \
    uint8_t reserved1:1; \
    uint8_t reserved2:1

typedef struct _image_base_t {
    TYPE_AND_FLAGS;
} image_base_t;

typedef struct _monochrome_5by5_t {
    TYPE_AND_FLAGS;
    uint8_t pixel44: 1;
    uint8_t bits24[3];
    
    /* This is an internal method it is up to the caller to validate the inputs */
    int getPixelValue(mp_int_t x, mp_int_t y);

} monochrome_5by5_t;

typedef struct _greyscale_t {
    TYPE_AND_FLAGS;
    uint8_t height;
    uint8_t width;
    uint8_t byte_data[]; /* Static initializer for this will have to be C, not C++ */
    void clear();
    
    /* Thiese are internal methods and it is up to the caller to validate the inputs */
    int getPixelValue(mp_int_t x, mp_int_t y);
    void setPixelValue(mp_int_t x, mp_int_t y, mp_int_t val);
} greyscale_t;

typedef union _microbit_image_obj_t {
    image_base_t base;
    monochrome_5by5_t monochrome_5by5;
    greyscale_t greyscale;
    
    mp_int_t height();
    mp_int_t width();
    greyscale_t *copy();
    greyscale_t *invert();
    greyscale_t *shiftLeft(mp_int_t n);
    greyscale_t *shiftUp(mp_int_t n);
    
    /* This is an internal method it is up to the caller to validate the inputs */
    int getPixelValue(mp_int_t x, mp_int_t y);
    
} microbit_image_obj_t;

microbit_image_obj_t *microbit_image_for_char(char c);
mp_obj_t microbit_image_slice(microbit_image_obj_t *img, mp_int_t start, mp_int_t width, mp_int_t stride);
mp_obj_t scrolling_string_image_iterable(mp_obj_t str);

#define SMALL_IMAGE(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p44) \
{ \
    { &microbit_image_type }, \
    1, 0, 0, (p44), \
    { \
        (p0)|((p1)<<1)|((p2)<<2)|((p3)<<3)|((p4)<<4)|((p5)<<5)|((p6)<<6)|((p7)<<7), \
        (p8)|((p9)<<1)|((p10)<<2)|((p11)<<3)|((p12)<<4)|((p13)<<5)|((p14)<<6)|((p15)<<7), \
        (p16)|((p17)<<1)|((p18)<<2)|((p19)<<3)|((p20)<<4)|((p21)<<5)|((p22)<<6)|((p23)<<7) \
    } \
}

extern int BRIGHTNESS_SCALE[];

extern monochrome_5by5_t BLANK_IMAGE;