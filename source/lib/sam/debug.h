#ifndef DEBUG_H
#define DEBUG_H

#include "sam.h"

void PrintPhonemes(phoneme_t *phonemes);
void PrintOutput(
	unsigned char *flag, 
	unsigned char *f1, 
	unsigned char *f2, 
	unsigned char *f3,
	unsigned char *a1, 
	unsigned char *a2, 
	unsigned char *a3,
	unsigned char *p);

void PrintRule(int offset);

#endif