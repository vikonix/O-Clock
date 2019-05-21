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

//////////////////////////////////////////////////////////////////////////////
//O'Clock - simple matrix clock
//Tested with Arduino Nano v3.0
//Screen - 8x32 led matrix (SPI)
//Time - rtc DS3231 (I2C)
//Sensor - BME280 (I2C)
//3 or 2 keys ('+', '-', 'Mode')
//Brights adjustment with photoresistor
//Passive piezo buzzer for melody playing

#include <EEPROM.h>
#include <LEDMatrixDriver.hpp>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "oclock.h"


// Define the ChipSelect pin for the led matrix (Dont use the SS or MISO pin of your Arduino!)
// Other pins are Arduino specific SPI pins (MOSI=DIN, SCK=CLK of the LEDMatrix)
// see https://www.arduino.cc/en/Reference/SPI
// Connect DIN->D11 CLK->D13
const uint8_t LEDMATRIX_CS_PIN = 9; //D9

// buzzer pin
const uint8_t Buzzer_Pin       = 8; //D8 Passive piezo buzzer connected to +5V
// photo resistor pin
const uint8_t Photo_Pin        = A3; //Photoresistor connected to +5V with 10KOm to -V

const uint8_t KeyMode_Pin      = 4; //D4 //Keys connected to -V
const uint8_t KeyPlus_Pin      = 5; //D5 -//-
const uint8_t KeyMinus_Pin     = 6; //D6 -//- (optional)

// Number of 8x8 segments you are connecting
const int LEDMATRIX_SEGMENTS   = 4;
const int LEDMATRIX_WIDTH      = LEDMATRIX_SEGMENTS * 8;

// screen buffers
byte screen_buffer[LEDMATRIX_WIDTH];

// The display object
LEDMatrixDriver lmd(LEDMATRIX_SEGMENTS, LEDMATRIX_CS_PIN);

// The clock object
// Connect SDA->A4 SCL->A5
RTC_DS3231 rtc; // I2C

// BME 280 module (optional)
// BME280 Address fixed in library Adafruit_BME280.h
// original Address is 0x77, China module Address is 0x76.
// Don't forgot to set next define with correct Address !!!
#define MY_BME280_ADDRESS (0x76)
#define SEALEVELPRESSURE_HPA (1013.25)

// Connect SDA->A4 SCL->A5
Adafruit_BME280 bme; // I2C

// Key objects
Bounce KeyMode  = Bounce();
Bounce KeyPlus  = Bounce();
Bounce KeyMinus = Bounce();

int EepromSize = EEPROM.length();

//////////////////////////////////////////////////////////////////////////////
// alarm melody in RTTTL format
const char melody1[] PROGMEM = "Aha:d=4,o=4,b=160:8f#5,8f#5,8f#5,8d5,8p,8b,8p,8e5,8p,8e5,8p,8e5,8g#5,8g#5,8a5,8b5,8a5,8a5,8a5,8e5,8p,8d5,8p,8f#5,8p,8f#5,8p,8f#5,8e5,8e5,8f#5,8e5,8f#5,8f#5,8f#5,8d5,8p,8b,8p,8e5,8p,8e5,8p,8e5,8g#5,8g#5,8a5,8b5,8a5,8a5,8a5,8e5,8p,8d5,8p,8f#5,8p,8f#5,8p,8f#5,8e5,8e5";
const char melody2[] PROGMEM = "Europe:d=4,o=5,b=125:p,8p,16b,16a,b,e,p,8p,16c6,16b,8c6,8b,a,p,8p,16c6,16b,c6,e,p,8p,16a,16g,8a,8g,8f#,8a,g.,16f#,16g,a.,16g,16a,8b,8a,8g,8f#,e,c6,2b.,16b,16c6,16b,16a,1b";
const char melody3[] PROGMEM = "IndianaJ:d=4,o=5,b=250:e,8p,8f,8g,8p,1c6,8p.,d,8p,8e,1f,p.,g,8p,8a,8b,8p,1f6,p,a,8p,8b,2c6,2d6,2e6,e,8p,8f,8g,8p,1c6,p,d6,8p,8e6,1f.6,g,8p,8g,e.6,8p,d6,8p,8g,e.6,8p,d6,8p,8g,f.6,8p,e6,8p,8d6,2c6";
const char melody4[] PROGMEM = "Popcorn:d=4,o=5,b=125:8c6,8a#,8c6,8g,8d#,8g,c,8c6,8a#,8c6,8g,8d#,8g,c,8c6,8d6,8d#6,16c6,8d#6,16c6,8d#6,8d6,16a#,8d6,16a#,8d6,8c6,8a#,8g,8a#,c6";
const char melody5[] PROGMEM = "Axel-F:d=4,o=5,b=125:g,8a#.,16g,16p,16g,8c6,8g,8f,g,8d.6,16g,16p,16g,8d#6,8d6,8a#,8g,8d6,8g6,16g,16f,16p,16f,8d,8a#,2g,p,16f6,8d6,8c6,8a#,g,8a#.,16g,16p,16g,8c6,8g,8f,g,8d.6,16g,16p,16g,8d#6,8d6,8a#,8g,8d6,8g6,16g,16f,16p,16f,8d,8a#,2g";

