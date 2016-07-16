#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "render.h"
#include "RenderTabs.h"

#include "debug.h"
#include "sam.h"
extern int debug;

void AddInflection(sam_memory* sam, unsigned char mem48, unsigned char phase1, unsigned char punctuation);


// contains the final soundbuffer
extern int bufferpos;

//timetable for more accurate c64 simulation
const int timetable[5][5] =
{
	{162, 167, 167, 127, 128},
	{226, 60, 60, 0, 0},
	{225, 60, 59, 0, 0},
	{200, 0, 0, 54, 55},
	{199, 0, 0, 54, 54}
};

extern void SamOutputByte(unsigned int pos, unsigned char b);

void Output(int index, unsigned char A)
{
    static unsigned oldtimetableindex = 0;
    bufferpos += timetable[oldtimetableindex][index];
    oldtimetableindex = index;
    SamOutputByte(bufferpos, (A & 15)*16);
}



//written by me because of different table positions.
// mem[47] = ...
// 168=pitches
// 169=frequency1
// 170=frequency2
// 171=frequency3
// 172=amplitude1
// 173=amplitude2
// 174=amplitude3
unsigned char Read(sam_memory* sam, unsigned char p, unsigned char Y)
{
    if (p > RENDER_FRAMES) {
        sam_error = "Out-of-buffer read";
        return 0;
    }
	switch(p)
	{
	case 168: return sam->render.pitch[Y];
	case 169: return sam->render.freq_amp[Y].freq1;
	case 170: return sam->render.freq_amp[Y].freq2;
	case 171: return sam->render.freq_amp[Y].freq3;
	case 172: return sam->render.freq_amp[Y].amp1;
	case 173: return sam->render.freq_amp[Y].amp2;
	case 174: return sam->render.freq_amp[Y].amp3;
	}
	sam_error = "Read error";
	return 0;
}

void Write(sam_memory* sam, unsigned char p, unsigned char Y, unsigned char value)
{
    if (p > RENDER_FRAMES) {
        sam_error = "Out-of-buffer write";
        return;
    }
	switch(p)
	{
	case 168: sam->render.pitch[Y] = value; return;
	case 169: sam->render.freq_amp[Y].freq1 = value;  return;
	case 170: sam->render.freq_amp[Y].freq2 = value;  return;
	case 171: sam->render.freq_amp[Y].freq3 = value;  return;
	case 172: sam->render.freq_amp[Y].amp1 = value;  return;
	case 173: sam->render.freq_amp[Y].amp2 = value;  return;
	case 174: sam->render.freq_amp[Y].amp3 = value;  return;
	}
    sam_error = "Write error";
}


// -------------------------------------------------------------------------
//Code48227
// Render a sampled sound from the sampleTable.
//
//   Phoneme   Sample Start   Sample End
//   32: S*    15             255
//   33: SH    257            511
//   34: F*    559            767
//   35: TH    583            767
//   36: /H    903            1023
//   37: /X    1135           1279
//   38: Z*    84             119
//   39: ZH    340            375
//   40: V*    596            639
//   41: DH    596            631
//
//   42: CH
//   43: **    399            511
//
//   44: J*
//   45: **    257            276
//   46: **
// 
//   66: P*
//   67: **    743            767
//   68: **
//
//   69: T*
//   70: **    231            255
//   71: **
//
// The SampledPhonemesTable[] holds flags indicating if a phoneme is
// voiced or not. If the upper 5 bits are zero, the sample is voiced.
//
// Samples in the sampleTable are compressed, with bits being converted to
// bytes from high bit to low, as follows:
//
//   unvoiced 0 bit   -> X
//   unvoiced 1 bit   -> 5
//
//   voiced 0 bit     -> 6
//   voiced 1 bit     -> 24
//
// Where X is a value from the table:
//
//   { 0x18, 0x1A, 0x17, 0x17, 0x17 };
//
// The index into this table is determined by masking off the lower
// 3 bits from the SampledPhonemesTable:
//
//        index = (SampledPhonemesTable[i] & 7) - 1;
//
// For voices samples, samples are interleaved between voiced output.


