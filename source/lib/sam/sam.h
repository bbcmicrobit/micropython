#ifndef SAM_H
#define SAM_H

#define DEFAULT_SING     false
#define DEFAULT_PITCH    64
#define DEFAULT_SPEED    72
#define DEFAULT_MOUTH    128
#define DEFAULT_THROAT   128

typedef struct _prepare_memory {
    char input[256];
    unsigned char stress[256]; //numbers from 0 to 8
    unsigned char phonemeLength[256];
    unsigned char phonemeindex[256];
} prepare_memory;

typedef struct _common_memory {
    unsigned char speed;
    unsigned char pitch;
    unsigned char mouth;
    unsigned char throat;
    int singmode;
    unsigned char phonemeIndexOutput[60];
    unsigned char stressOutput[60];
    unsigned char phonemeLengthOutput[60];
} common_memory;

typedef struct _render_memory {
    unsigned char pitches[256];
    unsigned char frequency1[256];
    unsigned char frequency2[256];
    unsigned char frequency3[256];
    unsigned char amplitude1[256];
    unsigned char amplitude2[256];
    unsigned char amplitude3[256];
    unsigned char sampledConsonantFlag[256];
} render_memory;


typedef struct _sam_memory {
    common_memory common;
    prepare_memory prepare;
    render_memory render;
} sam_memory;

void SetInput(sam_memory* mem, const char *_input, unsigned int len);

int SAMMain(sam_memory* mem);

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