const char* const MelodyArray[] PROGMEM = {melody1, melody2, melody3, melody4, melody5};
const int MelodiesCount = sizeof(MelodyArray) / sizeof(const char*);

Alarm alarm(Buzzer_Pin, MelodyArray, MelodiesCount);

//////////////////////////////////////////////////////////////////////////////
// current time
DateTime CurTime;
int CurHours = -1;//not inited
int CurMins  = 0;
int CurSecs  = 0;

int GetCurTime()
{
  int h = CurHours;
  int m = CurMins;
  int s = CurSecs;

  CurTime  = rtc.now();
  CurHours = CurTime.hour();
  CurMins  = CurTime.minute();
  CurSecs  = CurTime.second();

  if(h == -1)
  {
    // first call
    return CHANGED_ALL;
  }
  // check modified digits
  int mod = CHANGED_NOTHING;
  if(h / 10 != CurHours / 10)
    mod = CHANGED_HOUR10;
  else if(h % 10 != CurHours % 10)
    mod = CHANGED_HOUR1;
  else if(m / 10 != CurMins / 10)
    mod = CHANGED_MIN10;
  else if(m % 10 != CurMins % 10)
    mod = CHANGED_MIN1;
  else if(s != CurSecs)
    mod = CHANGED_SEC;

  return mod;
}


//////////////////////////////////////////////////////////////////////////////
void SetTime(const DateTime& dt)
{
  rtc.adjust(dt);
}


//////////////////////////////////////////////////////////////////////////////
void AdjustBright()
{
  // current bright level
  static int CurBrightLevel = 0;

  // logarithmic level
  byte level[] = {0,0,0,0,0,0,1,2,4,8}; // 0 = low, 15 = high
  // maximal analog value
  const int MaxAnalogValue = 1023;

  int v = analogRead(Photo_Pin);
  // calc linear level
  int l = (v * sizeof(level)) / MaxAnalogValue;
  // get logarithmic level
  l = level[l];

  if(CurBrightLevel != l)
  {
    CurBrightLevel = l;
    lmd.setIntensity(CurBrightLevel);
  }
}


//////////////////////////////////////////////////////////////////////////////
void Beep(int n)
{
  if(n < 1 || n > 5)
    n = 1;
  while(n--)
  {
    tone(Buzzer_Pin, 500, 10);
    if(n)
      delay(150);
    else
      delay(10);
  }
  noTone(Buzzer_Pin);
}

void BeepError()
{
  tone(Buzzer_Pin, 500, 1000);
  delay(1000);
  noTone(Buzzer_Pin);
}


//////////////////////////////////////////////////////////////////////////////
void ShowBuffer(byte* buffer)
{
  if(buffer && screen_buffer != buffer)
    memcpy(screen_buffer, buffer, LEDMATRIX_WIDTH);

  for(byte row = 0; row < LEDMATRIX_WIDTH; row++)
    lmd.setColumn(row, screen_buffer[row]);
  lmd.display();
}

/////////////////////////////////////////////////////////////////////////////
bool fBME280present = false; 
bool IsTPHpresent()
{
  return fBME280present;
}

bool ReadTPH(float& Temperature, float& Pressure, float& Humidity)
{
  if(!fBME280present)
    return false;
  bme.takeForcedMeasurement();
  Temperature = bme.readTemperature();
  Pressure    = bme.readPressure();
  Humidity    = bme.readHumidity();
  return true;
}

/////////////////////////////////////////////////////////////////////////////
void setup()
{
  pinMode(Buzzer_Pin, OUTPUT); // Set buzzer an output
  Beep();

  // init the display
  lmd.setEnabled(true);
  lmd.setIntensity(0); // 0 = low, 15 = high
  lmd.clear();

  Serial.begin(9600);
  Serial.println(F("Vikonix O'Clock Version:" VERSION));
  //Serial.print(F("EEPROM size=")); Serial.println(EepromSize, DEC);

  if(!rtc.begin())
  {
    Serial.println(F("Rtc ERROR"));
    BeepError();
  }

  if(rtc.lostPower())
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Beep(2);
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  KeyMode.attach(KeyMode_Pin, INPUT_PULLUP);
  KeyMode.interval(20);
  KeyPlus.attach(KeyPlus_Pin, INPUT_PULLUP);
  KeyPlus.interval(20);
  KeyMinus.attach(KeyMinus_Pin, INPUT_PULLUP);
  KeyMinus.interval(20);

  LoadConfig();

  if(!bme.begin(MY_BME280_ADDRESS)) 
    Serial.println(F("BME280 sensor not found"));
  else
  {
    Serial.println(F("BME280 sensor Ok"));
    fBME280present = true;
    bme.setSampling(
      Adafruit_BME280::MODE_FORCED,
      Adafruit_BME280::SAMPLING_X1, // temperature
      Adafruit_BME280::SAMPLING_X1, // pressure
      Adafruit_BME280::SAMPLING_X1, // humidity
      Adafruit_BME280::FILTER_OFF   
    );
  }
}

