#include "WifiConnector.h"
#include <WifiStatusView.h>

static_assert(WifiReason::ASSOC_LEAVE == WIFI_REASON_ASSOC_LEAVE);
static_assert(WifiReason::FOUR_WAY_HANDSHAKE_TIMEOUT ==
              WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT);
static_assert(WifiReason::BEACON_TIMEOUT == WIFI_REASON_BEACON_TIMEOUT);
static_assert(WifiReason::NO_AP_FOUND == WIFI_REASON_NO_AP_FOUND);
static_assert(WifiReason::AUTH_FAIL == WIFI_REASON_AUTH_FAIL);
static_assert(WifiReason::HANDSHAKE_TIMEOUT == WIFI_REASON_HANDSHAKE_TIMEOUT);

WifiConnector::WifiConnector(ILed &led) : _led{led} {}

WifiState WifiConnector::state() const { return _machine.state(); }

bool WifiConnector::begin() {
  WiFi.setAutoReconnect(false);
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    onEvent(event, info);
  });
  return true;
}

void WifiConnector::onEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    publish(WifiLinkStatus::Up);
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    publish(WifiLinkStatus::GotIp);
    break;
  case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    publish(WifiLinkStatus::LostIp);
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    _linkDownReason.store(info.wifi_sta_disconnected.reason,
                          std::memory_order_relaxed);
    publish(WifiLinkStatus::Down);
    break;
  default:
    break;
  }
}

void WifiConnector::publish(WifiLinkStatus status) {
  _linkStatus.store(status, std::memory_order_release);
}

void WifiConnector::tick(uint32_t) {
  WifiLinkStatus status =
      _linkStatus.exchange(WifiLinkStatus::None, std::memory_order_acquire);

  switch (status) {
  case WifiLinkStatus::None:
    break;
  case WifiLinkStatus::Up:
    apply({WifiEventKind::LinkUp, 0});
    break;
  case WifiLinkStatus::GotIp:
    apply({WifiEventKind::GotIp, 0});
    break;
  case WifiLinkStatus::LostIp:
    apply({WifiEventKind::LostIp, 0});
    break;
  case WifiLinkStatus::Down:
    apply({WifiEventKind::LinkDown,
           _linkDownReason.load(std::memory_order_relaxed)});
    break;
  }

  showStateIfChanged();
}

void WifiConnector::connect(const String &ssid, const String &password) {
  _ssid = ssid;
  _password = password;
  apply({WifiEventKind::ConnectRequested, 0});
}

void WifiConnector::disconnect() {
  apply({WifiEventKind::DisconnectRequested, 0});
}

void WifiConnector::apply(const WifiEvent &event) {
  WifiState before = _machine.state();
  WifiAction action = _machine.handle(event);
  log(before, event);
  perform(action);
}

void WifiConnector::perform(WifiAction action) {
  switch (action) {
  case WifiAction::None:
    break;
  case WifiAction::Begin:
    WiFi.begin(_ssid.c_str(), _password.c_str());
    break;
  case WifiAction::Disconnect:
    WiFi.disconnect();
    break;
  }
}

void WifiConnector::log(WifiState before, const WifiEvent &event) {
  if (event.kind == WifiEventKind::LinkDown &&
      event.reason != WifiReason::ASSOC_LEAVE && before != WifiState::Idle &&
      before != WifiState::Failed) {
    Serial.printf("[WiFi] Link down: %s (reason %u), retries left %u\n",
                  describeDisconnectReason(event.reason), event.reason,
                  _machine.retriesLeft());
  }

  WifiState after = _machine.state();
  if (after == before)
    return;

  Serial.printf("[WiFi] %s -> %s\n", wifiStateName(before),
                wifiStateName(after));
  if (after == WifiState::Connecting)
    Serial.printf("[WiFi] Connecting to %s\n", _ssid.c_str());
  if (after == WifiState::Connected)
    Serial.printf("[WiFi] IP address: %s\n",
                  WiFi.localIP().toString().c_str());
}

void WifiConnector::showStateIfChanged() {
  WifiState current = _machine.state();
  if (_hasShownState && current == _shownState)
    return;
  _hasShownState = true;
  _shownState = current;
  showWifiState(_led, current);
}