// Code48227()
unsigned char RenderSample(sam_memory* sam, unsigned char *mem66, unsigned sample, unsigned char pos)
{     
	int tempA;
	// current phoneme's index
    unsigned char mem47;
    unsigned char mem49 = pos;
    unsigned char mem53;
    unsigned char mem56;

	// mask low three bits and subtract 1 get value to 
	// convert 0 bits on unvoiced samples.
	unsigned char A = sample&7;
	unsigned char X = A-1;

    // store the result
	mem56 = X;
	
	// determine which offset to use from table { 0x18, 0x1A, 0x17, 0x17, 0x17 }
	// T, S, Z                0          0x18
	// CH, J, SH, ZH          1          0x1A
	// P, F*, V, TH, DH       2          0x17
	// /H                     3          0x17
	// /X                     4          0x17

    // get value from the table
    if (X >= sizeof(tab48426))
        sam_error = "Out-of-buffer read";
	mem53 = tab48426[X];
	mem47 = X;      //46016+mem[56]*256
	
	// voiced sample?
	A = sample & 248;
	if(A == 0)
	{
        // voiced phoneme: Z*, ZH, V*, DH
		pos = mem49;
		A = sam->render.pitch[9] >> 4;
		
		// jump to voiced portion
		goto pos48315;
	}
	
	pos = A ^ 255;
pos48274:
         
    // step through the 8 bits in the sample
	mem56 = 8;
	
	// get the next sample from the table
    // mem47*256 = offset to start of samples
	A = sampleTable[mem47*256+pos];
pos48280:

    // left shift to get the high bit
	tempA = A;
	A = A << 1;
	//48281: BCC 48290
	
	// bit not set?
	if ((tempA & 128) == 0)
	{
        // convert the bit to value from table
		X = mem53;
		//mem[54296] = X;
        // output the byte
		Output(1, X);
		// if X != 0, exit loop
		if(X != 0) goto pos48296;
	}
	
	// output a 5 for the on bit
	Output(2, 5);

	//48295: NOP
pos48296:

	X = 0;

    // decrement counter
	mem56--;
	
	// if not done, jump to top of loop
	if (mem56 != 0) goto pos48280;
	
	// increment position
	pos++;
	if (pos != 0) goto pos48274;
	
	// restore values and return
	pos = mem49;
	return pos;


	unsigned char phase1;

pos48315:
// handle voiced samples here

   // number of samples?
	phase1 = A ^ 255;

	pos = *mem66;
	do
	{
		//pos48321:

        // shift through all 8 bits
		mem56 = 8;
		//A = Read(sam, mem47, pos);
		
		// fetch value from table
		A = sampleTable[mem47*256+pos];

        // loop 8 times
		//pos48327:
		do
		{
			//48327: ASL A
			//48328: BCC 48337
			
			// left shift and check high bit
			tempA = A;
			A = A << 1;
			if ((tempA & 128) != 0)
			{
                // if bit set, output 26
				X = 26;
				Output(3, X);
			} else
			{
				//timetable 4
				// bit is not set, output a 6
				X=6;
				Output(4, X);
			}

			mem56--;
		} while(mem56 != 0);

        // move ahead in the table
		pos++;
		
		// continue until counter done
		phase1++;

	} while (phase1 != 0);
	//	if (phase1 != 0) goto pos48321;
	
	// restore values and return
	A = 1;
	*mem66 = pos;
	pos = mem49;
	return pos;
}



// RENDER THE PHONEMES IN THE LIST
//
// The phoneme list is converted into sound through the steps:
//
// 1. Copy each phoneme <length> number of times into the frames list,
//    where each frame represents 10 milliseconds of sound.
//
// 2. Determine the transitions lengths between phonemes, and linearly
//    interpolate the values across the frames.
//
// 3. Offset the pitches by the fundamental frequency.
//
// 4. Render the each frame.



