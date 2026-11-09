#pragma once

#include <Arduino.h>

struct IWifiConnector {
  virtual void connect(const String &ssid, const String &password) = 0;
  virtual void disconnect() = 0;

protected:
  ~IWifiConnector() = default;
};
