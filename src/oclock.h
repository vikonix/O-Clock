/*
FreeBSD License

Copyright (c) 2019, Valeriy Kovalev: valeriy.kovalev.software@gmail.com
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

#ifndef __O_CLOCK_H__
#define __O_CLOCK_H__

#include <Arduino.h>
#include <RTClib.h>
#include <Bounce2.h>

#include "alarm.h"
#include "screen.h"

//////////////////////////////////////////////////////////////////////////////
//O'Clock version
#define VERSION "1.0"


//////////////////////////////////////////////////////////////////////////////
// current time variables
extern DateTime CurTime;
extern int CurHours;//-1 not inited
extern int CurMins;
extern int CurSecs;


extern Alarm alarm;
extern const int MelodiesCount;
extern int EepromSize;

//////////////////////////////////////////////////////////////////////////////
// keys variables
extern Bounce KeyMode;
extern Bounce KeyPlus;
extern Bounce KeyMinus;

//////////////////////////////////////////////////////////////////////////////
// auxilary functions
int  GetCurTime();
void SetTime(const DateTime& dt);
void AdjustBright();
void Beep(int n = 1);
void BeepError();
bool IsTPHpresent();
bool ReadTPH(float& Temperature, float& Pressure, float& Humidity);


void LoadConfig();
void SaveConfig();
void ConfigChanged();

#endif //__O_CLOCK_H__