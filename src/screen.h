/*
FreeBSD License

Copyright (c) 2019,2020 vikonix: valeriy.kovalev.software@gmail.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __SCREEN_H__
#define __SCREEN_H__

//////////////////////////////////////////////////////////////////////////////
//fonts and functions for working with matrix screen 8x32

//////////////////////////////////////////////////////////////////////////////
//imported parameters and functions for working with screen
extern const int LEDMATRIX_WIDTH;
extern byte screen_buffer[];

void ShowBuffer(byte* buffer = NULL);


//////////////////////////////////////////////////////////////////////////////
#define CHANGED_NOTHING 0
#define CHANGED_ALL     6
#define CHANGED_SEC     CHANGED_ALL
#define CHANGED_MIN1    1
#define CHANGED_MIN10   2
#define CHANGED_HOUR1   3
#define CHANGED_HOUR10  4
#define SCROLL_ALL      5

/////////////////////////////////////////////////////////////////////////////
void DisplayTime(int hours, int mins, int secs, bool alarm, byte scroll_mode, boolean fUp);
void PrintTinyString(const char* str, int x, int y, bool fixed = false);
void InverseBlock(int x, int y, int sizex, int sizey);
void PrintPictogram(int x, const byte* buff, int size);
void DisplayTemp(int temp, int rh);


#endif //__SCREEN_H__