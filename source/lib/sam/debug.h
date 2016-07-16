#ifndef DEBUG_H
#define DEBUG_H

#include "sam.h"

void PrintPhonemes(char* title, phoneme_t *phonemes);
void PrintOutput(unsigned char *flags, render_freq_amp_t *frames, unsigned char *pitches, unsigned char count);

void PrintRule(int offset);

#endif