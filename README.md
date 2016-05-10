# DataLogger
Teensy Simple Data Logger for Prop Shield

This sketch provides an example of recording temperature once a minute to the Flash memory on the Teensy prop shield.

If you don't have an SD card, you can use flash memory to log data. This example uses the prop shield to read temperature and to store data.

This example shows 3 things:
* Write data to flash using the SerialFlash library (https://github.com/PaulStoffregen/SerialFlash.git)
* (needs to be enabled via #define) Go into low power mode between samples using the Snooze library (https://github.com/duff2013/Snooze.git)
* Read temperature on prop shield using the Adafruit_MPL3115A2 library (https://github.com/adafruit/Adafruit_MPL3115A2_Library.git)

If you don't have the libraries installed, here is how I do it:

1. Go to website of the library (*.git)
2. select 'Download ZIP' and save the file
3. In Teensyduino, select Sketch -> Include Library -> Add .ZIP Library ...
4. Browse to the downloaded file and open it

Configurable at compile time:

1. Enable Snooze library by setting #define SLEEP 1
2. Set Local time offset by setting GMT_OFFSET to hours ahead or behind GMT (UTC). e.g. Chicago is -5 during daylight saving time.

Typical usage:

1. erase (e) or smart erase (m) to delete previous data in Flash
2. [optional] change sample rate (n#) from default of 60 seconds, e.g. n5 to sample every 5 seconds
3. [optional] if no real time clock (RTC), set time
  * Go to epochconverter.com and copy timestamp
  * Use (t) command to set time, e.g. t1462759642 to set time to 09 May 2016 02:07:22 GMT
4. start logging (s)
5. LED flashes every 1 second when not logging, 10 seconds when logging
6. stop logging (s)
7. list the data (l)
8. copy and paste the data into a spreadsheet as a CSV file

Typical usage when using a battery or USB charger:

1. erase (e) or smart erase (m) to delete previous data in Flash
2. [optional] change sample rate (n#) from default of 60 seconds, e.g. n5 to sample every 5 seconds
3. [optional] if no real time clock, set time
  * Go to epochconverter.com and copy timestamp
  * Use (t) command to set time, e.g. t1462759642 to set time to 09 May 2016 02:07:22 GMT
4. unplug USB from PC and connect battery or charger
5. Time will be read and set the first time it turned on without USB serial port
6. Logging will start automatically without USB serial port
7. LED flashes every 1 second when not logging, 10 seconds when logging
8. Turn off Teensy to stop recording
9. Plug USB into PC
10. list the data (l)
11. copy and paste the data into a spreadsheet as a CSV file

Note: Turning on the Teensy a second time on the battery will not log any more data. Data must be erased before logging again. This was because the RTC resets to 1970 on second powerup. If you have an RTC, you would have to find the end of the data in the file and seek to that position before continuing logging.
