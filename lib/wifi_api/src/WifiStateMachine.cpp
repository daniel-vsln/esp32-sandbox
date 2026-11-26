#include "WifiStateMachine.h"

WifiState WifiStateMachine::state() const { return _state; }

uint8_t WifiStateMachine::retriesLeft() const { return _retriesLeft; }

uint8_t WifiStateMachine::lastReason() const { return _lastReason; }

WifiAction WifiStateMachine::handle(const WifiEvent &event) {
  switch (event.kind) {
  case WifiEventKind::ConnectRequested:
    _lastReason = 0;
    return startAttempt(WifiState::Connecting);

  case WifiEventKind::DisconnectRequested:
    if (_state == WifiState::Idle)
      return WifiAction::None;
    _state = WifiState::Idle;
    return WifiAction::Disconnect;

  case WifiEventKind::LinkUp:
    return WifiAction::None;

  case WifiEventKind::GotIp:
    if (_state == WifiState::Connecting || _state == WifiState::Reconnecting) {
      _state = WifiState::Connected;
      _retriesLeft = MAX_RETRIES;
    }
    return WifiAction::None;

  case WifiEventKind::LostIp:
    if (_state == WifiState::Connected)
      return startAttempt(WifiState::Reconnecting);
    return WifiAction::None;

  case WifiEventKind::LinkDown:
    return onLinkDown(event.reason);
  }
  return WifiAction::None;
}

WifiAction WifiStateMachine::startAttempt(WifiState next) {
  _state = next;
  _retriesLeft = MAX_RETRIES;
  return WifiAction::Begin;
}

WifiAction WifiStateMachine::onLinkDown(uint8_t reason) {
  if (reason == WifiReason::ASSOC_LEAVE)
    return WifiAction::None;

  switch (_state) {
  case WifiState::Idle:
  case WifiState::Failed:
    return WifiAction::None;

  case WifiState::Connected:
    _lastReason = reason;
    return startAttempt(WifiState::Reconnecting);

  case WifiState::Connecting:
  case WifiState::Reconnecting:
    _lastReason = reason;
    if (_retriesLeft == 0) {
      _state = WifiState::Failed;
      return WifiAction::Disconnect;
    }
    _retriesLeft--;
    return WifiAction::Begin;
  }
  return WifiAction::None;
}

const char *wifiStateName(WifiState state) {
  switch (state) {
  case WifiState::Idle:
    return "idle";
  case WifiState::Connecting:
    return "connecting";
  case WifiState::Connected:
    return "connected";
  case WifiState::Reconnecting:
    return "reconnecting";
  case WifiState::Failed:
    return "failed";
  }
  return "unknown";
}

const char *describeDisconnectReason(uint8_t reason) {
  switch (reason) {
  case WifiReason::FOUR_WAY_HANDSHAKE_TIMEOUT:
  case WifiReason::HANDSHAKE_TIMEOUT:
    return "probably wrong password";
  case WifiReason::AUTH_FAIL:
    return "authentication failed";
  case WifiReason::NO_AP_FOUND:
    return "network not found";
  case WifiReason::BEACON_TIMEOUT:
    return "signal lost";
  default:
    return "connection lost";
  }
}
