#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "debug.h"
#include "sam.h"
#include "render.h"
#include "SamTabs.h"

extern int debug;

char *sam_error = "OK";

// contains the final soundbuffer
int bufferpos=0;


void SetInput(sam_memory* sam, const char *_input, unsigned int l)
{
    sam->prepare.input = _input;
    sam->prepare.input_length = l;
}

void Init(sam_memory* sam);
int Parser1(sam_memory* sam);
void Parser2(sam_memory* sam);
int SAMMain(sam_memory* sam);
void CopyStress(sam_memory* sam);
void SetPhonemeLength(sam_memory* sam);
void AdjustLengths(sam_memory* sam);
void Code41240(sam_memory* sam);
void Insert(sam_memory* sam, unsigned char position, unsigned char index, unsigned char length, unsigned char stress, unsigned char pitch);
void InsertBreath(sam_memory* sam);
void PrepareOutput(sam_memory* sam);

// 168=pitches 
// 169=frequency1
// 170=frequency2
// 171=frequency3
// 172=amplitude1
// 173=amplitude2
// 174=amplitude3


void Init(sam_memory* sam)
{
	int i;

	bufferpos = 0;

	/*
	freq2data = &mem[45136];
	freq1data = &mem[45056];
	freq3data = &mem[45216];
	*/
	//pitches = &mem[43008];
	/*
	frequency1 = &mem[43264];
	frequency2 = &mem[43520];
	frequency3 = &mem[43776];
	*/
	/*
	amplitude1 = &mem[44032];
	amplitude2 = &mem[44288];
	amplitude3 = &mem[44544];
	*/
	//phoneme = &mem[39904];
	/*
	ampl1data = &mem[45296];
	ampl2data = &mem[45376];
	ampl3data = &mem[45456];
	*/

	for(i=0; i<INPUT_PHONEMES; i++)
	{
		sam->prepare.phoneme_input[i].stress = 0;
		sam->prepare.phoneme_input[i].length = 0;
	}
	
	for(i=0; i<OUTPUT_PHONEMES; i++)
	{
		sam->common.phoneme_output[i].index = 0;
		sam->common.phoneme_output[i].stress = 0;
        sam->common.phoneme_output[i].length = 0;
        sam->common.phoneme_output[i].pitch = 0;
	}
	sam->prepare.phoneme_input[INPUT_PHONEMES-1].index = PHONEME_END; //to prevent buffer overflow // ML : changed from 32 to 255 to stop freezing with long inputs

}

void  ClearInput(sam_memory* sam) {
    sam->prepare.input = NULL;
    sam->prepare.input_length = 0;
}

int SAMMain(sam_memory* sam)
{
	Init(sam);

    int err = !Parser1(sam);
    ClearInput(sam);
    if (err) return 0;

	if (debug) {
		PrintPhonemes("Input phonemes", sam->prepare.phoneme_input);
    }
    Parser2(sam);
    CopyStress(sam);
    SetPhonemeLength(sam);
    AdjustLengths(sam);
	Code41240(sam);
    unsigned char X = 0;
	do
	{
		unsigned char A = sam->prepare.phoneme_input[X].index;
		if (A > 80)
		{
            if (A == PHONEME_END)
                break;
            sam_error = "INTERNAL ERROR: Illegal phoneme index";
            return 0;
		}
		X++;
	} while (X < INPUT_PHONEMES);

    if (debug)
        printf("Insert Breadth\r\n");
	InsertBreath(sam);

    if (debug) {
        PrintPhonemes("Processed phonemes", sam->prepare.phoneme_input);
    }

	PrepareOutput(sam);
    if (strcmp(sam_error, "OK"))
        return 0;
	return 1;
}


//void Code48547()
void PrepareOutput(sam_memory* sam)
{
	unsigned char A = 0;
	unsigned char X = 0;
	unsigned char Y = 0;

	//pos48551:
	while(1)
	{
		A = sam->prepare.phoneme_input[X].index;
		if (A == PHONEME_END)
		{
			sam->common.phoneme_output[Y].index = PHONEME_END;
			Render(sam);
			return;
		}
		if (A == PHONEME_END_BREATH)
		{
			X++;
			int temp = X;
			//mem[48546] = X;
			sam->common.phoneme_output[Y].index = PHONEME_END;
			Render(sam);
			//X = mem[48546];
			X=temp;
			Y = 0;
			continue;
		}

		if (A == 0)
		{
			X++;
			continue;
		}

		sam->common.phoneme_output[Y].index = A;
		sam->common.phoneme_output[Y].length = sam->prepare.phoneme_input[X].length;
        sam->common.phoneme_output[Y].stress = sam->prepare.phoneme_input[X].stress;
        sam->common.phoneme_output[Y].pitch = sam->prepare.phoneme_input[X].pitch;
		X++;
		Y++;
	}
}

void InsertBreath(sam_memory* sam)
{
	unsigned char index = 0;
	unsigned char frames = 0;
	while(1)
	{
		unsigned char phoneme_id = sam->prepare.phoneme_input[index].index;
		if (phoneme_id == PHONEME_END)
            return;
        frames += sam->prepare.phoneme_input[index].length;
		if (frames >= RENDER_FRAMES-16)
		{
            index++;
            Insert(sam, index, 0, 0, 0, 0);
            frames = 0;
            index++;
            Insert(sam, index, PHONEME_END_BREATH, 0, 0, 0);
            frames = 0;
        }
        // If phoneme is punctuation, then insert a breath after it.
        else if (phoneme_id < PHONEME_END_BREATH && (flags2[phoneme_id]&1))
        {
            index++;
            Insert(sam, index, PHONEME_END_BREATH, 0, 0, 0);
            frames = 0;
        }
        index++;
	}

}

// Iterates through the phoneme buffer, copying the stress value from
// the following phoneme under the following circumstance:
       
