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

#include <Arduino.h>
#include <EEPROM.h>

#include "oclock.h"

//////////////////////////////////////////////////////////////////////////////
// O'Clock main loop() with Clock Menu code

//delay in automatic key repeat mode
//0-without delay
#define KEY_DELAY 1

//show pressure in mm Hg
//#define PRESSURE_IN_MM

//automaticaly show pressure, temperature and date  
#define SWITCH_MODE_PERIOD     2 //n minutes - showing period of Pressure/Temp/Date
#define AUTO_SHOW_PRESSURE     2 //0-Off else duration in seconds
#define AUTO_SHOW_TEMP         2 //0-Off else duration in seconds
#define AUTO_SHOW_DATE         2 //0-Off else duration in seconds

//at this seconds parameter was displayed
#define SHOW_PRESSURE_ON_SECS 15 //seconds to show pressure
#define SHOW_TEMP_ON_SECS     30 //seconds to show temperature
#define SHOW_DATE_ON_SECS     45 //seconds to show date

//////////////////////////////////////////////////////////////////////////////
#define BLINK_LOG   8 /* 8 -> 500ms */
#define IS_BLINK() (((millis() >> (BLINK_LOG + 1)) & 1) == 0) 


//////////////////////////////////////////////////////////////////////////////
//pictogramms
//first byte - size
const byte AlarmPic[] = 
  {6, 0x20, 0x3e, 0xbf, 0xbf, 0x3e, 0x20};
  //{6, 0x20, 0x3e, 0xa3, 0xa3, 0x3e, 0x20};
const byte GradC[] = 
  {5, 0x02, 0x00, 0x1c, 0x22, 0x22};

//////////////////////////////////////////////////////////////////////////////
// Clock Configuration
bool fConfigChanged = false;
struct ClockConfig {
  //alarm
  uint8_t days;
  uint8_t hour;
  uint8_t mins;
  uint8_t melody;
} config;

void LoadConfig()
{
  Serial.println(F("LoadConfig"));
  EEPROM.get(0, config);
  if(config.days != 0xff) //not inited EEPROM
    alarm.SetAlarm(config.days, config.hour, config.mins, config.melody);
}

void SaveConfig()
{
  if(fConfigChanged)
  {
    Serial.println(F("SaveConfig"));
    fConfigChanged = false;
    alarm.GetAlarm(config.days, config.hour, config.mins, config.melody);
    EEPROM.put(0, config);
  }
}

void ConfigChanged()
{
  fConfigChanged = true;
}

//////////////////////////////////////////////////////////////////////////////
// Clock Menu codes
enum {
  //Show menu
  MODE_SHOW_BEGIN,
    MODE_SHOW_CLOCK,
    MODE_SHOW_TEMP,
    MODE_SHOW_PRESSURE,
    MODE_SHOW_DATE,
    MODE_SHOW_ALARM,
  MODE_SHOW_END,

  //Setup menu
  MODE_SET_BEGIN,
    MODE_SET_TIME,
    MODE_SET_ALARM,
    MODE_SHOW_VERSION,
  MODE_SET_END,

  //change time/date menu
  MODE_CH_BEGIN,
    MODE_CH_HOUR,
    MODE_CH_MIN,
    MODE_CH_SEC,
    MODE_CH_DAY,
    MODE_CH_MONTH,
    MODE_CH_YEAR,
  MODE_CH_END,

  //change alarm menu
  MODE_CH_ALARM_BEGIN,
    MODE_CH_ALARM_HOUR,
    MODE_CH_ALARM_MIN,
    MODE_CH_ALARM_DAY1,
    MODE_CH_ALARM_DAY2,
    MODE_CH_ALARM_DAY3,
    MODE_CH_ALARM_DAY4,
    MODE_CH_ALARM_DAY5,
    MODE_CH_ALARM_DAY6,
    MODE_CH_ALARM_DAY7,
    MODE_CH_ALARM_MELODY,
  MODE_CH_ALARM_END,
  
