/* SerialFlash Library - for filesystem-like access to SPI Serial Flash memory
 * Copyright (C) 2016 LAtimes2
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "TemperatureSensor.h"

// Constructor
TemperatureSensor::TemperatureSensor ()
  : sensorExists (false)
{
  tempsensor = Adafruit_MPL3115A2();
}

bool TemperatureSensor::begin () {

  if (tempsensor.begin()) {
    sensorExists = true;
  } else {
    Serial.println("Couldn't find MPL3115A2!");
  }

  return sensorExists;
}

float TemperatureSensor::readTempC () {
  return tempsensor.getTemperature();
}

float TemperatureSensor::readTempF () {
  // convert celsius to fahrenheit
  return readTempC() * 9.0 / 5.0 + 32;;
}

