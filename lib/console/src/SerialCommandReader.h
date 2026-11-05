#pragma once

#include "ICommandProcessor.h"
#include <Arduino.h>

class SerialCommandReader {
  Stream &_stream;
  ICommandProcessor &_processor;
  unsigned long _prevMillis{0};
  unsigned long _period{100};

public:
  SerialCommandReader(Stream &stream, ICommandProcessor &processor);
  void poll(unsigned long currentMillis);
};