//     1. The current phoneme is voiced, excluding plosives and fricatives
//     2. The following phoneme is voiced, excluding plosives and fricatives, and
//     3. The following phoneme is stressed
//
//  In those cases, the stress value+1 from the following phoneme is copied.
//
// For example, the word LOITER is represented as LOY5TER, with as stress
// of 5 on the diphtong OY. This routine will copy the stress value of 6 (5+1)
// to the L that precedes it.


//void Code41883()
void CopyStress(sam_memory* sam)
{
    // loop thought all the phonemes to be output
	unsigned char pos=0; //mem66
	while(1)
	{
        // get the phomene
		unsigned char Y = sam->prepare.phoneme_input[pos].index;
		
	    // exit at end of buffer
		if (Y == PHONEME_END) return;
		
		// if CONSONANT_FLAG set, skip - only vowels get stress
		if ((flags[Y] & 64) == 0) {pos++; continue;}
		// get the next phoneme
		Y = sam->prepare.phoneme_input[pos+1].index;
		if (Y == PHONEME_END) //prevent buffer overflow
		{
			pos++; continue;
		} else
		// if the following phoneme is a vowel, skip
		if ((flags[Y] & 128) == 0)  {pos++; continue;}

        // get the stress value at the next position
		Y = sam->prepare.phoneme_input[pos+1].stress;
		
		// if next phoneme is not stressed, skip
		if (Y == 0)  {pos++; continue;}

		// if next phoneme is not a VOWEL OR ER, skip
		if ((Y & 128) != 0)  {pos++; continue;}

		// copy stress from prior phoneme to this one
		sam->prepare.phoneme_input[pos].stress = Y+1;
		
		// advance pointer
		pos++;
	}

}


//void Code41014()
void Insert(sam_memory* sam, unsigned char position/*var57*/, unsigned char index, unsigned char length, unsigned char stress, unsigned char pitch)
{
	int i;
	for(i=INPUT_PHONEMES-3; i >= position; i--) // ML : always keep last safe-guarding.
	{
        sam->prepare.phoneme_input[i+1] = sam->prepare.phoneme_input[i];
	}

	sam->prepare.phoneme_input[position].index = index;
	sam->prepare.phoneme_input[position].length = length;
    sam->prepare.phoneme_input[position].stress = stress;
    sam->prepare.phoneme_input[position].pitch = pitch;
	return;
}

// The input[] buffer contains a string of phonemes and stress markers along
// the lines of:
//
//     DHAX KAET IHZ AH5GLIY. <0x9B>
//
// The byte 0x9B marks the end of the buffer. Some phonemes are 2 bytes 
// long, such as "DH" and "AX". Others are 1 byte long, such as "T" and "Z". 
// There are also stress markers, such as "5" and ".".
//
// The first character of the phonemes are stored in the table signInputTable1[].
// The second character of the phonemes are stored in the table signInputTable2[].
// The stress characters are arranged in low to high stress order in stressInputTable[].
// 
// The following process is used to parse the input[] buffer:
// 
// Repeat until the <0x9B> character is reached:
//
//        First, a search is made for a 2 character match for phonemes that do not
//        end with the '*' (wildcard) character. On a match, the index of the phoneme 
//        is added to phonemeIndex[] and the buffer position is advanced 2 bytes.
//
//        If this fails, a search is made for a 1 character match against all
//        phoneme names ending with a '*' (wildcard). If this succeeds, the 
//        phoneme is added to phonemeIndex[] and the buffer position is advanced
//        1 byte.
// 
//        If this fails, search for a 1 character match in the stressInputTable[].
//        If this succeeds, the stress value is placed in the last stress[] table
//        at the same index of the last added phoneme, and the buffer position is
//        advanced by 1 byte.
//
//        If this fails, return a 0.
//
// On success:
//
//    1. phonemeIndex[] will contain the index of all the phonemes.
//    2. The last index in phonemeIndex[] will be PHONEME_END.
//    3. stress[] will contain the stress value for each phoneme

