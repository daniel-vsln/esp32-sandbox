#include <WifiStateMachine.h>
#include <unity.h>

static const WifiEvent CONNECT{WifiEventKind::ConnectRequested, 0};
static const WifiEvent DISCONNECT{WifiEventKind::DisconnectRequested, 0};
static const WifiEvent LINK_UP{WifiEventKind::LinkUp, 0};
static const WifiEvent GOT_IP{WifiEventKind::GotIp, 0};
static const WifiEvent LOST_IP{WifiEventKind::LostIp, 0};
static const WifiEvent TICK{WifiEventKind::Tick, 0};

static WifiEvent linkDown(uint8_t reason) {
  return {WifiEventKind::LinkDown, reason};
}

static const WifiEvent WRONG_PASSWORD =
    linkDown(WifiReason::FOUR_WAY_HANDSHAKE_TIMEOUT);
static const WifiEvent SIGNAL_LOST = linkDown(WifiReason::BEACON_TIMEOUT);
static const WifiEvent OWN_LEAVE = linkDown(WifiReason::ASSOC_LEAVE);

static constexpr uint32_t TIMEOUT = WifiStateMachine::ATTEMPT_TIMEOUT_MS;

struct Machine {
  WifiStateMachine m;
  uint32_t now = 0;

  WifiAction at(uint32_t timeMs, const WifiEvent &event) {
    now = timeMs;
    return m.handle(event, now);
  }

  WifiAction after(uint32_t deltaMs, const WifiEvent &event) {
    return at(now + deltaMs, event);
  }

  WifiAction retryAfterBackoff() { return after(m.retryDelayMs(), TICK); }
};

static void assertState(WifiState expected, const Machine &machine) {
  TEST_ASSERT_EQUAL_STRING(wifiStateName(expected),
                           wifiStateName(machine.m.state()));
}

static void assertAction(WifiAction expected, WifiAction actual) {
  TEST_ASSERT_EQUAL(static_cast<int>(expected), static_cast<int>(actual));
}

static Machine connecting() {
  Machine machine;
  machine.at(0, CONNECT);
  return machine;
}

static Machine connected() {
  Machine machine = connecting();
  machine.after(100, LINK_UP);
  machine.after(100, GOT_IP);
  return machine;
}

static Machine reconnecting() {
  Machine machine = connected();
  machine.after(100, SIGNAL_LOST);
  return machine;
}

static Machine failed() {
  Machine machine = connecting();
  for (int i = 0; i < WifiStateMachine::MAX_RETRIES; i++) {
    machine.after(100, WRONG_PASSWORD);
    machine.retryAfterBackoff();
  }
  machine.after(100, WRONG_PASSWORD);
  return machine;
}

void test_starts_idle() {
  Machine machine;
  assertState(WifiState::Idle, machine);
}

void test_idle_connect_begins_connecting_immediately() {
  Machine machine;
  assertAction(WifiAction::Begin, machine.at(0, CONNECT));
  assertState(WifiState::Connecting, machine);
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES, machine.m.retriesLeft());
}

void test_idle_ignores_link_events_and_time() {
  Machine machine;
  assertAction(WifiAction::None, machine.at(0, WRONG_PASSWORD));
  assertAction(WifiAction::None, machine.at(0, GOT_IP));
  assertAction(WifiAction::None, machine.at(0, LOST_IP));
  assertAction(WifiAction::None, machine.at(0, DISCONNECT));
  assertAction(WifiAction::None, machine.at(60000, TICK));
  assertState(WifiState::Idle, machine);
}

void test_link_up_alone_does_not_mean_connected() {
  Machine machine = connecting();
  assertAction(WifiAction::None, machine.after(100, LINK_UP));
  assertState(WifiState::Connecting, machine);
}

void test_got_ip_while_connecting_means_connected() {
  Machine machine = connected();
  assertState(WifiState::Connected, machine);
}

