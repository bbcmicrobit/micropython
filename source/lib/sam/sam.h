#ifndef SAM_H
#define SAM_H

void SetInput(char *_input);
void SetSpeed(unsigned char _speed);
void SetPitch(unsigned char _pitch);
void SetMouth(unsigned char _mouth);
void SetThroat(unsigned char _throat);
void EnableSingmode();
void DisableSingmode();

int SAMMain();

char* GetBuffer();
int GetBufferLength();

#endif

