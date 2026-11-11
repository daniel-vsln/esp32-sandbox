#include "WifiLed.h"

namespace {
constexpr uint32_t GREEN = 0x00FF00;
constexpr uint32_t RED = 0xFF0000;
constexpr uint32_t WHITE = 0xFFFFFF;
constexpr uint32_t BLACK = 0x000000;
} // namespace

WifiLed::WifiLed(IRgbOutput &output) : _output{output} {}

bool WifiLed::isBlinking() const { return _isBlinking; }

uint32_t WifiLed::color() const { return _color; }

void WifiLed::tick(uint32_t nowMs) {
  if (_blinkRestartPending) {
    _blinkStartMs = nowMs;
    _blinkRestartPending = false;
  }

  bool lit =
      !_isBlinking || ((nowMs - _blinkStartMs) / BLINK_HALF_PERIOD_MS) % 2 == 0;
  uint32_t rgb = lit ? _color : BLACK;

  if (_hasWritten && rgb == _writtenRgb)
    return;

  _hasWritten = true;
  _writtenRgb = rgb;
  _output.write((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

void WifiLed::turnOff() { _color = BLACK; }

void WifiLed::turnGreen() { _color = GREEN; }

void WifiLed::turnRed() { _color = RED; }

void WifiLed::turnWhite() { _color = WHITE; }

bool WifiLed::isGreen() const { return _color == GREEN; }

bool WifiLed::isRed() const { return _color == RED; }

bool WifiLed::isWhite() const { return _color == WHITE; }

bool WifiLed::isOff() const { return _color == BLACK; }

void WifiLed::turnBlinkOn() {
  if (_isBlinking)
    return;
  _isBlinking = true;
  _blinkRestartPending = true;
}

void WifiLed::turnBlinkOff() {
  _isBlinking = false;
  _blinkRestartPending = false;
}

void WifiLed::turnGreenBlinkOn() {
  turnGreen();
  turnBlinkOn();
}

void WifiLed::turnRedBlinkOn() {
  turnRed();
  turnBlinkOn();
}

void WifiLed::turnGreenSolidOn() {
  turnGreen();
  turnBlinkOff();
}
void WifiLed::turnRedSolidOn() {
  turnRed();
  turnBlinkOff();
}