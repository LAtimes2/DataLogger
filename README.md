# DataLogger
Teensy Simple Data Logger for Prop Shield

This sketch provides an example of recording temperature once a minute to the Flash memory on the Teensy prop shield.

If you don't have an SD card, you can use flash memory to log data. This example uses the prop shield to read temperature and to store data.

This example is meant to show 3 things:
1. Write data to flash using the SerialFlash library ()
2. Go into low power mode between samples using the Snooze library ()
3. Read temperature using the Adafruit_MPL3115A2 library ()
