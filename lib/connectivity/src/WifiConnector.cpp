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
  _queue.create();
  WiFi.setAutoReconnect(false);
  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    onEvent(event, info);
  });
  return true;
}

void WifiConnector::onEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    _inbox.post({WifiEventKind::LinkUp, 0});
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    _inbox.post({WifiEventKind::GotIp, 0});
    break;
  case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    _inbox.post({WifiEventKind::LostIp, 0});
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    _inbox.post({WifiEventKind::LinkDown, info.wifi_sta_disconnected.reason});
    break;
  default:
    break;
  }
}

void WifiConnector::tick(uint32_t) {
  WifiEvent event;
  while (_inbox.take(event))
    apply(event);

  reportDroppedEvents();
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

void WifiConnector::reportDroppedEvents() {
  uint32_t dropped = _inbox.takeDroppedCount();
  if (dropped > 0)
    Serial.printf("[WiFi] Event queue overflow: %u event(s) dropped\n",
                  static_cast<unsigned>(dropped));
}

void WifiConnector::showStateIfChanged() {
  WifiState current = _machine.state();
  if (_hasShownState && current == _shownState)
    return;
  _hasShownState = true;
  _shownState = current;
  showWifiState(_led, current);
}