// input[] holds the string of phonemes, each two bytes wide
// signInputTable1[] holds the first character of each phoneme
// signInputTable2[] holds te second character of each phoneme
// phonemeIndex[] holds the indexes of the phonemes after parsing input[]
//
// The parser scans through the input[], finding the names of the phonemes
// by searching signInputTable1[] and signInputTable2[]. On a match, it
// copies the index of the phoneme into the phonemeIndexTable[].
//
// The character <0x9B> marks the end of text in input[]. When it is reached,
// the index PHONEME_END is placed at the end of the phonemeIndexTable[], and the
// function returns with a 1 indicating success.
int Parser1(sam_memory* sam)
{
	int i;
	unsigned char sign1;
	unsigned char sign2;
	unsigned char position = 0;
	unsigned char X = 0;
	unsigned char A = 0;
	unsigned char Y = 0;
	
	// CLEAR THE STRESS TABLE
	for(i=0; i<INPUT_PHONEMES; i++)
		sam->prepare.phoneme_input[i].stress = 0;

  // THIS CODE MATCHES THE PHONEME LETTERS TO THE TABLE
	// pos41078:
	while(1)
	{
        if (position >= INPUT_PHONEMES) {
            // Run out of space for phonemes -- This won't happen with a string from the reciter,
            // but can happen with manually created phonetic input.
            sam_error = "Phonemes too long";
            return 0;
        }
		// TEST FOR END OF STRING
		if (X >= sam->prepare.input_length)
		{
           // MARK ENDPOINT AND RETURN
			sam->prepare.phoneme_input[position].index = PHONEME_END;      //mark endpoint
			// REACHED END OF PHONEMES, SO EXIT
			return 1;       //all ok
		}

        // GET THE FIRST CHARACTER FROM THE PHONEME BUFFER
        sign1 = sam->prepare.input[X];

        if (sign1 > 96)
            sign1 -= 32;

		// GET THE NEXT CHARACTER FROM THE BUFFER
		X++;
        if (X == sam->prepare.input_length)
        {
            sign2 = 0;
        } else {
            sign2 = sam->prepare.input[X];
        }

        if (sign2 > 96)
            sign2 -= 32;

        if (sign1 == '#') {
            unsigned pitch = sign2 - '0';
            if (pitch > 9) {
                sam_error = "Illegal pitch";
                return 0;
            }
            X++;
            while (X < sam->prepare.input_length && ((unsigned)(sam->prepare.input[X] - '0')) < 10) {
                pitch *= 10;
                pitch += sam->prepare.input[X] - '0';
                X++;
            }
            if (pitch > 255) {
                sam_error = "Illegal pitch";
                return 0;
            }
            sam->common.pitch = pitch;
            continue;
        }

		// NOW sign1 = FIRST CHARACTER OF PHONEME, AND sign2 = SECOND CHARACTER OF PHONEME

       // TRY TO MATCH PHONEMES ON TWO TWO-CHARACTER NAME
       // IGNORE PHONEMES IN TABLE ENDING WITH WILDCARDS

       // SET INDEX TO 0
		Y = 0;
pos41095:
         
         // GET FIRST CHARACTER AT POSITION Y IN signInputTable
         // --> should change name to PhonemeNameTable1
		A = signInputTable1[Y];
		
		// FIRST CHARACTER MATCHES?
		if (A == sign1)
		{
           // GET THE CHARACTER FROM THE PhonemeSecondLetterTable
			A = signInputTable2[Y];
			// NOT A SPECIAL AND MATCHES SECOND CHARACTER?
			if ((A != '*') && (A == sign2))
			{
               // STORE THE INDEX OF THE PHONEME INTO THE phomeneIndexTable
                sam->prepare.phoneme_input[position].index = Y;
                sam->prepare.phoneme_input[position].pitch = sam->common.pitch;

				// ADVANCE THE POINTER TO THE phonemeIndexTable
				position++;
				// ADVANCE THE POINTER TO THE phonemeInputBuffer
				X++;

				// CONTINUE PARSING
				continue;
			}
		}
		
		// NO MATCH, TRY TO MATCH ON FIRST CHARACTER TO WILDCARD NAMES (ENDING WITH '*')
		
		// ADVANCE TO THE NEXT POSITION
		Y++;
		// IF NOT END OF TABLE, CONTINUE
		if (Y != 81) goto pos41095;

// REACHED END OF TABLE WITHOUT AN EXACT (2 CHARACTER) MATCH.
// THIS TIME, SEARCH FOR A 1 CHARACTER MATCH AGAINST THE WILDCARDS

// RESET THE INDEX TO POINT TO THE START OF THE PHONEME NAME TABLE
		Y = 0;
pos41134:
// DOES THE PHONEME IN THE TABLE END WITH '*'?
		if (signInputTable2[Y] == '*')
		{
// DOES THE FIRST CHARACTER MATCH THE FIRST LETTER OF THE PHONEME
			if (signInputTable1[Y] == sign1)
			{
                // SAVE THE POSITION AND MOVE AHEAD
				sam->prepare.phoneme_input[position].index = Y;
                sam->prepare.phoneme_input[position].pitch = sam->common.pitch;
				
				// ADVANCE THE POINTER
				position++;
				
				// CONTINUE THROUGH THE LOOP
				continue;
			}
		}
		Y++;
		if (Y != 81) goto pos41134; //81 is size of PHONEME NAME table

// FAILED TO MATCH WITH A WILDCARD. ASSUME THIS IS A STRESS
// CHARACTER. SEARCH THROUGH THE STRESS TABLE

        // SET INDEX TO POSITION 8 (END OF STRESS TABLE)
		Y = 8;
		
       // WALK BACK THROUGH TABLE LOOKING FOR A MATCH
		while( (sign1 != stressInputTable[Y]) && (Y>0))
		{
  // DECREMENT INDEX
			Y--;
		}

        // REACHED THE END OF THE SEARCH WITHOUT BREAKING OUT OF LOOP?
		if (Y == 0)
		{
			//mem[39444] = X;
			//41181: JSR 42043 //Error
           // FAILED TO MATCH ANYTHING, RETURN 0 ON FAILURE
            sam_error = "Phoneme not understood";
			return 0;
		}
// SET THE STRESS FOR THE PRIOR PHONEME
		sam->prepare.phoneme_input[position-1].stress = Y;
	} //while
}




//change phonemelength depedendent on stress
//void Code41203()
void SetPhonemeLength(sam_memory* sam)
{
	unsigned char A;
	int position = 0;
	while(sam->prepare.phoneme_input[position].index != PHONEME_END )
	{
        A = sam->prepare.phoneme_input[position].stress;
        //41218: BMI 41229
        if ((A == 0) || ((A&128) != 0))
        {
            sam->prepare.phoneme_input[position].length = phonemeLengthTable[sam->prepare.phoneme_input[position].index];
        } else
        {
            sam->prepare.phoneme_input[position].length = phonemeStressedLengthTable[sam->prepare.phoneme_input[position].index];
        }
        position++;
    }
}


void Code41240(sam_memory* sam)
{
	unsigned char pos=0;

	while(sam->prepare.phoneme_input[pos].index != PHONEME_END)
	{
		unsigned char index; //register AC
		unsigned char X = pos;
		index = sam->prepare.phoneme_input[pos].index;
		if ((flags[index]&2) == 0)
		{
			pos++;
			continue;
		} else
		if ((flags[index]&1) == 0)
		{
			Insert(sam, pos+1, index+1, phonemeLengthTable[index+1], sam->prepare.phoneme_input[pos].stress, sam->prepare.phoneme_input[pos].pitch);
			Insert(sam, pos+2, index+2, phonemeLengthTable[index+2], sam->prepare.phoneme_input[pos].stress, sam->prepare.phoneme_input[pos].pitch);
			pos += 3;
			continue;
		}
        unsigned char A;
		do
		{
			X++;
			A = sam->prepare.phoneme_input[X].index;
		} while(A==0);

		if (A != PHONEME_END)
		{
			if ((flags[A] & 8) != 0)  {pos++; continue;}
			if ((A == 36) || (A == 37)) {pos++; continue;} // '/H' '/X'
		}

		Insert(sam, pos+1, index+1, phonemeLengthTable[index+1], sam->prepare.phoneme_input[pos].stress, sam->prepare.phoneme_input[pos].pitch);
		Insert(sam, pos+2, index+2, phonemeLengthTable[index+2], sam->prepare.phoneme_input[pos].stress, sam->prepare.phoneme_input[pos].pitch);
		pos += 3;
	};

}