void test_link_down_while_connecting_waits_for_backoff() {
  Machine machine = connecting();

  assertAction(WifiAction::None, machine.at(100, WRONG_PASSWORD));
  assertState(WifiState::Connecting, machine);
  TEST_ASSERT_TRUE(machine.m.isWaitingToRetry());
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES - 1, machine.m.retriesLeft());
  TEST_ASSERT_EQUAL(WifiReason::FOUR_WAY_HANDSHAKE_TIMEOUT,
                    machine.m.lastReason());

  assertAction(WifiAction::None, machine.at(1099, TICK));
  assertAction(WifiAction::Begin, machine.at(1100, TICK));
  TEST_ASSERT_FALSE(machine.m.isWaitingToRetry());
}

void test_backoff_doubles_up_to_limit() {
  Machine machine = connecting();
  const uint32_t expected[] = {1000, 2000, 4000, 8000, 8000};

  for (uint32_t delay : expected) {
    machine.after(100, WRONG_PASSWORD);
    TEST_ASSERT_EQUAL_UINT32(delay, machine.m.retryDelayMs());
    assertAction(WifiAction::None, machine.after(delay - 1, TICK));
    assertAction(WifiAction::Begin, machine.after(1, TICK));
  }
}

void test_backoff_is_measured_across_millis_overflow() {
  Machine machine;
  machine.at(0xFFFFFF00u, CONNECT);
  machine.at(0xFFFFFF00u, WRONG_PASSWORD);

  assertAction(WifiAction::None, machine.at(0x00000000u, TICK));
  assertAction(WifiAction::Begin, machine.at(1000 - 0x100, TICK));
}

void test_connecting_fails_after_retries_exhausted() {
  Machine machine = connecting();
  for (int i = 0; i < WifiStateMachine::MAX_RETRIES; i++) {
    assertAction(WifiAction::None, machine.after(100, WRONG_PASSWORD));
    assertAction(WifiAction::Begin, machine.retryAfterBackoff());
  }

  assertAction(WifiAction::Disconnect, machine.after(100, WRONG_PASSWORD));
  assertState(WifiState::Failed, machine);
}

void test_attempt_times_out_without_progress() {
  Machine machine = connecting();

  assertAction(WifiAction::None, machine.at(TIMEOUT - 1, TICK));
  assertAction(WifiAction::Disconnect, machine.at(TIMEOUT, TICK));
  assertState(WifiState::Connecting, machine);
  TEST_ASSERT_TRUE(machine.m.isWaitingToRetry());
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES - 1, machine.m.retriesLeft());

  assertAction(WifiAction::Begin, machine.retryAfterBackoff());
}

void test_link_up_restarts_attempt_timeout() {
  Machine machine = connecting();
  machine.at(10000, LINK_UP);

  assertAction(WifiAction::None, machine.at(10000 + TIMEOUT - 1, TICK));
  assertAction(WifiAction::Disconnect, machine.at(10000 + TIMEOUT, TICK));
}

void test_timeouts_exhaust_retries_into_failed() {
  Machine machine = connecting();
  for (int i = 0; i < WifiStateMachine::MAX_RETRIES; i++) {
    assertAction(WifiAction::Disconnect, machine.after(TIMEOUT, TICK));
    assertAction(WifiAction::Begin, machine.retryAfterBackoff());
  }

  assertAction(WifiAction::Disconnect, machine.after(TIMEOUT, TICK));
  assertState(WifiState::Failed, machine);
}

void test_link_down_while_waiting_for_backoff_is_ignored() {
  Machine machine = connecting();
  machine.at(100, WRONG_PASSWORD);

  assertAction(WifiAction::None, machine.at(500, SIGNAL_LOST));
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES - 1, machine.m.retriesLeft());
  assertAction(WifiAction::Begin, machine.at(1100, TICK));
}

void test_waiting_for_backoff_does_not_time_out() {
  Machine machine = connecting();
  machine.at(100, WRONG_PASSWORD);

  assertAction(WifiAction::Begin, machine.at(100 + TIMEOUT, TICK));
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES - 1, machine.m.retriesLeft());
}

void test_connected_never_times_out() {
  Machine machine = connected();
  assertAction(WifiAction::None, machine.after(10 * TIMEOUT, TICK));
  assertState(WifiState::Connected, machine);
}

