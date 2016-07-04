#ifndef RENDER_H
#define RENDER_H

#include "sam.h"

void Render(sam_memory* sam);
void SetMouthThroat(unsigned char mouth, unsigned char throat);
void OutputFrames(sam_memory *sam, unsigned char frame_count);

/** Scaling c64 rate to sample rate */
// Rate for 22.05kHz
// #define SCALE_RATE(x) (((x)*1310)>>16)
// Rate for 7.8125KHz
#define SCALE_RATE(x) (((x)*420)>>16)

#endif
