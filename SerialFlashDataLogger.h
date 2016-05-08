/* SerialFlashDataLogger Library - for filesystem-like access to SPI Serial Flash memory
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

#ifndef SerialFlashDataLogger_h_
#define SerialFlashDataLogger_h_

class SerialFlashDataLogger
{
public:

  bool begin (uint8_t pin = 6);
  bool dataFileExists ();
  uint32_t getCapacity ();
  bool openDataFile (uint32_t size);

  void eraseAll ();
  void smartEraseAll (uint32_t bufferSize, bool serialPrint);
  void listData ();
  void writeData ();
  bool writeData (const void *buf, uint32_t wrlen);
  void eraseSampleRate ();
  bool readSampleRate (uint32_t &sampleMilliseconds);
  void writeSampleRate (uint32_t sampleMilliseconds);
  void eraseTime ();
  bool readTime (uint32_t &time);
  void writeTime (uint32_t time);

  void hexDumpFlash ();
  void listDirectory ();

private:
  void printSpaces(int num);
};

#endif