void test_disconnect_while_connecting_goes_idle() {
  Machine machine = connecting();
  assertAction(WifiAction::Disconnect, machine.after(100, DISCONNECT));
  assertState(WifiState::Idle, machine);
}

void test_disconnect_while_waiting_for_backoff_cancels_retry() {
  Machine machine = connecting();
  machine.at(100, WRONG_PASSWORD);

  assertAction(WifiAction::Disconnect, machine.at(200, DISCONNECT));
  assertAction(WifiAction::None, machine.at(5000, TICK));
  assertState(WifiState::Idle, machine);
}

void test_link_down_after_connected_reconnects_immediately() {
  Machine machine = connected();
  assertAction(WifiAction::Begin, machine.after(100, SIGNAL_LOST));
  assertState(WifiState::Reconnecting, machine);
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES, machine.m.retriesLeft());
  TEST_ASSERT_EQUAL(WifiReason::BEACON_TIMEOUT, machine.m.lastReason());
}

void test_lost_ip_after_connected_reconnects() {
  Machine machine = connected();
  assertAction(WifiAction::Begin, machine.after(100, LOST_IP));
  assertState(WifiState::Reconnecting, machine);
}

void test_disconnect_while_connected_goes_idle() {
  Machine machine = connected();
  assertAction(WifiAction::Disconnect, machine.after(100, DISCONNECT));
  assertState(WifiState::Idle, machine);
}

void test_reconnecting_recovers_on_got_ip() {
  Machine machine = reconnecting();
  machine.after(100, SIGNAL_LOST);
  machine.retryAfterBackoff();

  machine.after(100, GOT_IP);

  assertState(WifiState::Connected, machine);
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES, machine.m.retriesLeft());
}

void test_reconnect_after_recovery_starts_with_initial_backoff() {
  Machine machine = connecting();
  machine.after(100, WRONG_PASSWORD);
  machine.retryAfterBackoff();
  machine.after(100, WRONG_PASSWORD);
  machine.retryAfterBackoff();
  machine.after(100, GOT_IP);

  machine.after(100, SIGNAL_LOST);
  machine.after(100, SIGNAL_LOST);

  TEST_ASSERT_EQUAL_UINT32(WifiStateMachine::BACKOFF_INITIAL_MS,
                           machine.m.retryDelayMs());
}

void test_reconnecting_fails_after_retries_exhausted() {
  Machine machine = reconnecting();
  for (int i = 0; i < WifiStateMachine::MAX_RETRIES; i++) {
    assertAction(WifiAction::None, machine.after(100, SIGNAL_LOST));
    assertAction(WifiAction::Begin, machine.retryAfterBackoff());
  }

  assertAction(WifiAction::Disconnect, machine.after(100, SIGNAL_LOST));
  assertState(WifiState::Failed, machine);
}

void test_disconnect_while_reconnecting_goes_idle() {
  Machine machine = reconnecting();
  assertAction(WifiAction::Disconnect, machine.after(100, DISCONNECT));
  assertState(WifiState::Idle, machine);
}

void test_failed_ignores_link_events_and_time() {
  Machine machine = failed();
  assertAction(WifiAction::None, machine.after(100, WRONG_PASSWORD));
  assertAction(WifiAction::None, machine.after(100, GOT_IP));
  assertAction(WifiAction::None, machine.after(100, LOST_IP));
  assertAction(WifiAction::None, machine.after(10 * TIMEOUT, TICK));
  assertState(WifiState::Failed, machine);
}

void test_failed_connect_starts_over() {
  Machine machine = failed();
  assertAction(WifiAction::Begin, machine.after(100, CONNECT));
  assertState(WifiState::Connecting, machine);
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES, machine.m.retriesLeft());
}

void test_failed_disconnect_goes_idle() {
  Machine machine = failed();
  assertAction(WifiAction::Disconnect, machine.after(100, DISCONNECT));
  assertState(WifiState::Idle, machine);
}

void test_connect_while_connected_restarts_with_new_attempt() {
  Machine machine = connected();
  assertAction(WifiAction::Begin, machine.after(100, CONNECT));
  assertState(WifiState::Connecting, machine);
}

