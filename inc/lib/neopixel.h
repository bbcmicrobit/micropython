/*

Original source of this neopixel driver: https://github.com/lavallc/nrf51-neopixel

The MIT License (MIT)

Copyright (c) 2013 Lava

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

 #ifndef NEOPIXEL_H
 #define NEOPIXEL_H


#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"


//These defines are timed specific to a series of if statements and will need to be changed
//to compensate for different writing algorithms than the one in neopixel.c
#define NEOPIXEL_SEND_ONE	NRF_GPIO->OUTSET = (1UL << PIN); \
		__ASM ( \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
			); \
		NRF_GPIO->OUTCLR = (1UL << PIN); \

#define NEOPIXEL_SEND_ZERO NRF_GPIO->OUTSET = (1UL << PIN); \
		__ASM (  \
				" NOP\n\t"  \
			);  \
		NRF_GPIO->OUTCLR = (1UL << PIN);  \
		__ASM ( \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
			);
		
extern void nrf_delay_us(uint32_t volatile number_of_us);
typedef union {
		struct {
			uint8_t g, r, b;
		}simple;
    uint8_t grb[3];
} color_t;

typedef struct {
	uint8_t pin_num;
	uint16_t num_leds;
	color_t *leds;
} neopixel_strip_t;

/**
  @brief Initialize GPIO and data location
  @param[in] pointer to Strip structure
	@param[in] pin number for GPIO
*/
void neopixel_init(neopixel_strip_t *strip, uint8_t pin_num, uint16_t num_leds);
	
/**
  @brief Turn all LEDs off
  @param[in] pointer to Strip structure
*/
void neopixel_clear(neopixel_strip_t *strip);

/**
  @brief Update strip with structure data
  @param[in] pointer to Strip structure
*/
void neopixel_show(neopixel_strip_t *strip);

/**
  @brief Write RGB value to LED structure
  @param[in] pointer to Strip structure
	@param[in] red value
	@param[in] green value
	@param[in] blue value
	@param[in] LED number (starting at 1)
  @retval 0 Successful write
  @retval 1 LED number is out of bounds
*/
uint8_t neopixel_set_color(neopixel_strip_t *strip, uint16_t index, uint8_t red, uint8_t green, uint8_t blue );


/**
  @brief Write RGB value to LED structure and update LED
  @param[in] pointer to Strip structure
	@param[in] red value
	@param[in] green value
	@param[in] blue value
	@param[in] LED number (starting at 1)
  @retval 0 Successful write
  @retval 1 LED number is out of bounds
*/
uint8_t neopixel_set_color_and_show(neopixel_strip_t *strip, uint16_t index, uint8_t red, uint8_t green, uint8_t blue);

/**
  @brief Clears structure data
  @param[in] pointer to Strip structure
*/
void neopixel_destroy(neopixel_strip_t *strip);

#endif