// Rewrites the phonemes using the following rules:
//
//       <DIPHTONG ENDING WITH WX> -> <DIPHTONG ENDING WITH WX> WX
//       <DIPHTONG NOT ENDING WITH WX> -> <DIPHTONG NOT ENDING WITH WX> YX
//       UL -> AX L
//       UM -> AX M
//       <STRESSED VOWEL> <SILENCE> <STRESSED VOWEL> -> <STRESSED VOWEL> <SILENCE> Q <VOWEL>
//       T R -> CH R
//       D R -> J R
//       <VOWEL> R -> <VOWEL> RX
//       <VOWEL> L -> <VOWEL> LX
//       G S -> G Z
//       K <VOWEL OR DIPHTONG NOT ENDING WITH IY> -> KX <VOWEL OR DIPHTONG NOT ENDING WITH IY>
//       G <VOWEL OR DIPHTONG NOT ENDING WITH IY> -> GX <VOWEL OR DIPHTONG NOT ENDING WITH IY>
//       S P -> S B
//       S T -> S D
//       S K -> S G
//       S KX -> S GX
//       <ALVEOLAR> UW -> <ALVEOLAR> UX
//       CH -> CH CH' (CH requires two phonemes to represent it)
//       J -> J J' (J requires two phonemes to represent it)
//       <UNSTRESSED VOWEL> T <PAUSE> -> <UNSTRESSED VOWEL> DX <PAUSE>
//       <UNSTRESSED VOWEL> D <PAUSE>  -> <UNSTRESSED VOWEL> DX <PAUSE>


