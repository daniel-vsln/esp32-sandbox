#pragma once

struct ILed {
  virtual void turnOff() = 0;
  virtual void turnGreen() = 0;
  virtual void turnRed() = 0;
  virtual void turnWhite() = 0;
  virtual void turnBlinkOn() = 0;
  virtual void turnBlinkOff() = 0;

protected:
  ~ILed() = default;
};
