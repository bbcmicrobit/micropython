#ifndef RECITER_C
#define RECITER_C

//int TextToPhonemes(char *input, char *output);

typedef struct _reciter_memory {
    char input[128];
    unsigned char inputtemp[128];   // secure copy of input tab36096
} reciter_memory;

int TextToPhonemes(reciter_memory *mem);

#endif

