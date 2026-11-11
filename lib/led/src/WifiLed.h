#pragma once

#include "ILed.h"
#include "IRgbOutput.h"
#include <ITickable.h>
#include <stdint.h>

class WifiLed : public ITickable, public ILed {
  static constexpr uint32_t BLINK_HALF_PERIOD_MS = 500;

  IRgbOutput &_output;
  uint32_t _color = 0;
  bool _isBlinking = false;
  bool _blinkRestartPending = false;
  uint32_t _blinkStartMs = 0;
  bool _hasWritten = false;
  uint32_t _writtenRgb = 0;

public:
  explicit WifiLed(IRgbOutput &output);

  void tick(uint32_t nowMs) override;

  void turnOff() override;
  void turnGreen() override;
  void turnRed() override;
  void turnWhite() override;
  void turnBlinkOn() override;
  void turnBlinkOff() override;

  void turnGreenBlinkOn();
  void turnRedBlinkOn();
  void turnGreenSolidOn();
  void turnRedSolidOn();

  bool isBlinking() const;
  bool isGreen() const;
  bool isRed() const;
  bool isWhite() const;
  bool isOff() const;
  uint32_t color() const;
};
