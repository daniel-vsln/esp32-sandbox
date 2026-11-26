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

void WifiConnector::tick(uint32_t nowMs) {
  WifiEvent event;
  while (_inbox.take(event))
    apply(event, nowMs);

  reportDroppedEvents();
  apply({WifiEventKind::Tick, 0}, nowMs);
  showStateIfChanged();
}

void WifiConnector::connect(const String &ssid, const String &password) {
  _ssid = ssid;
  _password = password;
  apply({WifiEventKind::ConnectRequested, 0}, millis());
}

void WifiConnector::disconnect() {
  apply({WifiEventKind::DisconnectRequested, 0}, millis());
}

void WifiConnector::apply(const WifiEvent &event, uint32_t nowMs) {
  WifiState before = _machine.state();
  uint8_t retriesBefore = _machine.retriesLeft();
  WifiAction action = _machine.handle(event, nowMs);
  log(before, retriesBefore, event, action);
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

void WifiConnector::log(WifiState before, uint8_t retriesBefore,
                        const WifiEvent &event, WifiAction action) {
  WifiState after = _machine.state();
  bool attemptFailed = (_machine.isWaitingToRetry() &&
                        _machine.retriesLeft() < retriesBefore) ||
                       (after == WifiState::Failed && before != after);

  if (attemptFailed) {
    if (event.kind == WifiEventKind::Tick)
      Serial.printf("[WiFi] Attempt timed out after %u ms\n",
                    static_cast<unsigned>(WifiStateMachine::ATTEMPT_TIMEOUT_MS));
    else
      Serial.printf("[WiFi] Attempt failed: %s (reason %u)\n",
                    describeDisconnectReason(event.reason), event.reason);
    if (after != WifiState::Failed)
      Serial.printf("[WiFi] Retrying in %u ms, retries left %u\n",
                    static_cast<unsigned>(_machine.retryDelayMs()),
                    _machine.retriesLeft());
  }

  if (event.kind == WifiEventKind::LinkDown && before == WifiState::Connected &&
      after == WifiState::Reconnecting)
    Serial.printf("[WiFi] Link lost: %s (reason %u)\n",
                  describeDisconnectReason(event.reason), event.reason);

  if (after != before)
    Serial.printf("[WiFi] %s -> %s\n", wifiStateName(before),
                  wifiStateName(after));
  if (action == WifiAction::Begin)
    Serial.printf("[WiFi] Connecting to %s\n", _ssid.c_str());
  if (after == WifiState::Connected && before != after)
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
