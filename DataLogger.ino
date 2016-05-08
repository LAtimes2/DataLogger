#include "SerialFlashDataLogger.h"
#include "TemperatureSensor.h"
#include <TimeLib.h>

#define SLEEP 0  // change to 1 to use the Snooze library to sleep between samples

#if SLEEP
#include <Snooze.h>

SnoozeBlock snoozeConfig;
int sleepTime = 0;
#endif

#define GMT_OFFSET -5            // conversion from GMT to local time, in hours
#define LED_TIME_NOT_LOGGING  1  // flash LED at this rate if not logging
#define LED_TIME_LOGGING     10  // flash LED at this rate if logging

// number of bytes at start of block that must be 0xFF to consider it erased
#define SMART_ERASE_SIZE 128

// hardware settings
#define LED_PIN 13
const int FlashChipSelect = 6; // digital pin for flash chip CS pin

bool flashFull = false;
bool logging = false;
uint32_t maxDataLength = 0;
uint32_t nextBlinkTime = 0;
uint32_t nextReadTime = 0;
uint32_t sampleMilliseconds = 60000;  // default is 60 seconds

TemperatureSensor theTemperature;
SerialFlashDataLogger dataLogger;

void setup() {

  Serial.begin(9600);

  // wait up to 1 second for Arduino Serial Monitor
  unsigned long startMillis = millis();
  while (!Serial && (millis() - startMillis < 1000)) ;

  Serial.println("");
  Serial.println("Data Logging");

  if (dataLogger.begin(FlashChipSelect)) {
    // maximum data size is all of chip except for 1 MByte for directory info, time, and sample rate
    maxDataLength = dataLogger.getCapacity () - 0x100000;
  } else {
    Serial.println(" *** Unable to access SPI Flash chip");
  }

  theTemperature.begin ();

  showMenu ();

  updateTime ();
  updateSampleRate ();

  // if no USB, start logging now
  if (!Serial) {
    logging = true;
    startLogging ();
  }
}

void loop() {

  float temperature;
  unsigned long currentTime;
  int blinkTime;
  String logString;

  if (Serial.available()) {
    processMessage();
  }

  // read temperature periodically
  if (millis() >= nextReadTime) {

    nextReadTime = nextReadTime + sampleMilliseconds;

    temperature = theTemperature.readTempF ();
    currentTime = now ();

    if (logging) {
      logString = valueToLogString (temperature, currentTime);

      if (!dataLogger.writeData (logString.c_str(), logString.length())) {
        if (!flashFull) {
          Serial.print ("Flash data is full - stopped writing\n");
          flashFull = true;
        }
      }
      // also print to serial port
      Serial.print(logString);
    }
  }

  // blink LED
  if (millis() >= nextBlinkTime) {
    if (logging && !flashFull) {
      blinkTime = LED_TIME_LOGGING * 1000;
    } else {
      blinkTime = LED_TIME_NOT_LOGGING * 1000;
    }
    nextBlinkTime = millis() + blinkTime;
    blinkled();
  }

#if SLEEP
  // compute shortest sleep time to next event
  sleepTime = nextBlinkTime - millis();
  int workingSleepTime = nextReadTime - millis();
  if (workingSleepTime < sleepTime) sleepTime = workingSleepTime;

  // only sleep if not connected via USB
  if (!Serial && sleepTime > 0) {
    snoozeConfig.setTimer (sleepTime);

    // sleep until timer goes off
    int who = Snooze.deepSleep (snoozeConfig);

    // update the clock for time sleeping
    systick_millis_count += sleepTime;
  }
#endif

}

void updateTime () {
  uint32_t pctime;

  if (dataLogger.readTime (pctime)) {

    setTime (pctime);

    // delete file to indicate time has been used
    dataLogger.eraseTime ();
  }
}

void updateSampleRate () {
  dataLogger.readSampleRate (sampleMilliseconds);
}

