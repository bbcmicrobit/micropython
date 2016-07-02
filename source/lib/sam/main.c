#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "reciter.h"
#include "sam.h"

int sam_main(const char *words, bool phonetic, bool sing, int pitch, int speed, int mouth, int throat)
{
	int i;
	char input[256];
	for(i=0; i<256; i++) input[i] = 0;
    strncat(input, words, 256);
    strncat(input, " ", 256);

    if(sing) {
        EnableSingmode();
    } else {
        DisableSingmode();
    }
    SetPitch(pitch);
    SetSpeed(speed);
    SetMouth(mouth);
    SetThroat(throat);

	for(i=0; input[i] != 0; i++)
		input[i] = toupper((int)input[i]);

	if (!phonetic)
	{
		strncat(input, "[", 256);
		if (!TextToPhonemes(input)) return 1;
	} else strncat(input, "\x9b", 256);

	SetInput(input);
    return SAMMain();
}
