/*
FreeBSD License

Copyright (c) 2019, vikonix: valeriy.kovalev.software@gmail.com
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

#ifndef __ALARM_H__
#define __ALARM_H__

#include <Arduino.h>
#include <RTClib.h>
#include <NonBlockingRtttl.h>

//////////////////////////////////////////////////////////////////////////////
// Class for alarm management and playing melody when alarm went off 

#define SNOOZE_TIME (5 * 60)  //snooze period in seconds
#define ALARM_TIME  30        //alarm time in seconds

//#define DEBUG_ALARM

class Alarm
{
protected:
  uint8_t             m_Pin;
  const char* const*  m_MelodyArray;
  uint8_t             m_MelodyCount;
  
  //alarm config
  uint8_t             m_Days; //bit mask for week days Mon->0x1 Sum->0x40
  uint8_t             m_Hour;
  uint8_t             m_Min;
  uint8_t             m_Melody;

  char*               m_Buffer;
  bool                m_Play;
  long                m_Snooze;

public:
  // melody array in RTTTL format in PROGMEM
  // example: "Popcorn:d=4,o=5,b=125:8c6,8a#,8c6,8g,8d#,8g,c,8c6,8a#,8c6,8g,8d#,8g,c,8c6,8d6,8d#6,16c6,8d#6,16c6,8d#6,8d6,16a#,8d6,16a#,8d6,8c6,8a#,8g,8a#,c6";
  Alarm(uint8_t pin, const char* const* MelodyArray, uint8_t MelodyCount) :
    m_Pin(pin),
    m_MelodyArray(MelodyArray),
    m_MelodyCount(MelodyCount),
    m_Days(0),
    m_Hour(0),
    m_Min(0),
    m_Melody(0),
    m_Buffer(nullptr),
    m_Play(false),
    m_Snooze(0)
  {}  
  ~Alarm()
  {
    delete m_Buffer;
  }

  //Convert from day number "0-Sun .. 6-Sat" to 1 bit "0-Mon .. 6-Sun"
  static uint8_t DayOfTheWeek(uint8_t day) 
  {
    if(day == 0)//Sunday
      day = 6;
    else
      day >>= 1;
    return 1 << day;
  }

  void SetAlarm(uint8_t days, uint8_t hour, uint8_t mins, uint8_t melody)  
  {
    m_Days    = days;
    m_Hour    = hour; 
    m_Min     = mins;
    m_Melody  = melody;
    if(m_Melody >= m_MelodyCount)
      m_Melody = 0;

    const char* pMelody = (const char*)pgm_read_word(&(m_MelodyArray[m_Melody]));
    size_t len = strlen_P(pMelody);
    delete m_Buffer;
    m_Buffer = new char[len + 1];
    if(m_Buffer)
      strcpy_P(m_Buffer, pMelody);
  }
  
  void GetAlarm(uint8_t& days, uint8_t& hour, uint8_t& mins, uint8_t& melody)
  {
    days      = m_Days;
    hour      = m_Hour;
    mins      = m_Min;
    melody    = m_Melody;
  }
  
  void Play() 
  {
#ifdef DEBUG_ALARM    
    Serial.println(F("Alarm.Play"));
#endif    
    if(m_Buffer)
      rtttl::begin(m_Pin, m_Buffer);
  }
  
  bool CheckTime(const DateTime& time)
  {
    rtttl::play();

    //check for restarting melody
    if(m_Play)
    {
      if(rtttl::done())
      {
        if(m_Snooze + ALARM_TIME > time.secondstime())
          Play();
        else
          //60 seconds timeout done
          m_Play = false;  
      }
    }
    else if(m_Snooze == time.secondstime())
    {
#ifdef DEBUG_ALARM    
      Serial.println(F("Snooze"));
#endif      
      m_Play = true;
      Play();
    }
    else if(
      (m_Days & DayOfTheWeek(time.dayOfTheWeek()))
    && m_Hour == time.hour()
    && m_Min  == time.minute()
    && 0      == time.second()
    )
    {
#ifdef DEBUG_ALARM    
      Serial.println(F("Alarm"));
#endif      
      m_Play = true;
      m_Snooze = time.secondstime();
      Play();
    }

    return m_Play;
  }

  void Reset(const DateTime* time = nullptr)                   
  {
#ifdef DEBUG_ALARM    
    Serial.println(F("Alarm.Reset"));
#endif    
    m_Play = false;
    rtttl::stop();

    if(!time)
      //end alarm
      m_Snooze = 0;
    else
      //add snooze period
      m_Snooze = time->secondstime() + SNOOZE_TIME;
  }
};

#endif //__ALARM_H__