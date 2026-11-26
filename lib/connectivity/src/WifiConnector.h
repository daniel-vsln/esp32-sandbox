#pragma once

#include <Arduino.h>
#include <ILed.h>
#include <ITickable.h>
#include <IWifiConnector.h>
#include <WiFi.h>
#include <WifiStateMachine.h>
#include <atomic>

enum class WifiLinkStatus : uint8_t { None, Up, GotIp, LostIp, Down };

class WifiConnector : public ITickable, public IWifiConnector {
  ILed &_led;
  std::atomic<WifiLinkStatus> _linkStatus{WifiLinkStatus::None};
  std::atomic<uint8_t> _linkDownReason{0};
  WifiStateMachine _machine;
  bool _hasShownState = false;
  WifiState _shownState = WifiState::Idle;
  String _ssid;
  String _password;

  void onEvent(WiFiEvent_t event, WiFiEventInfo_t info);
  void publish(WifiLinkStatus status);
  void apply(const WifiEvent &event);
  void perform(WifiAction action);
  void log(WifiState before, const WifiEvent &event);
  void showStateIfChanged();

public:
  explicit WifiConnector(ILed &led);

  bool begin();
  void connect(const String &ssid, const String &password) override;
  void disconnect() override;
  void tick(uint32_t nowMs) override;

  WifiState state() const;
};
