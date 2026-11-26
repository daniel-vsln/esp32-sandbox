#pragma once

#include "FreeRtosWifiEventQueue.h"
#include <Arduino.h>
#include <ILed.h>
#include <ITickable.h>
#include <IWifiConnector.h>
#include <WiFi.h>
#include <WifiEventInbox.h>
#include <WifiStateMachine.h>

class WifiConnector : public ITickable, public IWifiConnector {
  ILed &_led;
  FreeRtosWifiEventQueue _queue;
  WifiEventInbox _inbox{_queue};
  WifiStateMachine _machine;
  bool _started = false;
  bool _hasShownState = false;
  WifiState _shownState = WifiState::Idle;
  String _ssid;
  String _password;

  void onEvent(WiFiEvent_t event, WiFiEventInfo_t info);
  void apply(const WifiEvent &event, uint32_t nowMs);
  void perform(WifiAction action);
  void log(WifiState before, uint8_t retriesBefore, const WifiEvent &event,
           WifiAction action);
  void reportDroppedEvents();
  void showStateIfChanged();

public:
  explicit WifiConnector(ILed &led);

  bool begin();
  void connect(const String &ssid, const String &password) override;
  void disconnect() override;
  void tick(uint32_t nowMs) override;

  WifiState state() const;
};
