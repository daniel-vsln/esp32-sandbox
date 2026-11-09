#pragma once

#include <stdint.h>

struct IRgbOutput {
  virtual void write(uint8_t red, uint8_t green, uint8_t blue) = 0;

protected:
  ~IRgbOutput() = default;
};
