# O-Clock
Arduino Simple LED Matrix 8x32 Alarm Clock.

Clock with big animated digits.
For alarm signal selected melody is playing.
Alarm can be armed for specified days of week.

Tested with Arduino Nano v3.0
Screen - 8x32 led matrix (SPI)
Time - rtc DS3231 (I2C)
Sensor - BME280 (I2C)
3 or 2 keys ('+', '-', 'Mode')
Brights adjustment with photoresistor
Passive piezo buzzer for melody playing

Project can be compiled in Arduino IDE or in PlatformIO

External librarys:
 - LEDMatrixDriver
 - RTClib
 - NonBlockingRTTTL
 - Bounce2
 - Adafruit Unified Sensor
 - Adafruit BME280 Library