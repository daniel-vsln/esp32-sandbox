#pragma once

#include <Arduino.h>

class ICommandProcessor {
public:
  virtual ~ICommandProcessor() = default;
  virtual bool process(const String &line) = 0;
};
