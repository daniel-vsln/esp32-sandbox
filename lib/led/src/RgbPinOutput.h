#pragma once

#include "IRgbOutput.h"
#include <Arduino.h>

class RgbPinOutput : public IRgbOutput {
  uint8_t _pin;

public:
  explicit RgbPinOutput(uint8_t pin) : _pin{pin} {}

  void write(uint8_t red, uint8_t green, uint8_t blue) override {
    rgbLedWrite(_pin, red, green, blue);
  }
};