//void Code41397()
void Parser2(sam_memory* sam)
{
	unsigned char pos = 0; //mem66;
	unsigned char mem58 = 0;


  // Loop through phonemes
	while(1)
	{
// SET X TO THE CURRENT POSITION
		unsigned char X = pos;
// GET THE PHONEME AT THE CURRENT POSITION
		unsigned char A = sam->prepare.phoneme_input[pos].index;

// DEBUG: Print phoneme and index
// DEBUG:		if (debug && A != PHONEME_END) printf("%d: %c%c\n", X, signInputTable1[A], signInputTable2[A]);

// Is phoneme pause or pitch shift?
		if (A == 0)
		{
// Move ahead to the 
			pos++;
			continue;
		}
		
// If end of phonemes flag reached, exit routine
		if (A == PHONEME_END) return;
		
// Copy the current phoneme index to Y
		unsigned char Y = A;

// RULE: 
//       <DIPHTONG ENDING WITH WX> -> <DIPHTONG ENDING WITH WX> WX
//       <DIPHTONG NOT ENDING WITH WX> -> <DIPHTONG NOT ENDING WITH WX> YX
// Example: OIL, COW


// Check for DIPHTONG
		if ((flags[A] & 16) == 0) goto pos41457;

// Not a diphthong. Get the stress
		mem58 = sam->prepare.phoneme_input[pos].stress;
		
// End in IY sound?
		A = flags[Y] & 32;
		
// If ends with IY, use YX, else use WX
		if (A == 0) A = 20; else A = 21;    // 'WX' = 20 'YX' = 21
		//pos41443:
// Insert at WX or YX following, copying the stress

		//DEBUG: if (A==20) printf("RULE: insert WX following diphtong NOT ending in IY sound\n");
		//DEBUG: if (A==21) printf("RULE: insert YX following diphtong ending in IY sound\n");
		Insert(sam, pos+1, A, 0, mem58, sam->prepare.phoneme_input[pos].pitch);
		X = pos;
// Jump to ???
		goto pos41749;



pos41457:
         
// RULE:
//       UL -> AX L
// Example: MEDDLE
       
// Get phoneme
		A = sam->prepare.phoneme_input[X].index;
// Skip this rule if phoneme is not UL
		if (A != 78) goto pos41487;  // 'UL'
		A = 24;         // 'L'                 //change 'UL' to 'AX L'
		
		//DEBUG: printf("RULE: UL -> AX L\n");

pos41466:
// Get current phoneme stress
		mem58 = sam->prepare.phoneme_input[X].stress;
		
// Change UL to AX
		sam->prepare.phoneme_input[X].index = 13;  // 'AX'
// Perform insert. Note code below may jump up here with different values
		Insert(sam, X+1, A, 0, mem58, sam->prepare.phoneme_input[X].pitch);
		pos++;
// Move to next phoneme
		continue;

pos41487:
         
// RULE:
//       UM -> AX M
// Example: ASTRONOMY
         
// Skip rule if phoneme != UM
		if (A != 79) goto pos41495;   // 'UM'
		// Jump up to branch - replaces current phoneme with AX and continues
		A = 27; // 'M'  //change 'UM' to  'AX M'
		//DEBUG: printf("RULE: UM -> AX M\n");
		goto pos41466;
pos41495:

// RULE:
//       UN -> AX N
// Example: FUNCTION

         
// Skip rule if phoneme != UN
		if (A != 80) goto pos41503; // 'UN'
		
		// Jump up to branch - replaces current phoneme with AX and continues
		A = 28;         // 'N' //change UN to 'AX N'
		//DEBUG: printf("RULE: UN -> AX N\n");
		goto pos41466;
pos41503:
         
// RULE:
//       <STRESSED VOWEL> <SILENCE> <STRESSED VOWEL> -> <STRESSED VOWEL> <SILENCE> Q <VOWEL>
// EXAMPLE: AWAY EIGHT
         
		Y = A;
// VOWEL set?
		A = flags[A] & 128;

// Skip if not a vowel
		if (A != 0)
		{
// Get the stress
			A = sam->prepare.phoneme_input[X].stress;

// If stressed...
			if (A != 0)
			{
// Get the following phoneme
				X++;
				A = sam->prepare.phoneme_input[X].index;
// If following phoneme is a pause

				if (A == 0)
				{
// Get the phoneme following pause
					X++;
					Y = sam->prepare.phoneme_input[X].index;

// Check for end of buffer flag
					if (Y == PHONEME_END) //buffer overflow
// ??? Not sure about these flags
     					A = 65&128;
					else
// And VOWEL flag to current phoneme's flags
     					A = flags[Y] & 128;

// If following phonemes is not a pause
					if (A != 0)
					{
// If the following phoneme is not stressed
						A = sam->prepare.phoneme_input[X].stress;
						if (A != 0)
						{
// Insert a glottal stop and move forward
							//DEBUG: printf("RULE: Insert glottal stop between two stressed vowels with space between them\n");
							// 31 = 'Q'
							Insert(sam, X, 31, 0, 0, 0);
							pos++;
							continue;
						}
					}
				}
			}
		}


// RULES FOR PHONEMES BEFORE R
//        T R -> CH R
// Example: TRACK


// Get current position and phoneme
		X = pos;
		A = sam->prepare.phoneme_input[pos].index;
		if (A != 23) goto pos41611;     // 'R'
		
// Look at prior phoneme
		X--;
		A = sam->prepare.phoneme_input[pos-1].index;
		//pos41567:
		if (A == 69)                    // 'T'
		{
// Change T to CH
			//DEBUG: printf("RULE: T R -> CH R\n");
			sam->prepare.phoneme_input[pos-1].index = 42;
			goto pos41779;
		}


// RULES FOR PHONEMES BEFORE R
//        D R -> J R
// Example: DRY

// Prior phonemes D?
		if (A == 57)                    // 'D'
		{
// Change D to J
			sam->prepare.phoneme_input[pos-1].index = 44;
			//DEBUG: printf("RULE: D R -> J R\n");
			goto pos41788;
		}

// RULES FOR PHONEMES BEFORE R
//        <VOWEL> R -> <VOWEL> RX
// Example: ART


// If vowel flag is set change R to RX
		A = flags[A] & 128;
		//DEBUG: printf("RULE: R -> RX\n");
		if (A != 0) sam->prepare.phoneme_input[pos].index = 18;  // 'RX'
		
// continue to next phoneme
		pos++;
		continue;

pos41611:

// RULE:
//       <VOWEL> L -> <VOWEL> LX
// Example: ALL

// Is phoneme L?
		if (A == 24)    // 'L'
		{
// If prior phoneme does not have VOWEL flag set, move to next phoneme
			if ((flags[sam->prepare.phoneme_input[pos-1].index] & 128) == 0) {pos++; continue;}
// Prior phoneme has VOWEL flag set, so change L to LX and move to next phoneme
			//DEBUG: printf("RULE: <VOWEL> L -> <VOWEL> LX\n");
			sam->prepare.phoneme_input[X].index = 19;     // 'LX'
			pos++;
			continue;
		}
		
// RULE:
//       G S -> G Z
//
// Can't get to fire -
//       1. The G -> GX rule intervenes
//       2. Reciter already replaces GS -> GZ

// Is current phoneme S?
		if (A == 32)    // 'S'
		{
// If prior phoneme is not G, move to next phoneme
			if (sam->prepare.phoneme_input[pos-1].index != 60) {pos++; continue;}
// Replace S with Z and move on
			//DEBUG: printf("RULE: G S -> G Z\n");
			sam->prepare.phoneme_input[pos].index = 38;    // 'Z'
			pos++;
			continue;
		}

// RULE:
//             K <VOWEL OR DIPHTONG NOT ENDING WITH IY> -> KX <VOWEL OR DIPHTONG NOT ENDING WITH IY>
// Example: COW

// Is current phoneme K?
		if (A == 72)    // 'K'
		{
// Get next phoneme
			Y = sam->prepare.phoneme_input[pos+1].index;
// If at end, replace current phoneme with KX
			if (Y == PHONEME_END) sam->prepare.phoneme_input[pos].index = 75; // ML : prevents an index out of bounds problem
			else
			{
// VOWELS AND DIPHTONGS ENDING WITH IY SOUND flag set?
				A = flags[Y] & 32;
				//DEBUG: if (A==0) printf("RULE: K <VOWEL OR DIPHTONG NOT ENDING WITH IY> -> KX <VOWEL OR DIPHTONG NOT ENDING WITH IY>\n");
// Replace with KX
				if (A == 0) sam->prepare.phoneme_input[pos].index = 75;  // 'KX'
			}
		}
		else

// RULE:
//             G <VOWEL OR DIPHTONG NOT ENDING WITH IY> -> GX <VOWEL OR DIPHTONG NOT ENDING WITH IY>
// Example: GO


// Is character a G?
		if (A == 60)   // 'G'
		{
// Get the following character
			unsigned char index = sam->prepare.phoneme_input[pos+1].index;
			
// At end of buffer?
			if (index == PHONEME_END) //prevent buffer overflow
			{
				pos++; continue;
			}
			else
// If diphtong ending with YX, move continue processing next phoneme
			if ((flags[index] & 32) != 0) {pos++; continue;}
// replace G with GX and continue processing next phoneme
			//DEBUG: printf("RULE: G <VOWEL OR DIPHTONG NOT ENDING WITH IY> -> GX <VOWEL OR DIPHTONG NOT ENDING WITH IY>\n");
			sam->prepare.phoneme_input[pos].index = 63; // 'GX'
			pos++;
			continue;
		}
		
// RULE:
//      S P -> S B
//      S T -> S D
//      S K -> S G
//      S KX -> S GX
// Examples: SPY, STY, SKY, SCOWL
		
		Y = sam->prepare.phoneme_input[pos].index;
		//pos41719:
// Replace with softer version?
		A = flags[Y] & 1;
		if (A == 0) goto pos41749;
		A = sam->prepare.phoneme_input[pos-1].index;
		if (A != 32)    // 'S'
		{
			A = Y;
			goto pos41812;
		}
		// Replace with softer version
		//DEBUG: printf("RULE: S* %c%c -> S* %c%c\n", signInputTable1[Y], signInputTable2[Y],signInputTable1[Y-12], signInputTable2[Y-12]);
		sam->prepare.phoneme_input[pos].index = Y-12;
		pos++;
		continue;


pos41749:
         
// RULE:
//      <ALVEOLAR> UW -> <ALVEOLAR> UX
//
// Example: NEW, DEW, SUE, ZOO, THOO, TOO

//       UW -> UX

		A = sam->prepare.phoneme_input[X].index;
		if (A == 53)    // 'UW'
		{
// ALVEOLAR flag set?
			Y = sam->prepare.phoneme_input[X-1].index;
			A = flags2[Y] & 4;
// If not set, continue processing next phoneme
			if (A == 0) {pos++; continue;}
			//DEBUG: printf("RULE: <ALVEOLAR> UW -> <ALVEOLAR> UX\n");
			sam->prepare.phoneme_input[X].index = 16;
			pos++;
			continue;
		}
pos41779:

// RULE:
//       CH -> CH CH' (CH requires two phonemes to represent it)
// Example: CHEW

		if (A == 42)    // 'CH'
		{
			//        pos41783:
			//DEBUG: printf("CH -> CH CH+1\n");
			Insert(sam, X+1, A+1, 0, sam->prepare.phoneme_input[X].stress, sam->prepare.phoneme_input[X].pitch);
			pos++;
			continue;
		}

pos41788:
         
// RULE:
//       J -> J J' (J requires two phonemes to represent it)
// Example: JAY
         

		if (A == 44) // 'J'
		{
			//DEBUG: printf("J -> J J+1\n");
			Insert(sam, X+1, A+1, 0, sam->prepare.phoneme_input[X].stress, sam->prepare.phoneme_input[X].pitch);
			pos++;
			continue;
		}
		
// Jump here to continue 
pos41812:

// RULE: Soften T following vowel
// NOTE: This rule fails for cases such as "ODD"
//       <UNSTRESSED VOWEL> T <PAUSE> -> <UNSTRESSED VOWEL> DX <PAUSE>
//       <UNSTRESSED VOWEL> D <PAUSE>  -> <UNSTRESSED VOWEL> DX <PAUSE>
// Example: PARTY, TARDY


// Past this point, only process if phoneme is T or D
         
		if (A != 69)    // 'T'
		if (A != 57) {pos++; continue;}       // 'D'
		//pos41825:


// If prior phoneme is not a vowel, continue processing phonemes
		if ((flags[sam->prepare.phoneme_input[X-1].index] & 128) == 0) {pos++; continue;}
		
// Get next phoneme
		X++;
		A = sam->prepare.phoneme_input[X].index;
		//pos41841
// Is the next phoneme a pause?
		if (A != 0)
		{
// If next phoneme is not a pause, continue processing phonemes
			if ((flags[A] & 128) == 0) {pos++; continue;}
// If next phoneme is stressed, continue processing phonemes
// FIXME: How does a pause get stressed?
			if (sam->prepare.phoneme_input[X].stress != 0) {pos++; continue;}
//pos41856:
// Set phonemes to DX
		//DEBUG: printf("RULE: Soften T or D following vowel or ER and preceding a pause -> DX\n");
		sam->prepare.phoneme_input[pos].index = 30;       // 'DX'
		} else
		{
			A = sam->prepare.phoneme_input[X+1].index;
			if (A == PHONEME_END) //prevent buffer overflow
				A = 65 & 128;
			else
// Is next phoneme a vowel or ER?
				A = flags[A] & 128;
			//DEBUG: if (A != 0) printf("RULE: Soften T or D following vowel or ER and preceding a pause -> DX\n");
			if (A != 0) sam->prepare.phoneme_input[pos].index = 30;  // 'DX'
		}

		pos++;

	} // while
}


