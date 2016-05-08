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

#include <SerialFlash.h>
#include "SerialFlashDataLogger.h"

const char *sampleRateFilename = "sampleRate";
const char *timeFilename = "time";
const char *dataFilename = "data.log";

SerialFlashFile dataFile;

bool SerialFlashDataLogger::begin (uint8_t pin) {
  return SerialFlash.begin (pin);
}

bool SerialFlashDataLogger::dataFileExists () {
  // check if this file is already on the Flash chip
  return SerialFlash.exists(dataFilename);
}

uint32_t SerialFlashDataLogger::getCapacity () {
  uint8_t chipId[5];

  // get capacity in bytes of chip
  SerialFlash.readID (chipId);
  return SerialFlash.capacity (chipId);
}

bool SerialFlashDataLogger::openDataFile (uint32_t size) {
  bool returnValue = false;

  if (SerialFlash.createErasable(dataFilename, size)) {
    dataFile = SerialFlash.open(dataFilename);
    returnValue = true;
  }

  return returnValue;  
}

void SerialFlashDataLogger::eraseAll () {
  SerialFlash.eraseAll ();

  // wait for erase to complete
  while (SerialFlash.ready() == false);
}

void SerialFlashDataLogger::eraseSampleRate () {
  if (SerialFlash.exists (sampleRateFilename))
  {
    SerialFlashFile sampleRateFile = SerialFlash.open(sampleRateFilename);
    SerialFlash.remove (sampleRateFile);
  }
}

bool SerialFlashDataLogger::readSampleRate (uint32_t &sampleMilliseconds) {
  bool returnValue = false;

  if (SerialFlash.exists (sampleRateFilename))
  {
    SerialFlashFile sampleRateFile = SerialFlash.open(sampleRateFilename);

    sampleRateFile.read (&sampleMilliseconds, 4);
    sampleRateFile.close ();

    returnValue = true;
  }

  return returnValue;
}

void SerialFlashDataLogger::writeSampleRate (uint32_t sampleMilliseconds) {

  eraseSampleRate ();

  // write to file
  SerialFlash.createErasable (sampleRateFilename, 4);
  SerialFlashFile sampleRateFile = SerialFlash.open(sampleRateFilename);

  sampleRateFile.write (&sampleMilliseconds, 4);
  sampleRateFile.close ();
}

void SerialFlashDataLogger::eraseTime () {
  if (SerialFlash.exists (timeFilename))
  {
    SerialFlashFile timeFile = SerialFlash.open(timeFilename);
    SerialFlash.remove (timeFile);
  }
}

bool SerialFlashDataLogger::readTime (uint32_t &time) {
  bool returnValue = false;

  if (SerialFlash.exists (timeFilename))
  {
    SerialFlashFile timeFile = SerialFlash.open(timeFilename);

    timeFile.read (&time, 4);
    timeFile.close ();

    returnValue = true;
  }

  return returnValue;
}

void SerialFlashDataLogger::writeTime (uint32_t time) {

  eraseTime ();

  // write to file
  SerialFlash.createErasable (timeFilename, 4);
  SerialFlashFile timeFile = SerialFlash.open(timeFilename);

  timeFile.write (&time, 4);
  timeFile.close ();
}

void SerialFlashDataLogger::smartEraseAll (uint32_t bufferSize, bool serialPrint) {

  // Assumption: If first 128 bytes in a block are FF, assume it is empty. This works because all files
  // are eraseable (start on a block boundary), and Maxfiles is 600, which only takes 1/2 a block.
  
  int blockSize = SerialFlash.blockSize();
  int capacity = getCapacity ();

  byte buffer[bufferSize];

  int address = 0;
  int block_start_address = 0;

  while (address < capacity) {
    // check block
    bool do_erase = false;

    // read first bytes (specified by bufferSize)
    SerialFlash.read (address, buffer, bufferSize);
    for (uint32_t index = 0; index < bufferSize; ++index) {
      if (buffer[index] != 0xFF) {
        do_erase = true;
        break;
      }
    }
    if (do_erase) {
      if (serialPrint) {
        Serial.print("Erase block at address ");
        Serial.println (address, HEX);
      }
      SerialFlash.eraseBlock (address);
    } else {
      address += bufferSize;
    }

    block_start_address += blockSize;
    address = block_start_address;
  }

  if (serialPrint) {
    Serial.println("  Erase complete");
  }
}

void SerialFlashDataLogger::listData() {

  const uint32_t BufferSize = 128;

  char buffer[BufferSize + 1];  // add 1 for string terminator
  bool done = false;
  uint32_t remainingLength;

  // set string terminator
  buffer[BufferSize] = 0;
  
  if (SerialFlash.exists (dataFilename))
  {
    SerialFlashFile dataFile = SerialFlash.open(dataFilename);

    remainingLength = dataFile.size ();

    while (!done && remainingLength > 0) {
      dataFile.read (buffer, BufferSize);

      for (uint32_t index = 0; index < BufferSize; ++index) {
        if (buffer[index] == 0xFF) {
          buffer[index] = 0;
        }
      }

      Serial.print (buffer);

      remainingLength -= BufferSize;
    }
    dataFile.close ();
  }
}

bool SerialFlashDataLogger::writeData (const void *buf, uint32_t wrlen) {

  uint32_t lengthWritten = 0;

  lengthWritten = dataFile.write (buf, wrlen);

  // return false if all the data was not written
  return (lengthWritten == wrlen);
}

/////////
//
// These are for debugging
//
/////////

// Displays all non-zero values in the flash chip
void SerialFlashDataLogger::hexDumpFlash() {

  const uint32_t BufferSize = 128;

  int blockSize = SerialFlash.blockSize();
  int capacity = getCapacity ();
  bool has_data = false;

  byte buffer[BufferSize];

  int address = 0;
  int block_start_address = 0;

  // read each address
  while (address < capacity) {

    // read buffer of data
    while (address < block_start_address + blockSize) {

      SerialFlash.read (address, buffer, BufferSize);

      // look for data
      for (uint32_t index = 0; index < BufferSize; ++index) {

        // look for not erased (not 0xFF)
        if (!has_data && buffer[index] != 0xFF) {

          // data found - print start address
          Serial.print(address + index, HEX);
          Serial.print (": ");
          has_data = true;

        }
        // look for end of data
        else if (has_data && buffer[index] == 0xFF) {

          // print end address
          Serial.print (" : end at ");
          Serial.print(address + index, HEX);
          Serial.println ("");
          has_data = false;
        }

        if (has_data) {
          // print the data
          Serial.print (buffer[index], HEX);
          Serial.print (",");
        }
      }
      address += BufferSize;
    }
    block_start_address += blockSize;
    address = block_start_address;
  }
  Serial.println("\nDump complete");
}

void SerialFlashDataLogger::listDirectory() {

  SerialFlash.opendir();

  while (1) {
    char filename[64];
    unsigned long filesize;

    if (SerialFlash.readdir(filename, sizeof(filename), filesize)) {
      Serial.print("  ");
      Serial.print(filename);
      printSpaces(20 - strlen(filename));
      Serial.print("  ");
      Serial.print(filesize);
      Serial.print(" bytes");
      Serial.println();
    } else {
      break; // no more files
    }
  }
}

void SerialFlashDataLogger::printSpaces(int num) {
  for (int i=0; i < num; i++) {
    Serial.print(" ");
  }
}




