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

/* Lava
 * 
 * WS2812B Tricolor LED (neopixel) controller
 *
 *
 * Example code:
 
	neopixel_strip_t m_strip;
	uint8_t dig_pin_num = 6;
	uint8_t leds_per_strip = 24;
	uint8_t error;
	uint8_t led_to_enable = 10;
	uint8_t red = 255;
	uint8_t green = 0;
	uint8_t blue = 159;

	neopixel_init(&m_strip, dig_pin_num, leds_per_strip);
	neopixel_clear(&m_strip);
	error = neopixel_set_color_and_show(&m_strip, led_to_enable, red, green, blue);
	if (error) {
		//led_to_enable was not within number leds_per_strip
	}
	//clear and remove strip
	neopixel_clear(&m_strip);
	neopixel_destroy(&m_strip);
 
 
 * For use with BLE stack, see information below:
	- Include in main.c
		#include "ble_radio_notification.h"
	- Call (see nrf_soc.h: NRF_RADIO_NOTIFICATION_DISTANCES and NRF_APP_PRIORITIES)
		ble_radio_notification_init(NRF_APP_PRIORITY_xxx,
									NRF_RADIO_NOTIFICATION_DISTANCE_xxx,
									your_radio_callback_handler);
	- Create 
		void your_radio_callback_handler(bool radio_active)
		{
			if (radio_active == false)
			{
				neopixel_show(&strip1);
				neopixel_show(&strip2);
				//...etc
			}
		}
	- Do not use neopixel_set_color_and_show(...) with BLE, instead use uint8_t neopixel_set_color(...);
 */
 
 
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "nrf_gpio.h"
#include "lib/neopixel.h"

void neopixel_init(neopixel_strip_t *strip, uint8_t pin_num, uint16_t num_leds)
{
	strip->leds = (color_t*) malloc(sizeof(color_t) * num_leds);
	strip->pin_num = pin_num;
	strip->num_leds = num_leds;
	nrf_gpio_cfg_output(pin_num);
	NRF_GPIO->OUTCLR = (1UL << pin_num);
	for (int i = 0; i < num_leds; i++)
	{	
		strip->leds[i].simple.g = 0;
		strip->leds[i].simple.r = 0;
		strip->leds[i].simple.b = 0;
	}
}

void neopixel_clear(neopixel_strip_t *strip)
{
		for (int i = 0; i < strip->num_leds; i++)
		{
			strip->leds[i].simple.g = 0;
			strip->leds[i].simple.r = 0;
			strip->leds[i].simple.b = 0;
		}
			neopixel_show(strip);
}


/*
Be Aware - this function runs mostly with interrupts turned off
*/
void neopixel_show(neopixel_strip_t *strip)
{
	NRF_GPIO->OUTCLR = (1UL << strip->pin_num);
	uint8_t* Data_address = (uint8_t*) strip->leds; /* This cast is because Dave is a lazy person - it really is byte at the end */
	uint16_t num_leds = strip->num_leds;
	uint32_t pin = (1UL << strip->pin_num);
	//Store current status of registers we are likely to clobber.
	__ASM("push {r0,r1,r2,r3,r4,r5,r6}\n\t");

	//disable interrupts whilst we mess with timing critical waggling.
	uint32_t irq_state = __get_PRIMASK();
	__disable_irq();
	/*
	Setup the initial values
	r1 = Pin mask in the GPIO register
	r2 = GPIO clear register
	r3 = GPIO SET
	r4 = Address pointer for the data - we cast this as a byte earlier because it really is.
	r5 = Length of the data (number of LEDS * 3 bytes per LED) - If trying to add RGB+W this sum might need to be done conditionally
	r6 = Parallel to serial conversion mask

	The asm loads the GPIO output address to write to in order to set a bit (0x50000508) into r3.
	It then loads r2 by adding 4 to the value in r3 - the offset from there to the GPIO address to clear a bit.
	*/

	__ASM volatile("mov r3, %[value2]\n\t" 
					"add r2, r3,#4\n\t" 
					"mov r4, %[value3]\n\t" 
					"mov r5, %[value4]\n\t"
					"mov r1, %[value5]\n\t" :: [value2] "r" (&NRF_GPIO->OUTSET),
											   [value3] "r" (Data_address), 
											   [value4] "r" (num_leds*3), 
											   [value5] "r" (pin): "r1", "r2", "r3","r4","r5");
	/*
	This code serialises the data bits for each LED.
	The data byte is loaded in the common section (label .common) and then each bit is masked and tested for '0' (label .nextbit)
	If it is a '0' we turn off the pin asap and then move to the code that advances to the next bit/byte. If a '1' we leave the pin on and do the same thing.
	If the mask (r6) is still valid then we are still moving out the current byte, so repeat.
	If it is '0' then we have done this byte and need to load the next byte from the pointer in r4. 
	r5 contains the count of bytes - calculated above from num LEDs * 3 bytes per LED.
	--If this code needs to do RGB+W LEDS then that will need to be addressed.
	Once we run r5 down to '0' we exit the data shifting and return.
	*/
     __ASM volatile("b .start\n\t"
	   
	".nextbit:\n\t"
		"str r1, [r3, #0]\n\t"
		"tst r6, r0\n\t"
		"bne .bitisone\n\t"
		"str r1, [r2, #0]\n\t"
	".bitisone:\n\t"
		"lsr r6, #1\n\t"
		"bne .justbit\n\t"
	
		"add r4, #1\n\t"
		"sub r5, #1\n\t"
		"beq .stop\n\t"
		
	".start:\n\t"
		"movs r6, #0x80\n\t"
		"nop\n\t"

	".common:\n\t"
		"str r1, [r2, #0]\n\t"
		"ldrb r0, [r4,#0]\n\t"
		"b .nextbit\n\t"

	".justbit:\n\t"
		"b .common\n\t"

	".stop:\n\t"
		"str r1, [r2, #0]\n\t"

    "pop {r0,r1,r2,r3,r4,r5,r6}\n\t");
    __set_PRIMASK(irq_state);
}

uint8_t neopixel_set_color(neopixel_strip_t *strip, uint16_t index, uint8_t red, uint8_t green, uint8_t blue )
{
		if (index < strip->num_leds)
		{
			strip->leds[index].simple.r = red;
			strip->leds[index].simple.g = green;
			strip->leds[index].simple.b = blue;
		}
		else
				return 1;
		return 0;
}

uint8_t neopixel_set_color_and_show(neopixel_strip_t *strip, uint16_t index, uint8_t red, uint8_t green, uint8_t blue)
{
		if (index < strip->num_leds)
		{
			strip->leds[index].simple.r = red;
			strip->leds[index].simple.g = green;
			strip->leds[index].simple.b = blue;
		  neopixel_show(strip);	
		}
		else
				return 1;
		return 0;
}

void neopixel_destroy(neopixel_strip_t *strip)
{
	free(strip->leds);
	strip->num_leds = 0;
	strip->pin_num = 0;
}


