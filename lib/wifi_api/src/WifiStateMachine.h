#pragma once

#include <stdint.h>

enum class WifiState : uint8_t { Idle, Connecting, Connected, Reconnecting, Failed };

enum class WifiEventKind : uint8_t {
  ConnectRequested,
  DisconnectRequested,
  LinkUp,
  GotIp,
  LostIp,
  LinkDown
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

  WifiAction handle(const WifiEvent &event);

  WifiState state() const;
  uint8_t retriesLeft() const;
  uint8_t lastReason() const;

private:
  WifiState _state = WifiState::Idle;
  uint8_t _retriesLeft = MAX_RETRIES;
  uint8_t _lastReason = 0;

  WifiAction startAttempt(WifiState next);
  WifiAction onLinkDown(uint8_t reason);
};

const char *wifiStateName(WifiState state);
const char *describeDisconnectReason(uint8_t reason);
