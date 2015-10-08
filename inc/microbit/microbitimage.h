   
#define MAX_BRIGHTNESS 9

/** Monochrome images are immutable, which means that 
 * we only need one bit per pixel which saves quite a lot
 * of memory */

#define TYPE_AND_FLAGS \
    mp_obj_base_t base; \
    uint8_t five:1; \
    uint8_t monochrome:1; \
    uint8_t greyscale: 1; \
    uint8_t reserved:1

typedef struct _image_base_t {
    TYPE_AND_FLAGS;
} image_base_t;

typedef struct _monochrome_5by5_t {
    TYPE_AND_FLAGS;
    uint8_t pixel44: 1;
    uint8_t bits24[3];
    
    /* This is an internal method it is up to the caller to validate the inputs */
    int getPixelValue(mp_int_t x, mp_int_t y);
    void printPixel(mp_int_t x, mp_int_t y, const mp_print_t *print);

} monochrome_5by5_t;

typedef struct _monochrome_t {
    TYPE_AND_FLAGS;
    uint8_t height;
    uint8_t width;
    uint8_t bit_data[]; /* Static initializer for this will have to be C, not C++ */
    
    /* This is an internal method it is up to the caller to validate the inputs */
    int getPixelValue(mp_int_t x, mp_int_t y);
    void printPixel(mp_int_t x, mp_int_t y, const mp_print_t *print);
    
} monochrome_t;

typedef struct _greyscale_t {
    TYPE_AND_FLAGS;
    uint8_t height;
    uint8_t width;
    uint8_t byte_data[]; /* Static initializer for this will have to be C, not C++ */
    
    /* This is an internal method it is up to the caller to validate the inputs */
    int getPixelValue(mp_int_t x, mp_int_t y);
    void printPixel(mp_int_t x, mp_int_t y, const mp_print_t *print);
    
    /* This should only be used in constructors */
    void setPixelValue(mp_int_t x, mp_int_t y, mp_int_t val);
} greyscale_t;

typedef union _microbit_image_obj_t {
    image_base_t base;
    monochrome_5by5_t monochrome_5by5;
    monochrome_t monochrome;
    greyscale_t greyscale;
    
    mp_int_t height();
    mp_int_t width();
    void printPixel(mp_int_t x, mp_int_t y, const mp_print_t *print);
    union _microbit_image_obj_t *copy();
    
    /* This is an internal method it is up to the caller to validate the inputs */
    int getPixelValue(mp_int_t x, mp_int_t y);
    
    /* These are modifying or incomplete and should only be used in constructors */
    union _microbit_image_obj_t *shiftLeft(mp_int_t n);
    union _microbit_image_obj_t *shiftUp(mp_int_t n);
} microbit_image_obj_t;

#define SMALL_IMAGE(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p44) \
{ \
    { &microbit_image_type }, \
    1, 0, 0, 0, (p44), \
    { \
        (p0)|((p1)<<1)|((p2)<<2)|((p3)<<3)|((p4)<<4)|((p5)<<5)|((p6)<<6)|((p7)<<7), \
        (p8)|((p9)<<1)|((p10)<<2)|((p11)<<3)|((p12)<<4)|((p13)<<5)|((p14)<<6)|((p15)<<7), \
        (p16)|((p17)<<1)|((p18)<<2)|((p19)<<3)|((p20)<<4)|((p21)<<5)|((p22)<<6)|((p23)<<7) \
    } \
}

extern int BRIGHTNESS_SCALE[];