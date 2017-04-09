/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

/* The following code has been copied from the DAL (https://github.com/lancaster-university/microbit-dal)
 * and modified slightly to:
 *   Have a C api
 *   Not use the BLE softdevice code.
 *   Make sure that seed is not set to zero accidentally.
 *  (Mark Shannon 2017)
 */

extern "C" {

#include "device.h"
#include "nrf51_bitfields.h"


static uint32_t randomValue;

/**
  * Generate a random number in the given range.
  * We use a simple Galois LFSR random number generator here,
  * as a Galois LFSR is sufficient for our applications, and much more lightweight
  * than the hardware random number generator built int the processor, which takes
  * a long time and uses a lot of energy.
  *
  * KIDS: You shouldn't use this is the real world to generte cryptographic keys though...
  * have a think why not. :-)
  *
  * @param max the upper range to generate a number for. This number cannot be negative
  * @return A random, natural number between 0 and the max-1. Or MICROBIT_INVALID_VALUE (defined in ErrorNo.h) if max is <= 0.
  *
  * Example:
  * @code
  * uBit.random(200); //a number between 0 and 199
  * @endcode
  */
int MicroBit_random(int max)
{
    uint32_t m, result;

    // Our maximum return value is actually one less than passed
    max--;

    do {
        m = (uint32_t)max;
        result = 0;
        do {
            // Cycle the LFSR (Linear Feedback Shift Register).
            // We use an optimal sequence with a period of 2^32-1, as defined by Bruce Schneier here (a true legend in the field!),
            // For those interested, it's documented in his paper:
            // "Pseudo-Random Sequence Generator for 32-Bit CPUs: A fast, machine-independent generator for 32-bit Microprocessors"
            // https://www.schneier.com/paper-pseudorandom-sequence.html
            uint32_t rnd = randomValue;

            rnd = ((((rnd >> 31)
                          ^ (rnd >> 6)
                          ^ (rnd >> 4)
                          ^ (rnd >> 2)
                          ^ (rnd >> 1)
                          ^ rnd)
                          & 0x0000001)
                          << 31 )
                          | (rnd >> 1);

            randomValue = rnd;

            result = ((result << 1) | (rnd & 0x00000001));
        } while(m >>= 1);
    } while (result > (uint32_t)max);


    return result;
}


/**
  * Seed our a random number generator (RNG).
  * We use the NRF51822 in built cryptographic random number generator to seed a Galois LFSR.
  * We do this as the hardware RNG is relatively high power, and use the the BLE stack internally,
  * with a less than optimal application interface. A Galois LFSR is sufficient for our
  * applications, and much more lightweight.
  */
void MicroBit_seedRandom(void)
{
    randomValue = 0;

    // Start the Random number generator. No need to leave it running... I hope. :-)
    NRF_RNG->TASKS_START = 1;
    do {
        for(int i = 0; i < 4; i++)
        {
            // Clear the VALRDY EVENT
            NRF_RNG->EVENTS_VALRDY = 0;

            // Wait for a number ot be generated.
            while(NRF_RNG->EVENTS_VALRDY == 0);

            randomValue = (randomValue << 8) | ((int) NRF_RNG->VALUE);
        }
        // PRNG won't work if seed is zero.
    } while(randomValue == 0);

    // Disable the generator to save power.
    NRF_RNG->TASKS_STOP = 1;
}


/**
  * Seed our pseudo random number generator (PRNG) using the given 32-bit value.
  */
void MicroBit_setSeed(uint32_t seed)
{
    randomValue = seed;
}

}