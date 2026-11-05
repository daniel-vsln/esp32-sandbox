#pragma once

#include <Arduino.h>

class ICommandProcessor {
public:
  virtual ~ICommandProcessor() = default;
  // Повертає true, якщо команду знайдено й виконано
  virtual bool process(const String &line) = 0;
};