void test_own_leave_is_ignored_in_every_state() {
  Machine first = connecting();
  assertAction(WifiAction::None, first.after(100, OWN_LEAVE));
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES, first.m.retriesLeft());

  Machine up = connected();
  assertAction(WifiAction::None, up.after(100, OWN_LEAVE));
  assertState(WifiState::Connected, up);

  Machine again = reconnecting();
  assertAction(WifiAction::None, again.after(100, OWN_LEAVE));
  assertState(WifiState::Reconnecting, again);
}

void test_stale_leave_after_reconnect_request_keeps_retries() {
  Machine machine = connected();
  machine.after(100, DISCONNECT);
  machine.after(100, CONNECT);

  machine.after(100, OWN_LEAVE);

  assertState(WifiState::Connecting, machine);
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES, machine.m.retriesLeft());
}

void test_describes_disconnect_reasons() {
  TEST_ASSERT_EQUAL_STRING(
      "probably wrong password",
      describeDisconnectReason(WifiReason::FOUR_WAY_HANDSHAKE_TIMEOUT));
  TEST_ASSERT_EQUAL_STRING(
      "probably wrong password",
      describeDisconnectReason(WifiReason::HANDSHAKE_TIMEOUT));
  TEST_ASSERT_EQUAL_STRING("authentication failed",
                           describeDisconnectReason(WifiReason::AUTH_FAIL));
  TEST_ASSERT_EQUAL_STRING("network not found",
                           describeDisconnectReason(WifiReason::NO_AP_FOUND));
  TEST_ASSERT_EQUAL_STRING("signal lost",
                           describeDisconnectReason(WifiReason::BEACON_TIMEOUT));
  TEST_ASSERT_EQUAL_STRING("connection lost", describeDisconnectReason(2));
}

void runTests() {
  UNITY_BEGIN();
  RUN_TEST(test_starts_idle);
  RUN_TEST(test_idle_connect_begins_connecting_immediately);
  RUN_TEST(test_idle_ignores_link_events_and_time);
  RUN_TEST(test_link_up_alone_does_not_mean_connected);
  RUN_TEST(test_got_ip_while_connecting_means_connected);
  RUN_TEST(test_link_down_while_connecting_waits_for_backoff);
  RUN_TEST(test_backoff_doubles_up_to_limit);
  RUN_TEST(test_backoff_is_measured_across_millis_overflow);
  RUN_TEST(test_connecting_fails_after_retries_exhausted);
  RUN_TEST(test_attempt_times_out_without_progress);
  RUN_TEST(test_link_up_restarts_attempt_timeout);
  RUN_TEST(test_timeouts_exhaust_retries_into_failed);
  RUN_TEST(test_link_down_while_waiting_for_backoff_is_ignored);
  RUN_TEST(test_waiting_for_backoff_does_not_time_out);
  RUN_TEST(test_connected_never_times_out);
  RUN_TEST(test_disconnect_while_connecting_goes_idle);
  RUN_TEST(test_disconnect_while_waiting_for_backoff_cancels_retry);
  RUN_TEST(test_link_down_after_connected_reconnects_immediately);
  RUN_TEST(test_lost_ip_after_connected_reconnects);
  RUN_TEST(test_disconnect_while_connected_goes_idle);
  RUN_TEST(test_reconnecting_recovers_on_got_ip);
  RUN_TEST(test_reconnect_after_recovery_starts_with_initial_backoff);
  RUN_TEST(test_reconnecting_fails_after_retries_exhausted);
  RUN_TEST(test_disconnect_while_reconnecting_goes_idle);
  RUN_TEST(test_failed_ignores_link_events_and_time);
  RUN_TEST(test_failed_connect_starts_over);
  RUN_TEST(test_failed_disconnect_goes_idle);
  RUN_TEST(test_connect_while_connected_restarts_with_new_attempt);
  RUN_TEST(test_own_leave_is_ignored_in_every_state);
  RUN_TEST(test_stale_leave_after_reconnect_request_keeps_retries);
  RUN_TEST(test_describes_disconnect_reasons);
  UNITY_END();
}