  MODE_UNUSED
  //move here unused menu Id
};


//////////////////////////////////////////////////////////////////////////////
const char DaysSymbol[] = "MTWRFSU";
const char Months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//////////////////////////////////////////////////////////////////////////////
//change time mode
void ChangeTime(int Mode, int Increment)
{
  int h = CurTime.hour();
  int m = CurTime.minute();
  int s = CurTime.second();
  int D = CurTime.day();
  int M = CurTime.month();
  int Y = CurTime.year();

  switch(Mode)
  {
    case MODE_CH_HOUR:
      h += Increment;
      if(h < 0)
        h = 23;
      else if(h > 23)
        h = 0;
      break;
    case MODE_CH_MIN:
      m += Increment;
      if(m < 0)
        m = 59;
      else if(m > 59)
        m = 0;
      break;
    case MODE_CH_SEC:
      s += Increment;
      if(s < 0)
        s = 59;
      else if(s > 59)
        s = 0;
      break;
    case MODE_CH_DAY:
      D += Increment;
      if(D < 1)
        D = 31;
      else if(D > 31)
        D = 1;
      break;
    case MODE_CH_MONTH:
      M += Increment;
      if(M < 1)
        M = 12;
      else if(M > 12)
        M = 1;
      break;
    case MODE_CH_YEAR:
      Y += Increment;
      if(Y < 2010)
        Y = 2099;
      else if(Y > 2099)
        Y = 2010;
      break;
  }

  char buff[16];
  if(Mode == MODE_CH_HOUR || Mode == MODE_CH_MIN || Mode == MODE_CH_SEC)
  {
    sprintf(buff, "%02d:%02d.%02d", h, m, s);
    PrintTinyString(buff, 0, 1, true);
    int x = 0;
    if(Mode == MODE_CH_MIN)
      x = 12;
    else if(Mode == MODE_CH_SEC)
      x = 24;
    if(IS_BLINK())
    { 
      InverseBlock(x, 7, 7, 1);
    }
  }
  else
  {
    sprintf(buff, "%02d  %s.%d", D, Months[M - 1], Y - 2000);
    PrintTinyString(buff, 0, 1);
    int x  = 0;
    int sx = 7;
    if(Mode == MODE_CH_MONTH)
    {
      x  = 10;
      sx = 10;
    }
    else if(Mode == MODE_CH_YEAR)
    {
      x  = 24;
      sx = 6;
    }
    if(IS_BLINK())
    {  
      InverseBlock(x, 7, sx, 1);
    }
  }

  if(Increment)
  {
    SetTime(DateTime(Y, M, D, h, m, s));
    GetCurTime();
  }
}


