#include<stdio.h>
#include "sam.h"

extern unsigned char signInputTable1[];
extern unsigned char signInputTable2[];

void PrintPhonemes(phoneme_t *phonemes)
{
	int i = 0;
	printf("===========================================\n");

	printf("Internal Phoneme presentation:\n\n");
	printf(" idx    phoneme  length  stress\n");
	printf("------------------------------\n");

	while((phonemes[i].index != PHONEME_END) && (i < 255))
	{
		if (phonemes[i].index < 81)
		{
			printf(" %3i      %c%c      %3i       %i\n",
			phonemes[i].index,
			signInputTable1[phonemes[i].index],
			signInputTable2[phonemes[i].index],
			phonemes[i].length,
			phonemes[i].stress
			);
		} else
		{
			printf(" %3i      ??      %3i       %i\n", phonemes[i].index, phonemes[i].length, phonemes[i].stress);
		}
		i++;
	}
	printf("===========================================\n");
	printf("\n");
}

void PrintOutput(unsigned char *flags, render_freq_amp_t *frames, unsigned char *pitches)
{
	printf("===========================================\n");
	printf("Final data for speech output:\n\n");
	int i = 0;
	printf(" flags ampl1 freq1 ampl2 freq2 ampl3 freq3 pitch\n");
	printf("------------------------------------------------\n");
	while(i < 255)
	{
        render_freq_amp_t frame = frames[i];
		printf("%5i %5i %5i %5i %5i %5i %5i %5i\n", flags[i], frame.amp1, frame.freq1, frame.amp2, frame.freq2, frame.amp3, frame.freq3, pitches[i]);
		i++;
	}
	printf("===========================================\n");

}

extern unsigned char GetRuleByte(unsigned short mem62, unsigned char Y);

void PrintRule(int offset)
{
	int i = 1;
	unsigned char A = 0;
	printf("Applying rule: ");
	do
	{
		A = GetRuleByte(offset, i);
		if ((A&127) == '=') printf(" -> "); else printf("%c", A&127);
		i++;
	} while ((A&128)==0);
	printf("\n");
}

