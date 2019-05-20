# O-Clock
Arduino Simple LED Matrix 8x32 Alarm Clock.

It’s clock with big animated digits.
For alarm signal selected melody is playing.
Alarm can be armed for specified days of week.
Clock can show temperature, humidity and date.

 - Tested with Arduino Nano v3.0
 - Screen - 8x32 led matrix (SPI)
 - Time - rtc DS3231 (I2C)
 - Sensor - BME280 (I2C) (Optional)
 - 3 or 2 keys ('+', '-', 'Mode')
 - Brights adjustment with photoresistor
 - Passive piezo buzzer for melody playing

Project can be compiled in Arduino IDE or in PlatformIO

External libraries:
 - LEDMatrixDriver
 - RTClib
 - NonBlockingRTTTL
 - Bounce2
 - Adafruit Unified Sensor
 - Adafruit BME280 Library

 ![Screenshot](docs/breadboard1.jpg)

 ![Screenshot](docs/breadboard2.jpg)
 
 Version 1.0 - initial version.