//////////////////////////////////////////////////////////////////////////////
//change alarm mode
void ChangeAlarm(int Mode, int Increment)
{
  uint8_t d, uh, um, umelody;
  alarm.GetAlarm(d, uh, um, umelody);

  //we need signed numbers
  int h = uh;
  int m = um;
  int melody = umelody;

  switch(Mode)
  {
    case MODE_CH_ALARM_HOUR:
      h += Increment;
      if(h < 0)
        h = 23;
      else if(h > 23)
        h = 0;
      break;
    case MODE_CH_ALARM_MIN:
      m += Increment;
      if(m < 0)
        m = 59;
      else if(m > 59)
        m = 0;
      break;
    case MODE_CH_ALARM_DAY1:  
    case MODE_CH_ALARM_DAY2:  
    case MODE_CH_ALARM_DAY3:  
    case MODE_CH_ALARM_DAY4:  
    case MODE_CH_ALARM_DAY5:  
    case MODE_CH_ALARM_DAY6:  
    case MODE_CH_ALARM_DAY7: 
    { 
      uint8_t day  = Mode - MODE_CH_ALARM_DAY1;
      uint8_t mask = 1 << day;;
      if(Increment != 0)
        d ^= mask;
      break;
    }
    case MODE_CH_ALARM_MELODY:
      melody += Increment;
      if(melody < 0)
        melody = MelodiesCount - 1;
      else if(melody >= MelodiesCount)  
        melody = 0;
      break;  
  }

  char buff[16];
  if(Mode == MODE_CH_ALARM_HOUR || Mode == MODE_CH_ALARM_MIN)
  {
    sprintf(buff, "%02d:%02d", h, m);
    PrintTinyString(buff, 8, 1, true);
    PrintPictogram(0, &AlarmPic[1], AlarmPic[0]);

    int x = 8;
    if(Mode == MODE_CH_ALARM_MIN)
      x = 20;
    if(IS_BLINK())
    {
      InverseBlock(x, 7, 7, 1);
    }
  }
  else if(Mode == MODE_CH_ALARM_MELODY)
  {
    sprintf(buff, "Melody %d", melody + 1);
    PrintTinyString(buff, 0, 1, true);
    if(IS_BLINK())
    {
      InverseBlock(28, 7, 3, 1);
    }
  }
  else
  {
    PrintTinyString(DaysSymbol, 0, 0, true);
    uint8_t mask = 1;
    for(int i = 0; i < 7; ++i)
    {
      if((d & mask) != 0)
        InverseBlock(i * 4, 6, 3, 1);
      mask <<= 1;  
    }
    if(IS_BLINK())
      InverseBlock((Mode - MODE_CH_ALARM_DAY1) * 4, 7, 3, 1);
  }
  
  if(Increment != 0)
  {
    ConfigChanged();
    alarm.SetAlarm(d, h, m, melody);
    if(Mode == MODE_CH_ALARM_MELODY)
      alarm.Play();
  }
}

//////////////////////////////////////////////////////////////////////////////
#define KEY_TIMEOUT_PERIOD     5  //sec
#define LONG_PRESSED_PERIOD 2000  //msec
#define LOOP_SLEEP_TIME       50  //msec

