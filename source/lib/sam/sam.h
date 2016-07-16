#ifndef SAM_H
#define SAM_H

#define DEFAULT_SING     false
#define DEFAULT_PITCH    64
#define DEFAULT_SPEED    72
#define DEFAULT_MOUTH    128
#define DEFAULT_THROAT   128

typedef struct _phoneme_t {
    unsigned char index;
    unsigned char length;
    unsigned char stress; //numbers from 0 to 8
    unsigned char pitch;
} phoneme_t;

enum {
    PHONEME_IGNORE=0,
    PHONEME_END=127,
    PHONEME_END_BREATH=126
};

#define RENDER_FRAMES 256

#define INPUT_PHONEMES 128
#define OUTPUT_PHONEMES (RENDER_FRAMES/4)

typedef struct _prepare_memory {
    const char *input;
    unsigned int input_length;
    phoneme_t phoneme_input[INPUT_PHONEMES];
} prepare_memory;


typedef struct _common_memory {
    unsigned char speed;
    unsigned char pitch;
    unsigned char mouth;
    unsigned char throat;
    int singmode;
    phoneme_t phoneme_output[OUTPUT_PHONEMES];
} common_memory;

typedef struct _render_freq_amp_t {
    unsigned int freq1:6;
    unsigned int freq2:7;
    unsigned int freq3:7;
    unsigned int amp1:4;
    unsigned int amp2:4;
    unsigned int amp3:4;
} render_freq_amp_t;

typedef struct _render_memory {
    render_freq_amp_t freq_amp[RENDER_FRAMES];
    unsigned char pitch[RENDER_FRAMES];
    unsigned char flags[RENDER_FRAMES];
} render_memory;

typedef struct _sam_memory {
    common_memory common;
    prepare_memory prepare;
    render_memory render;
} sam_memory;

void SetInput(sam_memory* mem, const char *_input, unsigned int len);

int SAMMain(sam_memory* mem);

extern char *sam_error;

char* GetBuffer();
int GetBufferLength();

//char input[]={"/HAALAOAO MAYN NAAMAEAE IHSTT SAEBAASTTIHAAN \x9b\x9b\0"};
//unsigned char input[]={"/HAALAOAO \x9b\0"};
//unsigned char input[]={"AA \x9b\0"};
//unsigned char input[] = {"GUH5DEHN TAEG\x9b\0"};

//unsigned char input[]={"AY5 AEM EY TAO4LXKIHNX KAX4MPYUX4TAH. GOW4 AH/HEH3D PAHNK.MEYK MAY8 DEY.\x9b\0"};
//unsigned char input[]={"/HEH3LOW2, /HAW AH YUX2 TUXDEY. AY /HOH3P YUX AH FIYLIHNX OW4 KEY.\x9b\0"};
//unsigned char input[]={"/HEY2, DHIHS IH3Z GREY2T. /HAH /HAH /HAH.AYL BIY5 BAEK.\x9b\0"};
//unsigned char input[]={"/HAH /HAH /HAH \x9b\0"};
//unsigned char input[]={"/HAH /HAH /HAH.\x9b\0"};
//unsigned char input[]={".TUW BIY5Y3,, OHR NAA3T - TUW BIY5IYIY., DHAE4T IHZ DHAH KWEH4SCHAHN.\x9b\0"};
//unsigned char input[]={"/HEY2, DHIHS \x9b\0"};

//unsigned char input[]={" IYIHEHAEAAAHAOOHUHUXERAXIX  \x9b\0"};
//unsigned char input[]={" RLWWYMNNXBDGJZZHVDH \x9b\0"};
//unsigned char input[]={" SSHFTHPTKCH/H \x9b\0"};

//unsigned char input[]={" EYAYOYAWOWUW ULUMUNQ YXWXRXLX/XDX\x9b\0"};


#endif

