#include<stdio.h>
#include "sam.h"

extern unsigned char signInputTable1[];
extern unsigned char signInputTable2[];

void PrintPhonemes(char* title, phoneme_t *phonemes)
{
	int i = 0;
	printf("===========================================\r\n");

	printf("%s:\r\n\r\n", title);
	printf(" idx    phoneme  length  stress\r\n");
	printf("------------------------------\r\n");

	while((phonemes[i].index != PHONEME_END) && (i < 255))
	{
		if (phonemes[i].index < 81)
		{
			printf(" %3i      %c%c      %3i       %i\r\n",
			phonemes[i].index,
			signInputTable1[phonemes[i].index],
			signInputTable2[phonemes[i].index],
			phonemes[i].length,
			phonemes[i].stress
			);
		} else
		{
			printf(" %3i      ??      %3i       %i\r\n", phonemes[i].index, phonemes[i].length, phonemes[i].stress);
		}
		i++;
	}
	printf("===========================================\r\n");
	printf("\r\n");
}

void PrintOutput(unsigned char *flags, render_freq_amp_t *frames, unsigned char *pitches, unsigned char count)
{
	printf("===========================================\r\n");
	printf("Final data for speech output. %i frames:\r\n\r\n", count);
	int i = 0;
	printf(" flags ampl1 freq1 ampl2 freq2 ampl3 freq3 pitch\r\n");
	printf("------------------------------------------------\r\n");
	while(i < count)
	{
        render_freq_amp_t frame = frames[i];
		printf("%5i %5i %5i %5i %5i %5i %5i %5i\r\n", flags[i], frame.amp1, frame.freq1, frame.amp2, frame.freq2, frame.amp3, frame.freq3, pitches[i]);
		i++;
	}
	printf("===========================================\r\n");

}

/* For debugging or modifying reciter rules ...

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
	printf("\r\n");
}
*/