//////////////////////////////////////////////////////////////////////////////
void loop()
{
  //current mode
  static int  Mode = MODE_SHOW_CLOCK;
  static uint32_t ModeTimeout = 0;
  
  bool fBlink = false;
  static unsigned long blinkt = 0;
  
  unsigned long ms = millis();
  if(ms - blinkt >= (1ul << BLINK_LOG))
  {
    blinkt = ms;
    fBlink = true;
  }

  char buff[16];

  //Update Params
  int TimeChanged = GetCurTime();
  if(TimeChanged)
    AdjustBright();

  bool fAlarm = false;
  if(Mode == MODE_SHOW_CLOCK || Mode == MODE_CH_ALARM_MELODY)
    fAlarm = alarm.CheckTime(CurTime);

  KeyMode.update();
  KeyPlus.update();
  KeyMinus.update();

  // Check Keys pressing
  // only one key in time allowed
  bool fEvent = false;
  int  increment = 0;

  if(KeyMode.fell())
  {
    //Serial.println(F("Key mode"));
    if(fAlarm)
      alarm.Reset();
    else  
    {
      fEvent = true;

      if(Mode < MODE_SHOW_END)
      {
        ++Mode;
        if(Mode == MODE_SHOW_TEMP && !IsTPHpresent())
          //BME280 not inited
          ++Mode;
        if(Mode <= MODE_SHOW_BEGIN || Mode >= MODE_SHOW_END)
          Mode = MODE_SHOW_BEGIN + 1;
        if(Mode == MODE_SHOW_CLOCK)  
          TimeChanged = CHANGED_ALL;
      }
      else if(Mode < MODE_SET_END)
      {
        if(Mode == MODE_SET_TIME)
          Mode = MODE_CH_BEGIN + 1;
        else if(Mode == MODE_SET_ALARM)
          Mode = MODE_CH_ALARM_BEGIN + 1;
        else if(Mode == MODE_SHOW_VERSION)
        {
          Mode = MODE_SHOW_CLOCK;
          TimeChanged = CHANGED_ALL;
        }
      }
      else if(Mode < MODE_CH_END)
      {
        if(++Mode >= MODE_CH_END)
          Mode = MODE_SET_TIME;
      }
      else if(Mode < MODE_CH_ALARM_END)
      {
        if(Mode == MODE_CH_ALARM_MELODY)
          alarm.Reset();
        if(++Mode >= MODE_CH_ALARM_END)
          Mode = MODE_SET_ALARM;
      }
    }
  }
  else if(KeyPlus.fell())
  {
    //Serial.println(F("Key plus"));
    if(fAlarm)
      alarm.Reset(&CurTime);
    else
    {
      fEvent = true;

      if(Mode < MODE_SET_END)
      {
        if(Mode <= MODE_SET_BEGIN || ++Mode >= MODE_SET_END)
          Mode = MODE_SET_BEGIN + 1;
      }
      else
      {
        //change date/time
        ++increment;
      }
    }
  }
  else if(KeyMinus.fell())
  {
    //Serial.println(F("Key minus"));
    if(fAlarm)
      alarm.Reset(&CurTime);
    else
    {
      fEvent = true;

      if(Mode < MODE_SET_END)
      {
        if(--Mode <= MODE_SET_BEGIN)
          Mode = MODE_SET_END - 1;
      }
      else
      {
        //change date/time
        --increment;
      }
    }
  }

  //check long key pressing
  if(Mode == MODE_CH_HOUR
  || Mode == MODE_CH_MIN
  || Mode == MODE_CH_SEC
  || Mode == MODE_CH_DAY
  || Mode == MODE_CH_YEAR
  || Mode == MODE_CH_ALARM_HOUR
  || Mode == MODE_CH_ALARM_MIN
  )
  {
    static int KeyDelay = 0;
    if(!fEvent && !KeyPlus.read())
    {
      if(KeyPlus.currentDuration() >= LONG_PRESSED_PERIOD)
      {
        //long pressed 
        if(++KeyDelay >= KEY_DELAY)
        {
          fEvent = true;
          increment = 1;
          KeyDelay = 0;
        }
      }
    }
    else if(!fEvent && !KeyMinus.read())
    {
      if(KeyMinus.currentDuration() >= LONG_PRESSED_PERIOD)
      {
        //long pressed 
        if(++KeyDelay >= KEY_DELAY)
        {
          fEvent = true;
          increment = -1;
          KeyDelay = 0;
        }
      }
    }
    else
    {
      KeyDelay = 0;
    }
  }

  if(fEvent)
  {
    Beep();
    ModeTimeout = CurTime.secondstime() + KEY_TIMEOUT_PERIOD;
  }
  // Check Mode Timeout
  else if(Mode != MODE_SHOW_CLOCK && CurTime.secondstime() > ModeTimeout)
  {
    //Serial.println(F("reset mode"));
    ModeTimeout = 0;
    TimeChanged = SCROLL_ALL;//CHANGED_ALL;
    if(Mode == MODE_CH_ALARM_MELODY)
      alarm.Reset();
      
    Mode = MODE_SHOW_CLOCK;
  }

  // Apply Mode
  if(Mode == MODE_SHOW_CLOCK)
  {
    fBlink = false;
    if(TimeChanged)
    {
      uint8_t h, m, d, n;
      alarm.GetAlarm(d, h, m, n);
      bool alarm = (Alarm::DayOfTheWeek(CurTime.dayOfTheWeek()) & d) != 0;

      DisplayTime(CurHours, CurMins, CurSecs, alarm, TimeChanged, 0);
      SaveConfig();

#if AUTO_SHOW_PRESSURE != 0
      if(CurSecs == SHOW_PRESSURE_ON_SECS)
      {  
        Mode   = MODE_SHOW_PRESSURE;
        fEvent = true;
        ModeTimeout = CurTime.secondstime() + AUTO_SHOW_TEMP;
      }
      else 
#endif
#if AUTO_SHOW_TEMP != 0
      if(CurSecs == SHOW_TEMP_ON_SECS)
      {  
        Mode   = MODE_SHOW_TEMP;
        fEvent = true;
        ModeTimeout = CurTime.secondstime() + AUTO_SHOW_TEMP;
      }
      else 
#endif
#if AUTO_SHOW_DATE != 0
      if(CurSecs == SHOW_DATE_ON_SECS)
      {  
        Mode   = MODE_SHOW_DATE;
        fEvent = true;
        ModeTimeout = CurTime.secondstime() + AUTO_SHOW_DATE;
      }
      else
#endif      
      {
        fEvent = false;
      }

      if(fEvent && SWITCH_MODE_PERIOD && 0 != CurMins % SWITCH_MODE_PERIOD)
      {
          //skip for SWITCH_MODE_PERIOD
          Mode = MODE_SHOW_CLOCK;
          fEvent = false;
      }
    }
  }
  
  if(fEvent || fBlink)
  {
    if(Mode == MODE_SHOW_DATE)
    {
      sprintf(buff, "%d  %s. %d", CurTime.day(), Months[CurTime.month() - 1], CurTime.year() - 2000);
      PrintTinyString(buff, 0, 1);
    }
    else if(Mode == MODE_SHOW_TEMP || Mode == MODE_SHOW_PRESSURE)
    {
      float t, p, h;
      if(ReadTPH(t, p, h))
      {
        if(Mode == MODE_SHOW_TEMP)
        {
          int temp = t;
          int rh   = h;
          sprintf(buff, "%2d  %2d%%H", temp, rh);
          PrintTinyString(buff, 0, 1, true);
          PrintPictogram(8, &GradC[1], GradC[0]);
        }
        else
        {
#ifndef PRESSURE_IN_MM
          int hPa  = p / 100.0f;
          sprintf(buff, "%d  hPa", hPa);
#else
          int mmHg  = p / 133.332f;
          sprintf(buff, "%d   mm", mmHg);
#endif          
          PrintTinyString(buff, 1, 1);
        }
      }
      else
      {
        sprintf(buff, "BME280 ?");
        PrintTinyString(buff, 0, 1);
      }  
    }
    else if(Mode == MODE_SHOW_ALARM)
    {  
      uint8_t h, m, d, n;
      alarm.GetAlarm(d, h, m, n);
      sprintf(buff, "%02d:%02d", h, m);
      PrintTinyString(buff, 13, 1, true);
      PrintPictogram(3, &AlarmPic[1], AlarmPic[0]);
      PrintPictogram(0, &d, 1);
      uint8_t wd = Alarm::DayOfTheWeek(CurTime.dayOfTheWeek());
      PrintPictogram(1, &wd, 1);
    }
    else if(Mode == MODE_SET_TIME)
      PrintTinyString("Set>Time", 0, 1);
    else if(Mode == MODE_SET_ALARM)
      PrintTinyString("Set>Alrm", 0, 1);
    else if(Mode > MODE_CH_BEGIN && Mode < MODE_CH_END)
    {
      ChangeTime(Mode, increment);
      if(increment)
       {
         //time changed: update timeout
         ModeTimeout = CurTime.secondstime() + KEY_TIMEOUT_PERIOD;
       }
    }
    else if(Mode > MODE_CH_ALARM_BEGIN && Mode < MODE_CH_ALARM_END)
      ChangeAlarm(Mode, increment);
    else //if(Mode == MODE_SHOW_VERSION)
      PrintTinyString("O'Clk- " VERSION, 0, 1);
  
    ShowBuffer();
  }

  delay(LOOP_SLEEP_TIME);
}