// Applies various rules that adjust the lengths of phonemes
//
//         Lengthen <FRICATIVE> or <VOICED> between <VOWEL> and <PUNCTUATION> by 1.5
//         <VOWEL> <RX | LX> <CONSONANT> - decrease <VOWEL> length by 1
//         <VOWEL> <UNVOICED PLOSIVE> - decrease vowel by 1/8th
//         <VOWEL> <UNVOICED CONSONANT> - increase vowel by 1/2 + 1
//         <NASAL> <STOP CONSONANT> - set nasal = 5, consonant = 6
//         <VOICED STOP CONSONANT> {optional silence} <STOP CONSONANT> - shorten both to 1/2 + 1
//         <LIQUID CONSONANT> <DIPHTONG> - decrease by 2


//void Code48619()
void AdjustLengths(sam_memory* sam)
{

    // LENGTHEN VOWELS PRECEDING PUNCTUATION
    //
    // Search for punctuation. If found, back up to the first vowel, then
    // process all phonemes between there and up to (but not including) the punctuation.
    // If any phoneme is found that is a either a fricative or voiced, the duration is
    // increased by (length * 1.5) + 1

    // loop index
	unsigned char X = 0;
	unsigned char index;
    unsigned char mem56;

    // iterate through the phoneme list
	unsigned char loopIndex=0;
	while(1)
	{
        // get a phoneme
		index = sam->prepare.phoneme_input[X].index;
		
		// exit loop if end on buffer token
		if (index == PHONEME_END) break;

		// not punctuation?
		if((flags2[index] & 1) == 0)
		{
            // skip
			X++;
			continue;
		}
		
		// hold index
		loopIndex = X;
		
		// Loop backwards from this point
pos48644:
         
        // back up one phoneme
		X--;
		
		// stop once the beginning is reached
		if(X == 0) break;
		
		// get the preceding phoneme
		index = sam->prepare.phoneme_input[X].index;

		if (index != PHONEME_END) //inserted to prevent access overrun
		if((flags[index] & 128) == 0) goto pos48644; // if not a vowel, continue looping

		//pos48657:
		do
		{
            // test for vowel
			index = sam->prepare.phoneme_input[X].index;

			if (index != PHONEME_END)//inserted to prevent access overrun
			// test for fricative/unvoiced or not voiced
			if(((flags2[index] & 32) == 0) || ((flags[index] & 4) != 0))     //nochmal überprüfen
			{
				//A = flags[Y] & 4;
				//if(A == 0) goto pos48688;
								
                // get the phoneme length
				unsigned char A = sam->prepare.phoneme_input[X].length;

				// change phoneme length to (length * 1.5) + 1
				A = (A >> 1) + A + 1;
//DEBUG: printf("RULE: Lengthen <FRICATIVE> or <VOICED> between <VOWEL> and <PUNCTUATION> by 1.5\n");
//DEBUG: printf("PRE\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X, signInputTable1[sam->prepare.phoneme_input[X].index], signInputTable2[sam->prepare.phoneme_input[X].index], sam->prepare.phoneme_input[X].length);

				sam->prepare.phoneme_input[X].length = A;
				
//DEBUG: printf("POST\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X, signInputTable1[sam->prepare.phoneme_input[X].index], signInputTable2[sam->prepare.phoneme_input[X].index], sam->prepare.phoneme_input[X].length);

			}
            // keep moving forward
			X++;
		} while (X != loopIndex);
		//	if (X != loopIndex) goto pos48657;
		X++;
	}  // while

    // Similar to the above routine, but shorten vowels under some circumstances

    // Loop throught all phonemes
	loopIndex = 0;
	//pos48697

	while(1)
	{
        // get a phoneme
		X = loopIndex;
		index = sam->prepare.phoneme_input[X].index;
		
		// exit routine at end token
		if (index == PHONEME_END) return;

		// vowel?
		unsigned char A = flags[index] & 128;
		if (A != 0)
		{
            // get next phoneme
			X++;
			index = sam->prepare.phoneme_input[X].index;
			
			// get flags
			if (index == PHONEME_END)
			mem56 = 65; // use if end marker
			else
			mem56 = flags[index];

            // not a consonant
			if ((flags[index] & 64) == 0)
			{
                // RX or LX?
				if ((index == 18) || (index == 19))  // 'RX' & 'LX'
				{
                    // get the next phoneme
					X++;
					index = sam->prepare.phoneme_input[X].index;
					
					// next phoneme a consonant?
					if ((flags[index] & 64) != 0) {
                        // RULE: <VOWEL> RX | LX <CONSONANT>
                        
                        
//DEBUG: printf("RULE: <VOWEL> <RX | LX> <CONSONANT> - decrease length by 1\n");
//DEBUG: printf("PRE\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", loopIndex, signInputTable1[sam->prepare.phoneme_input[loopIndex].index], signInputTable2[sam->prepare.phoneme_input[loopIndex].index], sam->prepare.phoneme_input[loopIndex].length);
                        
                        // decrease length of vowel by 1 frame
    					sam->prepare.phoneme_input[loopIndex].length--;

//DEBUG: printf("POST\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", loopIndex, signInputTable1[sam->prepare.phoneme_input[loopIndex].index], signInputTable2[sam->prepare.phoneme_input[loopIndex].index], sam->prepare.phoneme_input[loopIndex].length);

                    }
                    // move ahead
					loopIndex++;
					continue;
				}
				// move ahead
				loopIndex++;
				continue;
			}
			
			
			// Got here if not <VOWEL>

            // not voiced
			if ((mem56 & 4) == 0)
			{
                       
                 // Unvoiced 
                 // *, .*, ?*, ,*, -*, DX, S*, SH, F*, TH, /H, /X, CH, P*, T*, K*, KX
                 
                // not an unvoiced plosive?
				if((mem56 & 1) == 0) {
                    // move ahead
                    loopIndex++; 
                    continue;
                }

                // P*, T*, K*, KX

                
                // RULE: <VOWEL> <UNVOICED PLOSIVE>
                // <VOWEL> <P*, T*, K*, KX>
                
                // move back
				X--;
				
//DEBUG: printf("RULE: <VOWEL> <UNVOICED PLOSIVE> - decrease vowel by 1/8th\n");
//DEBUG: printf("PRE\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X, signInputTable1[sam->prepare.phoneme_input[X].index], signInputTable2[sam->prepare.phoneme_input[X].index],  sam->prepare.phoneme_input[X].length);

                // decrease length by 1/8th
				mem56 = sam->prepare.phoneme_input[X].length >> 3;
				sam->prepare.phoneme_input[X].length -= mem56;

//DEBUG: printf("POST\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X, signInputTable1[sam->prepare.phoneme_input[X].index], signInputTable2[sam->prepare.phoneme_input[X].index], sam->prepare.phoneme_input[X].length);

                // move ahead
				loopIndex++;
				continue;
			}

            // RULE: <VOWEL> <VOICED CONSONANT>
            // <VOWEL> <WH, R*, L*, W*, Y*, M*, N*, NX, DX, Q*, Z*, ZH, V*, DH, J*, B*, D*, G*, GX>

//DEBUG: printf("RULE: <VOWEL> <VOICED CONSONANT> - increase vowel by 1/2 + 1\n");
//DEBUG: printf("PRE\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X-1, signInputTable1[sam->prepare.phoneme_input[X-1].index], signInputTable2[sam->prepare.phoneme_input[X-1].index],  sam->prepare.phoneme_input[X-1].length);

            // decrease length
			A = sam->prepare.phoneme_input[X-1].length;
			sam->prepare.phoneme_input[X-1].length = (A >> 2) + A + 1;     // 5/4*A + 1

//DEBUG: printf("POST\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X-1, signInputTable1[sam->prepare.phoneme_input[X-1].index], signInputTable2[sam->prepare.phoneme_input[X-1].index], sam->prepare.phoneme_input[X-1].length);

            // move ahead
			loopIndex++;
			continue;
			
		}


        // WH, R*, L*, W*, Y*, M*, N*, NX, Q*, Z*, ZH, V*, DH, J*, B*, D*, G*, GX

//pos48821:
           
        // RULE: <NASAL> <STOP CONSONANT>
        //       Set punctuation length to 6
        //       Set stop consonant length to 5
           
        // nasal?
        if((flags2[index] & 8) != 0)
        {
                          
            // M*, N*, NX, 

            // get the next phoneme
            X++;
            index = sam->prepare.phoneme_input[X].index;

            // end of buffer?
            if (index == PHONEME_END)
               A = 65&2;  //prevent buffer overflow
            else
                A = flags[index] & 2; // check for stop consonant


            // is next phoneme a stop consonant?
            if (A != 0)
            
               // B*, D*, G*, GX, P*, T*, K*, KX

            {
//DEBUG: printf("RULE: <NASAL> <STOP CONSONANT> - set nasal = 5, consonant = 6\n");
//DEBUG: printf("POST\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X, signInputTable1[sam->prepare.phoneme_input[X].index], signInputTable2[sam->prepare.phoneme_input[X].index], sam->prepare.phoneme_input[X].length);
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X-1, signInputTable1[sam->prepare.phoneme_input[X-1].index], signInputTable2[sam->prepare.phoneme_input[X-1].index], sam->prepare.phoneme_input[X-1].length);

                // set stop consonant length to 6
                sam->prepare.phoneme_input[X].length = 6;
                
                // set nasal length to 5
                sam->prepare.phoneme_input[X-1].length = 5;
                
//DEBUG: printf("POST\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X, signInputTable1[sam->prepare.phoneme_input[X].index], signInputTable2[sam->prepare.phoneme_input[X].index], sam->prepare.phoneme_input[X].length);
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X-1, signInputTable1[sam->prepare.phoneme_input[X-1].index], signInputTable2[sam->prepare.phoneme_input[X-1].index], sam->prepare.phoneme_input[X-1].length);

            }
            // move to next phoneme
            loopIndex++;
            continue;
        }


        // WH, R*, L*, W*, Y*, Q*, Z*, ZH, V*, DH, J*, B*, D*, G*, GX

        // RULE: <VOICED STOP CONSONANT> {optional silence} <STOP CONSONANT>
        //       Shorten both to (length/2 + 1)

        // (voiced) stop consonant?
        if((flags[index] & 2) != 0)
        {                         
            // B*, D*, G*, GX
                         
            // move past silence
            do
            {
                // move ahead
                X++;
                index = sam->prepare.phoneme_input[X].index;
            } while(index == 0);


            // check for end of buffer
            if (index == PHONEME_END) //buffer overflow
            {
                // ignore, overflow code
                if ((65 & 2) == 0) {loopIndex++; continue;}
            } else if ((flags[index] & 2) == 0) {
                // if another stop consonant, move ahead
                loopIndex++;
                continue;
            }

            // RULE: <UNVOICED STOP CONSONANT> {optional silence} <STOP CONSONANT>
//DEBUG: printf("RULE: <UNVOICED STOP CONSONANT> {optional silence} <STOP CONSONANT> - shorten both to 1/2 + 1\n");
//DEBUG: printf("PRE\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X, signInputTable1[sam->prepare.phoneme_input[X].index], signInputTable2[sam->prepare.phoneme_input[X].index], sam->prepare.phoneme_input[X].length);
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X-1, signInputTable1[sam->prepare.phoneme_input[X-1].index], signInputTable2[sam->prepare.phoneme_input[X-1].index], sam->prepare.phoneme_input[X-1].length);
// X gets overwritten, so hold prior X value for debug statement
// int debugX = X;
            // shorten the prior phoneme length to (length/2 + 1)
            sam->prepare.phoneme_input[X].length = (sam->prepare.phoneme_input[X].length >> 1) + 1;
            X = loopIndex;

            // also shorten this phoneme length to (length/2 +1)
            sam->prepare.phoneme_input[loopIndex].length = (sam->prepare.phoneme_input[loopIndex].length >> 1) + 1;