void processMessage() {
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  unsigned long pctime;
  byte inByte = Serial.read();

  blinkled ();

  if (inByte == 'd') {
    dataLogger.listDirectory();
  }

  if (inByte == 'e') {
    if (verifyErase ()) {
      dataLogger.eraseAll ();
      flashFull = false;
      Serial.println("  Erase complete");
    } else {
      Serial.println("  Not erased");      
    }
  }

  if (inByte == 'm') {
    if (verifyErase ()) {
      updateSampleRate();
      dataLogger.smartEraseAll (SMART_ERASE_SIZE, true);
      flashFull = false;

      // re-write sample rate after erasing
      dataLogger.writeSampleRate (sampleMilliseconds);
    }
  }

  if (inByte == 'n') {
    int seconds = Serial.parseInt();

    if (seconds > 0) {
      sampleMilliseconds = seconds * 1000;

      dataLogger.writeSampleRate (sampleMilliseconds);
    }

    updateSampleRate ();
    Serial.print("seconds = ");
    Serial.println(sampleMilliseconds / 1000);
  }

  if (inByte == 'l') {
    dataLogger.listData();
  }

  if (inByte == 's') {
    logging = !logging;
    if (logging) {
      startLogging();
    } else {
      Serial.println ("Stopped logging");
    }
  }

  if (inByte == 't') {

    // go to epochconverter.com to get current seconds since 1/1/1970

    pctime = Serial.parseInt();

    if (pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)

      // convert from GMT to local time
      pctime = pctime + GMT_OFFSET * 3600;

      setTime(pctime); // Sync Arduino clock to the time received on the serial port

      dataLogger.writeTime (pctime);

      Serial.print ("Time set to ");
      Serial.println (timeToString (pctime));
    } else {
      Serial.print ("Current time is ");
      Serial.println (timeToString (now ()));
    }
  }

  if (inByte == 'h') {
    dataLogger.hexDumpFlash();
  }

  if (inByte == '?') {
    showMenu();
  }

}

void blinkled () {

  // get current mode of LED pin (may be SPI clock)
  int currentMode = *portConfigRegister (LED_PIN);

  pinMode(LED_PIN, OUTPUT);

  digitalWriteFast(LED_PIN, HIGH);
  delay(150);
  digitalWriteFast(LED_PIN, LOW);
  delay(100);

  // restore mode of LED pin
  *portConfigRegister (LED_PIN) = currentMode;
}

String valueToLogString (float value, time_t currentTime) {

  return String(value) + ", " + timeToString (currentTime) + "\n";
}

String timeToString(time_t t){

  tmElements_t tm;
  String timeString = "";

  breakTime (t, tm);

  // digital clock display of the time
  timeString = String(tm.Hour);
  timeString += ":";
  timeString += digitToString(tm.Minute);
  timeString += ":";
  timeString += digitToString(tm.Second);
  timeString += " ";
  timeString += String(tm.Month);
  timeString += "/";
  timeString += String(tm.Day);
  timeString += "/";
  timeString += String(tmYearToCalendar(tm.Year));

  return timeString;
}

String digitToString(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  String digitString = "";
  if(digits < 10) {
    // add leading 0
    digitString += '0';
  }
  digitString += String(digits);

  return digitString;
}

void showMenu () {
  Serial.println("");
  Serial.println("User Menu :");
  Serial.println("");
  Serial.println("e : erase Flash");
  Serial.println("l : list (display) the logged data from Flash");
  Serial.println("m : smart erase");
  Serial.println("n# : number of seconds between samples");
  Serial.println("s : start/stop logging");
  Serial.println("t# : set date/time (from epochconverter.com or similar)");
  Serial.println("  Debugging commands:");
  Serial.println("d : directory of files in Flash");
  Serial.println("h : hex dump of flash memory");
  Serial.println("? : show this menu");
  Serial.println("");
}

void startLogging() {
  
  // check if this file is already on the Flash chip
  if (dataLogger.dataFileExists()) {
    logging = false;
    Serial.println(" Data already exists on the Flash chip - not logging");

    if (!Serial) {
      blinkled();
      blinkled();
      blinkled();
    }
  } else {
    // create the file on the Flash chip
    if (dataLogger.openDataFile (maxDataLength)) {
      Serial.println("  File created");
    } else {
      logging = false;
      Serial.println("  Error creating file");

      if (!Serial) {
        blinkled();
        blinkled();
      }
    }
  }
}

bool verifyErase() {
  uint32_t startTime = millis();
  Serial.println ("Are you sure you want to erase entire SPI Flash? y/[n]");

  // wait up to 5 seconds for a response
  while ((millis() - startTime) < 5000) {
    if (Serial.available()) {
      byte inByte = Serial.read();
      if (inByte == 'y') {
        return true;
      // if a non-character (line feed), ignore it
      } else if (inByte < '0') {
      // if any other character, return false
      } else {
        return false;
      }        
    }
  }
  return false;
}


