# O-Clock
Arduino Simple LED Matrix 8x32 Alarm Clock.

It's clock with big animated digits.
For alarm signal selected melody is playing.
Alarm can be armed for specified days of week.
Clock can show temperature, humidity and date.

 - Tested with Arduino Nano v3.0
 - Screen - 8x32 led matrix (SPI)
 - Time - rtc DS3231 (I2C)
 - Sensor - BME280 (I2C) (Optional)
 - 3 keys ('+', '-', 'Mode') ('-' optional)
 - Brights adjustment with photoresistor (5528 Light Dependent Resistor LDR 5MM)
 - Passive piezo buzzer for melody playing

Project can be compiled in Arduino IDE or in PlatformIO

External libraries:
 - LEDMatrixDriver
 - RTClib
 - EEPROM
 - NonBlockingRTTTL
 - Bounce2
 - Adafruit Unified Sensor
 - Adafruit BME280 Library

Breadboard photo
 ![Screenshot](docs/breadboard1.jpg)

Schematic
 ![Screenshot](docs/Schematic_O-Clock.png)
 
---
Version 1.0 - initial version.

You can see how this clock works on [YouTube](https://www.youtube.com/watch?v=Dx4R_fJPpwI).
 
External libraries Dependency Graph:
- LEDMatrixDriver 0.2.2
   - SPI 1.0
   - Adafruit GFX Library 1.5.0
      - SPI 1.0
- RTClib 1.2.1
   - Wire 1.0
- EEPROM 2.0
- NonBlockingRTTTL 1.2.2
- Bounce2 2.52
- Adafruit Unified Sensor 1.0.3
- Adafruit BME280 Library 1.0.8
   - Adafruit Unified Sensor 1.0.3
   - SPI 1.0
   - Wire 1.0
