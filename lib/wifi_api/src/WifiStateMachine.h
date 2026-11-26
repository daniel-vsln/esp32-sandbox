#pragma once

#include <stdint.h>

enum class WifiState : uint8_t { Idle, Connecting, Connected, Reconnecting, Failed };

enum class WifiEventKind : uint8_t {
  ConnectRequested,
  DisconnectRequested,
  LinkUp,
  GotIp,
  LostIp,
  LinkDown,
  Tick
};

struct WifiEvent {
  WifiEventKind kind;
  uint8_t reason;
};

enum class WifiAction : uint8_t { None, Begin, Disconnect };

namespace WifiReason {
constexpr uint8_t ASSOC_LEAVE = 8;
constexpr uint8_t FOUR_WAY_HANDSHAKE_TIMEOUT = 15;
constexpr uint8_t BEACON_TIMEOUT = 200;
constexpr uint8_t NO_AP_FOUND = 201;
constexpr uint8_t AUTH_FAIL = 202;
constexpr uint8_t HANDSHAKE_TIMEOUT = 204;
}

class WifiStateMachine {
public:
  static constexpr uint8_t MAX_RETRIES = 5;
  static constexpr uint32_t ATTEMPT_TIMEOUT_MS = 15000;
  static constexpr uint32_t BACKOFF_INITIAL_MS = 1000;
  static constexpr uint32_t BACKOFF_MAX_MS = 8000;

  WifiAction handle(const WifiEvent &event, uint32_t nowMs);

  WifiState state() const;
  uint8_t retriesLeft() const;
  uint8_t lastReason() const;
  bool isWaitingToRetry() const;
  uint32_t retryDelayMs() const;

private:
  WifiState _state = WifiState::Idle;
  uint8_t _retriesLeft = MAX_RETRIES;
  uint8_t _lastReason = 0;
  bool _attemptInProgress = false;
  uint32_t _phaseStartMs = 0;
  uint32_t _retryDelayMs = 0;
  uint32_t _nextBackoffMs = BACKOFF_INITIAL_MS;

  bool isTryingToConnect() const;
  WifiAction startConnecting(WifiState next, uint32_t nowMs);
  WifiAction beginAttempt(uint32_t nowMs);
  WifiAction failAttempt(uint32_t nowMs, WifiAction abort);
  WifiAction onLinkDown(uint8_t reason, uint32_t nowMs);
  WifiAction onTick(uint32_t nowMs);
};

const char *wifiStateName(WifiState state);
const char *describeDisconnectReason(uint8_t reason);
