#pragma once

#include <Arduino.h>
#include <ConnectionState.h>
#include <ITickable.h>
#include <IWifiConnector.h>
#include <WifiLed.h>

class WifiConnector : public ITickable, public IWifiConnector {
  ConnectionState _state = IDLE;
  WifiLed *_led;
  String _ssid;
  String _password;

public:
  explicit WifiConnector(WifiLed &led);

  void connect(const String &ssid, const String &password) override;
  void disconnect() override;
  void tick(uint32_t nowMs) override;
};