//void Code47574()
void Render(sam_memory* sam)
{
	unsigned char phase1 = 0;  //mem43
	unsigned char phase2;
	unsigned char phase3;
    unsigned char mem38;
	unsigned char mem40;
	unsigned char speedcounter; //mem45
    unsigned char mem47;
    unsigned char mem48;
    unsigned char mem49;
    unsigned char mem50;
    unsigned char mem51;
    unsigned char mem53;
    unsigned char mem56;
    unsigned char mem44 = 0;
	int i;
	if (sam->common.phoneme_output[0].index == PHONEME_END) return; //exit if no data

	unsigned char A = 0;
	unsigned char X = 0;

// CREATE FRAMES
//
// The length parameter in the list corresponds to the number of frames
// to expand the phoneme to. Each frame represents 10 milliseconds of time.
// So a phoneme with a length of 7 = 7 frames = 70 milliseconds duration.
//
// The parameters are copied from the phoneme to the frame verbatim.

    do
    {
        // get the index
        unsigned char Y = mem44;
        // get the phoneme at the index
        A = sam->common.phoneme_output[mem44].index;
        mem56 = A;

        // if terminal phoneme, exit the loop
        if (A == PHONEME_END) break;

        // period phoneme *.
        if (A == 1)
        {
        // add rising inflection
            A = 1;
            mem48 = 1;
            //goto pos48376;
            AddInflection(sam, mem48, phase1, X);
        }
        /*
        if (A == 2) goto pos48372;
        */

        // question mark phoneme?
        if (A == 2)
        {
            // create falling inflection
            mem48 = 255;
            AddInflection(sam, mem48, phase1, X);
        }
        //	pos47615:

        // get the stress amount (more stress = higher pitch)
        phase1 = tab47492[sam->common.phoneme_output[Y].stress + 1];

        // get number of frames to write
        phase2 = sam->common.phoneme_output[Y].length;
        unsigned char pitch = sam->common.phoneme_output[Y].pitch;
        Y = mem56;

        // copy from the source to the frames list
        do
        {
            sam->render.freq_amp[X].freq1 = get_freq1(Y, sam->common.mouth);     // F1 frequency
            sam->render.freq_amp[X].freq2 = get_freq2(Y, sam->common.throat);     // F2 frequency
            sam->render.freq_amp[X].freq3 = freq3data[Y];     // F3 frequency
            sam->render.freq_amp[X].amp1 = ampl1data[Y];     // F1 amplitude
            sam->render.freq_amp[X].amp2 = ampl2data[Y];     // F2 amplitude
            sam->render.freq_amp[X].amp3 = ampl3data[Y];     // F3 amplitude
            sam->render.flags[X] = sampledConsonantFlags[Y];        // phoneme data for sampled consonants
            sam->render.pitch[X] = pitch + phase1;      // pitch
            X++;
            phase2--;
        } while(phase2 != 0);
        mem44++;
    } while(mem44 != OUTPUT_PHONEMES);


// -------------------
//pos47694:

// CREATE TRANSITIONS
//
// Linear transitions are now created to smoothly connect the
// end of one sustained portion of a phoneme to the following
// phoneme. 
//
// To do this, three tables are used:
//
//  Table         Purpose
//  =========     ==================================================
//  blendRank     Determines which phoneme's blend values are used.
//
//  blendOut      The number of frames at the end of the phoneme that
//                will be used to transition to the following phoneme.
//
//  blendIn       The number of frames of the following phoneme that
//                will be used to transition into that phoneme.
//
// In creating a transition between two phonemes, the phoneme
// with the HIGHEST rank is used. Phonemes are ranked on how much
// their identity is based on their transitions. For example, 
// vowels are and diphthongs are identified by their sustained portion, 
// rather than the transitions, so they are given low values. In contrast,
// stop consonants (P, B, T, K) and glides (Y, L) are almost entirely
// defined by their transitions, and are given high rank values.
//
// Here are the rankings used by SAM:
//
//     Rank    Type                         Phonemes
//     2       All vowels                   IY, IH, etc.
//     5       Diphthong endings            YX, WX, ER
//     8       Terminal liquid consonants   LX, WX, YX, N, NX
//     9       Liquid consonants            L, RX, W
//     10      Glide                        R, OH
//     11      Glide                        WH
//     18      Voiceless fricatives         S, SH, F, TH
//     20      Voiced fricatives            Z, ZH, V, DH
//     23      Plosives, stop consonants    P, T, K, KX, DX, CH
//     26      Stop consonants              J, GX, B, D, G
//     27-29   Stop consonants (internal)   **
//     30      Unvoiced consonants          /H, /X and Q*
//     160     Nasal                        M
//
// To determine how many frames to use, the two phonemes are 
// compared using the blendRank[] table. The phoneme with the 
// higher rank is selected. In case of a tie, a blend of each is used:
//
//      if blendRank[phoneme1] ==  blendRank[phomneme2]
//          // use lengths from each phoneme
//          outBlendFrames = outBlend[phoneme1]
//          inBlendFrames = outBlend[phoneme2]
//      else if blendRank[phoneme1] > blendRank[phoneme2]
//          // use lengths from first phoneme
//          outBlendFrames = outBlendLength[phoneme1]
//          inBlendFrames = inBlendLength[phoneme1]
//      else
//          // use lengths from the second phoneme
//          // note that in and out are SWAPPED!
//          outBlendFrames = inBlendLength[phoneme2]
//          inBlendFrames = outBlendLength[phoneme2]
//
// Blend lengths can't be less than zero.
//
// Transitions are assumed to be symetrical, so if the transition 
// values for the second phoneme are used, the inBlendLength and 
// outBlendLength values are SWAPPED.
//
// For most of the parameters, SAM interpolates over the range of the last
// outBlendFrames-1 and the first inBlendFrames.
//
// The exception to this is the Pitch[] parameter, which is interpolates the
// pitch from the CENTER of the current phoneme to the CENTER of the next
// phoneme.
//
// Here are two examples. First, For example, consider the word "SUN" (S AH N)
//
//    Phoneme   Duration    BlendWeight    OutBlendFrames    InBlendFrames
//    S         2           18             1                 3
//    AH        8           2              4                 4
//    N         7           8              1                 2
//
// The formant transitions for the output frames are calculated as follows:
//
//     flags ampl1 freq1 ampl2 freq2 ampl3 freq3 pitch
//    ------------------------------------------------
// S
//    241     0     6     0    73     0    99    61   Use S (weight 18) for transition instead of AH (weight 2)
//    241     0     6     0    73     0    99    61   <-- (OutBlendFrames-1) = (1-1) = 0 frames
// AH
//      0     2    10     2    66     0    96    59 * <-- InBlendFrames = 3 frames
//      0     4    14     3    59     0    93    57 *
//      0     8    18     5    52     0    90    55 *
//      0    15    22     9    44     1    87    53
//      0    15    22     9    44     1    87    53   
//      0    15    22     9    44     1    87    53   Use N (weight 8) for transition instead of AH (weight 2).
//      0    15    22     9    44     1    87    53   Since N is second phoneme, reverse the IN and OUT values.
//      0    11    17     8    47     1    98    56 * <-- (InBlendFrames-1) = (2-1) = 1 frames
// N
//      0     8    12     6    50     1   109    58 * <-- OutBlendFrames = 1
//      0     5     6     5    54     0   121    61
//      0     5     6     5    54     0   121    61
//      0     5     6     5    54     0   121    61
//      0     5     6     5    54     0   121    61
//      0     5     6     5    54     0   121    61
//      0     5     6     5    54     0   121    61
//
// Now, consider the reverse "NUS" (N AH S):
//
//     flags ampl1 freq1 ampl2 freq2 ampl3 freq3 pitch
//    ------------------------------------------------
// N
//     0     5     6     5    54     0   121    61
//     0     5     6     5    54     0   121    61
//     0     5     6     5    54     0   121    61
//     0     5     6     5    54     0   121    61
//     0     5     6     5    54     0   121    61   
//     0     5     6     5    54     0   121    61   Use N (weight 8) for transition instead of AH (weight 2)
//     0     5     6     5    54     0   121    61   <-- (OutBlendFrames-1) = (1-1) = 0 frames
// AH
//     0     8    11     6    51     0   110    59 * <-- InBlendFrames = 2
//     0    11    16     8    48     0    99    56 *
//     0    15    22     9    44     1    87    53   Use S (weight 18) for transition instead of AH (weight 2)
//     0    15    22     9    44     1    87    53   Since S is second phoneme, reverse the IN and OUT values.
//     0     9    18     5    51     1    90    55 * <-- (InBlendFrames-1) = (3-1) = 2
//     0     4    14     3    58     1    93    57 *
// S
//   241     2    10     2    65     1    96    59 * <-- OutBlendFrames = 1
//   241     0     6     0    73     0    99    61

	A = 0;
	mem44 = 0;
	mem49 = 0; // mem49 starts at as 0
	X = 0;
	while(1) //while No. 1
	{
         // get the current and following phoneme
		unsigned char Y = sam->common.phoneme_output[X].index;
        A = sam->common.phoneme_output[X+1].index;
		X++;

		// exit loop at end token
		if (A == PHONEME_END) break;//goto pos47970;
 
        // get the ranking of each phoneme
		X = A;
		mem56 = blendRank[A];
		A = blendRank[Y];
		
		// compare the rank - lower rank value is stronger
		if (A == mem56)
		{
            // same rank, so use out blend lengths from each phoneme
			phase1 = outBlendLength[Y];
			phase2 = outBlendLength[X];
		} else
		if (A < mem56)
		{
            // first phoneme is stronger, so us it's blend lengths
			phase1 = inBlendLength[X];
			phase2 = outBlendLength[X];
		} else
		{
            // second phoneme is stronger, so use it's blend lengths
            // note the out/in are swapped
			phase1 = outBlendLength[Y];
			phase2 = inBlendLength[Y];
		}

		Y = mem44;
		A = mem49 + sam->common.phoneme_output[mem44].length; // A is mem49 + length
		mem49 = A; // mem49 now holds length + position
		A = A + phase2; //Maybe Problem because of carry flag

		//47776: ADC 42
		speedcounter = A;
		mem47 = 168;
		phase3 = mem49 - phase1; // what is mem49
		A = phase1 + phase2; // total transition?
		mem38 = A;
		
		X = A;
		X -= 2;
		if ((X & 128) == 0)
		do   //while No. 2
		{
			//pos47810:

          // mem47 is used to index the tables:
          // 168  pitches[]
          // 169  frequency1
          // 170  frequency2
          // 171  frequency3
          // 172  amplitude1
          // 173  amplitude2
          // 174  amplitude3

			mem40 = mem38;

			if (mem47 == 168)     // pitch
			{
                      
               // unlike the other values, the pitches[] interpolates from 
               // the middle of the current phoneme to the middle of the 
               // next phoneme
                      
				unsigned char mem36, mem37;
				// half the width of the current phoneme
				mem36 = sam->common.phoneme_output[mem44].length >> 1;
				// half the width of the next phoneme
				mem37 = sam->common.phoneme_output[mem44+1].length >> 1;
				// sum the values
				mem40 = mem36 + mem37; // length of both halves
				mem37 += mem49; // center of next phoneme
				mem36 = mem49 - mem36; // center index of current phoneme
				A = Read(sam, mem47, mem37); // value at center of next phoneme - end interpolation value
				//A = mem[address];
				
				Y = mem36; // start index of interpolation
				mem53 = A - Read(sam, mem47, mem36); // value to center of current phoneme
			} else
			{
                // value to interpolate to
				A = Read(sam, mem47, speedcounter);
				// position to start interpolation from
				Y = phase3;
				// value to interpolate from
				mem53 = A - Read(sam, mem47, phase3);
			}
			
			
			// calculate change per frame
			mem50 = (((signed char)(mem53) < 0) ? 128 : 0);
			mem51 = abs((signed char)mem53) % mem40;
			mem53 = (unsigned char)((signed char)(mem53) / mem40);

            // interpolation range
			X = mem40; // number of frames to interpolate over
			Y = phase3; // starting frame


            // linearly interpolate values

			mem56 = 0;
			//47907: CLC
			//pos47908:
			while(1)     //while No. 3
			{
				A = Read(sam, mem47, Y) + mem53; //carry alway cleared

				mem48 = A;
				Y++;
				X--;
				if(X == 0) break;

				mem56 += mem51;
				if (mem56 >= mem40)  //???
				{
					mem56 -= mem40; //carry? is set
					//if ((mem56 & 128)==0)
					if ((mem50 & 128)==0)
					{
						//47935: BIT 50
						//47937: BMI 47943
						if(mem48 != 0) mem48++;
					} else mem48--;
				}
				//pos47945:
				Write(sam, mem47, Y, mem48);
			} //while No. 3

			//pos47952:
			mem47++;
			//if (mem47 != 175) goto pos47810;
		} while (mem47 != 175);     //while No. 2
		//pos47963:
		mem44++;
		X = mem44;
	}  //while No. 1

	//goto pos47701;
	//pos47970:

    // add the length of this phoneme
	mem48 = mem49 + sam->common.phoneme_output[mem44].length;
	

// ASSIGN PITCH CONTOUR
//
// This subtracts the F1 frequency from the pitch to create a
// pitch contour. Without this, the output would be at a single
// pitch level (monotone).

	
	// don't adjust pitch if in sing mode
	if (!sam->common.singmode)
	{
        // iterate through the buffer
		for(i=0; i<RENDER_FRAMES; i++) {
            // subtract half the frequency of the formant 1.
            // this adds variety to the voice
    		sam->render.pitch[i] -= (sam->render.freq_amp[i].freq1 >> 1);
        }
	}
    OutputFrames(sam, mem48);
}

