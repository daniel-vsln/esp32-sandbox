#pragma once

#include <Arduino.h>

class WifiConnector {
  uint8_t _rgbPin;

  void turnLEDGreen() {}
  void turnLEDRed() {}
  void turnLEDWhite() {}

public:
  WifiConnector(uint8_t rgbPin, const char *ssid, const char *password);
};
