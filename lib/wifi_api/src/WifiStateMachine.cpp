#include "WifiStateMachine.h"

WifiState WifiStateMachine::state() const { return _state; }

uint8_t WifiStateMachine::retriesLeft() const { return _retriesLeft; }

uint8_t WifiStateMachine::lastReason() const { return _lastReason; }

bool WifiStateMachine::isWaitingToRetry() const {
  return isTryingToConnect() && !_attemptInProgress;
}

uint32_t WifiStateMachine::retryDelayMs() const { return _retryDelayMs; }

bool WifiStateMachine::isTryingToConnect() const {
  return _state == WifiState::Connecting || _state == WifiState::Reconnecting;
}

WifiAction WifiStateMachine::handle(const WifiEvent &event, uint32_t nowMs) {
  switch (event.kind) {
  case WifiEventKind::ConnectRequested:
    _lastReason = 0;
    return startConnecting(WifiState::Connecting, nowMs);

  case WifiEventKind::DisconnectRequested:
    if (_state == WifiState::Idle)
      return WifiAction::None;
    _state = WifiState::Idle;
    _attemptInProgress = false;
    return WifiAction::Disconnect;

  case WifiEventKind::LinkUp:
    if (isTryingToConnect() && _attemptInProgress)
      _phaseStartMs = nowMs;
    return WifiAction::None;

  case WifiEventKind::GotIp:
    if (isTryingToConnect()) {
      _state = WifiState::Connected;
      _retriesLeft = MAX_RETRIES;
      _attemptInProgress = false;
      _nextBackoffMs = BACKOFF_INITIAL_MS;
    }
    return WifiAction::None;

  case WifiEventKind::LostIp:
    if (_state == WifiState::Connected)
      return startConnecting(WifiState::Reconnecting, nowMs);
    return WifiAction::None;

  case WifiEventKind::LinkDown:
    return onLinkDown(event.reason, nowMs);

  case WifiEventKind::Tick:
    return onTick(nowMs);
  }
  return WifiAction::None;
}

WifiAction WifiStateMachine::startConnecting(WifiState next, uint32_t nowMs) {
  _state = next;
  _retriesLeft = MAX_RETRIES;
  _nextBackoffMs = BACKOFF_INITIAL_MS;
  return beginAttempt(nowMs);
}

WifiAction WifiStateMachine::beginAttempt(uint32_t nowMs) {
  _attemptInProgress = true;
  _phaseStartMs = nowMs;
  return WifiAction::Begin;
}

WifiAction WifiStateMachine::failAttempt(uint32_t nowMs, WifiAction abort) {
  _attemptInProgress = false;
  if (_retriesLeft == 0) {
    _state = WifiState::Failed;
    return WifiAction::Disconnect;
  }
  _retriesLeft--;
  _phaseStartMs = nowMs;
  _retryDelayMs = _nextBackoffMs;
  _nextBackoffMs = _nextBackoffMs >= BACKOFF_MAX_MS / 2 ? BACKOFF_MAX_MS
                                                         : _nextBackoffMs * 2;
  return abort;
}

WifiAction WifiStateMachine::onLinkDown(uint8_t reason, uint32_t nowMs) {
  if (reason == WifiReason::ASSOC_LEAVE)
    return WifiAction::None;

  switch (_state) {
  case WifiState::Idle:
  case WifiState::Failed:
    return WifiAction::None;

  case WifiState::Connected:
    _lastReason = reason;
    return startConnecting(WifiState::Reconnecting, nowMs);

  case WifiState::Connecting:
  case WifiState::Reconnecting:
    if (!_attemptInProgress)
      return WifiAction::None;
    _lastReason = reason;
    return failAttempt(nowMs, WifiAction::None);
  }
  return WifiAction::None;
}

WifiAction WifiStateMachine::onTick(uint32_t nowMs) {
  if (!isTryingToConnect())
    return WifiAction::None;

  uint32_t elapsed = nowMs - _phaseStartMs;
  if (_attemptInProgress) {
    if (elapsed >= ATTEMPT_TIMEOUT_MS)
      return failAttempt(nowMs, WifiAction::Disconnect);
    return WifiAction::None;
  }

  if (elapsed >= _retryDelayMs)
    return beginAttempt(nowMs);
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