void OutputFrames(sam_memory *sam, unsigned char frame_count) {

	unsigned char phase1 = 0;
	unsigned char phase2 = 0;
	unsigned char phase3 = 0;
	unsigned char speedcounter = 72; //sam standard speed
    unsigned char mem66 = 0;

    // RESCALE AMPLITUDE
    // Rescale volume from decibels to a linear scale.
	for(int i=RENDER_FRAMES-1; i>=0; i--)
	{
		sam->render.freq_amp[i].amp1 = amplitudeRescale[sam->render.freq_amp[i].amp1];
		sam->render.freq_amp[i].amp2 = amplitudeRescale[sam->render.freq_amp[i].amp2];
		sam->render.freq_amp[i].amp3 = amplitudeRescale[sam->render.freq_amp[i].amp3];
	}

	unsigned char Y = 0;
	unsigned char A = sam->render.pitch[0];
	unsigned char glottal_pulse = A;
	unsigned char count = A - (A>>2);     // 3/4*A ???

    if (debug)
    {
        PrintOutput(sam->render.flags, sam->render.freq_amp, sam->render.pitch, frame_count);
    }

// PROCESS THE FRAMES
//
// In traditional vocal synthesis, the glottal pulse drives filters, which
// are attenuated to the frequencies of the formants.
//
// SAM generates these formants directly with sin and rectangular waves.
// To simulate them being driven by the glottal pulse, the waveforms are
// reset at the beginning of each glottal pulse.

	//finally the loop for sound output
	//pos48078:
	while(1)
	{
        // get the sampled information on the phoneme
		A = sam->render.flags[Y];
		unsigned char sample = A;
		
		// unvoiced sampled phoneme?
		A = A & 248;
		if(A != 0)
		{
            // render the sample for the phoneme
			Y = RenderSample(sam, &mem66, sample, Y);
			
			// skip ahead two in the frame buffer
			Y += 2;
			frame_count -= 2;
		} else
		{
            // simulate the glottal pulse and formants
			unsigned char accum = multtable[sinus[phase1] | sam->render.freq_amp[Y].amp1];

			int carry = 0;
			if ((accum+multtable[sinus[phase2] | sam->render.freq_amp[Y].amp2] ) > 255) carry = 1;
			accum += multtable[sinus[phase2] | sam->render.freq_amp[Y].amp2];
			A = accum + multtable[rectangle[phase3] | sam->render.freq_amp[Y].amp3] + (carry?1:0);
			A = ((A + 136) & 255) >> 4; //there must be also a carry
			//mem[54296] = A;
			
			// output the accumulated value
			Output(0, A);
			speedcounter--;
			if (speedcounter != 0) goto pos48155;
			Y++; //go to next amplitude
			
			// decrement the frame count
			frame_count--;
		}
		
		// if the frame count is zero, exit the loop
		if(frame_count == 0) 	return;
		speedcounter = sam->common.speed;
pos48155:
         
        // decrement the remaining length of the glottal pulse
		glottal_pulse--;
		
		// finished with a glottal pulse?
		if(glottal_pulse == 0)
		{
pos48159:
            // fetch the next glottal pulse length
			A = sam->render.pitch[Y];
			glottal_pulse = A;
			A = A - (A>>2);
			count = A;
			
			// reset the formant wave generators to keep them in 
			// sync with the glottal pulse
			phase1 = 0;
			phase2 = 0;
			phase3 = 0;
			continue;
		}
		
		// decrement the count
		count--;
		
		// is the count non-zero and the sampled flag is zero?
		if((count != 0) || (sample == 0)) {
            // reset the phase of the formants to match the pulse
			phase1 += sam->render.freq_amp[Y].freq1;
			phase2 += sam->render.freq_amp[Y].freq2;
			phase3 += sam->render.freq_amp[Y].freq3;
			continue;
		}
		
		// voiced sampled phonemes interleave the sample with the
		// glottal pulse. The sample flag is non-zero, so render
		// the sample for the phoneme.
		Y = RenderSample(sam, &mem66, sample, Y);
		goto pos48159;
	}
}