//DEBUG: printf("POST\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", debugX, signInputTable1[sam->prepare.phoneme_input[debugX].index], signInputTable2[sam->prepare.phoneme_input[debugX].index], sam->prepare.phoneme_input[debugX].length);
//DEBUG: printf("phoneme %d (%c%c) length %d\n", debugX-1, signInputTable1[sam->prepare.phoneme_input[debugX-1].index], signInputTable2[sam->prepare.phoneme_input[debugX-1].index], sam->prepare.phoneme_input[debugX-1].length);


            // move ahead
            loopIndex++;
            continue;
        }


        // WH, R*, L*, W*, Y*, Q*, Z*, ZH, V*, DH, J*, **, 

        // RULE: <VOICED NON-VOWEL> <DIPHTONG>
        //       Decrease <DIPHTONG> by 2

        // liquic consonant?
        if ((flags2[index] & 16) != 0)
        {
            // R*, L*, W*, Y*
                           
            // get the prior phoneme
            index = sam->prepare.phoneme_input[X-1].index;

            // prior phoneme a stop consonant>
            if((flags[index] & 2) != 0)
                             // Rule: <LIQUID CONSONANT> <DIPHTONG>

//DEBUG: printf("RULE: <LIQUID CONSONANT> <DIPHTONG> - decrease by 2\n");
//DEBUG: printf("PRE\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X, signInputTable1[sam->prepare.phoneme_input[X].index], signInputTable2[sam->prepare.phoneme_input[X].index], sam->prepare.phoneme_input[X].length);
             
             // decrease the phoneme length by 2 frames (20 ms)
             sam->prepare.phoneme_input[X].length -= 2;

//DEBUG: printf("POST\n");
//DEBUG: printf("phoneme %d (%c%c) length %d\n", X, signInputTable1[sam->prepare.phoneme_input[X].index], signInputTable2[sam->prepare.phoneme_input[X].index], sam->prepare.phoneme_input[X].length);
         }

         // move to next phoneme
         loopIndex++;
         continue;
    }
//            goto pos48701;
}