// Create a rising or falling inflection 30 frames prior to 
// index X. A rising inflection is used for questions, and 
// a falling inflection is used for statements.

void AddInflection(sam_memory* sam, unsigned char mem48, unsigned char phase1, unsigned char punctuation)
{
	unsigned char A = punctuation;
	int Atemp = A;
	
	// backup 30 frames
	A = A - 30; 
	// if index is before buffer, point to start of buffer
	if (Atemp <= 30) A=0;
	unsigned char X = A;

	// FIXME: Explain this fix better, it's not obvious
	// ML : A =, fixes a problem with invalid pitch with '.'
	while( (A=sam->render.pitch[X]) == 127) X++;


    while(1) {

        // add the inflection direction
        A += mem48;
        phase1 = A;

        // set the inflection
        sam->render.pitch[X] = A;
        do {

            // increment the position
            X++;

            // exit if the punctuation has been reached
            if (X == punctuation) return; //goto pos47615;
        } while (sam->render.pitch[X] == 255);
        A = phase1;
    }
}

static inline unsigned char trans(unsigned char mem39212, unsigned char mem39213) {
    return (mem39212*mem39213) >> 7;
}

/*
    SAM's voice can be altered by changing the frequencies of the
    mouth formant (F1) and the throat formant (F2). Only the voiced
    phonemes (5-29 and 48-53) are altered.
*/

static const unsigned char recalculate[] = {
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0
};

static const unsigned char mouth_formants[] = {
    0, 0, 0, 0, 0, 10, 14, 19, 24, 27,
    23, 21, 16, 20, 14, 18, 14, 18, 18, 16,
    13, 15, 11, 18, 14, 11, 9, 6, 6, 6,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 19, 27,
    21, 27, 18, 13, 0,
};


unsigned char get_freq1(unsigned char pos, unsigned char mouth) {
    if (recalculate[pos]) {
        return trans(mouth, mouth_formants[pos]);
    } else {
        return freq1data[pos];
    }
}

static const unsigned char throat_formants[] = {
    0, 0, 0, 0, 0, 84, 73, 67, 63, 40,
    44, 31, 37, 45, 73, 49, 36, 30, 51, 37,
    29, 69, 24, 50, 30, 24, 83, 46, 54, 86,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 72, 39,
    31, 43, 30, 34, 0,
};

unsigned char get_freq2(unsigned char pos, unsigned char throat) {
    if (recalculate[pos]) {
        return trans(throat, throat_formants[pos]);
    } else {
        return freq2data[pos];
    }
}
