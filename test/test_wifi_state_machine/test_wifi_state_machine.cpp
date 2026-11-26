#include <WifiStateMachine.h>
#include <unity.h>

static const WifiEvent CONNECT{WifiEventKind::ConnectRequested, 0};
static const WifiEvent DISCONNECT{WifiEventKind::DisconnectRequested, 0};
static const WifiEvent LINK_UP{WifiEventKind::LinkUp, 0};
static const WifiEvent GOT_IP{WifiEventKind::GotIp, 0};
static const WifiEvent LOST_IP{WifiEventKind::LostIp, 0};

static WifiEvent linkDown(uint8_t reason) {
  return {WifiEventKind::LinkDown, reason};
}

static const WifiEvent WRONG_PASSWORD =
    linkDown(WifiReason::FOUR_WAY_HANDSHAKE_TIMEOUT);
static const WifiEvent SIGNAL_LOST = linkDown(WifiReason::BEACON_TIMEOUT);
static const WifiEvent OWN_LEAVE = linkDown(WifiReason::ASSOC_LEAVE);

static void assertState(WifiState expected, const WifiStateMachine &m) {
  TEST_ASSERT_EQUAL_STRING(wifiStateName(expected), wifiStateName(m.state()));
}

static void assertAction(WifiAction expected, WifiAction actual) {
  TEST_ASSERT_EQUAL(static_cast<int>(expected), static_cast<int>(actual));
}

static WifiStateMachine connected() {
  WifiStateMachine m;
  m.handle(CONNECT);
  m.handle(LINK_UP);
  m.handle(GOT_IP);
  return m;
}

static WifiStateMachine failed() {
  WifiStateMachine m;
  m.handle(CONNECT);
  for (int i = 0; i <= WifiStateMachine::MAX_RETRIES; i++)
    m.handle(WRONG_PASSWORD);
  return m;
}

void test_starts_idle() {
  WifiStateMachine m;
  assertState(WifiState::Idle, m);
}

void test_idle_connect_begins_connecting() {
  WifiStateMachine m;
  assertAction(WifiAction::Begin, m.handle(CONNECT));
  assertState(WifiState::Connecting, m);
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES, m.retriesLeft());
}

void test_idle_ignores_link_events() {
  WifiStateMachine m;
  assertAction(WifiAction::None, m.handle(WRONG_PASSWORD));
  assertAction(WifiAction::None, m.handle(GOT_IP));
  assertAction(WifiAction::None, m.handle(LOST_IP));
  assertAction(WifiAction::None, m.handle(DISCONNECT));
  assertState(WifiState::Idle, m);
}

void test_link_up_alone_does_not_mean_connected() {
  WifiStateMachine m;
  m.handle(CONNECT);
  assertAction(WifiAction::None, m.handle(LINK_UP));
  assertState(WifiState::Connecting, m);
}

void test_got_ip_while_connecting_means_connected() {
  WifiStateMachine m = connected();
  assertState(WifiState::Connected, m);
}

void test_link_down_while_connecting_retries() {
  WifiStateMachine m;
  m.handle(CONNECT);
  assertAction(WifiAction::Begin, m.handle(WRONG_PASSWORD));
  assertState(WifiState::Connecting, m);
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES - 1, m.retriesLeft());
  TEST_ASSERT_EQUAL(WifiReason::FOUR_WAY_HANDSHAKE_TIMEOUT, m.lastReason());
}

void test_connecting_fails_after_retries_exhausted() {
  WifiStateMachine m;
  m.handle(CONNECT);
  for (int i = 0; i < WifiStateMachine::MAX_RETRIES; i++)
    assertAction(WifiAction::Begin, m.handle(WRONG_PASSWORD));

  assertAction(WifiAction::Disconnect, m.handle(WRONG_PASSWORD));
  assertState(WifiState::Failed, m);
}

void test_disconnect_while_connecting_goes_idle() {
  WifiStateMachine m;
  m.handle(CONNECT);
  assertAction(WifiAction::Disconnect, m.handle(DISCONNECT));
  assertState(WifiState::Idle, m);
}

void test_link_down_after_connected_reconnects_with_fresh_retries() {
  WifiStateMachine m = connected();
  assertAction(WifiAction::Begin, m.handle(SIGNAL_LOST));
  assertState(WifiState::Reconnecting, m);
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES, m.retriesLeft());
  TEST_ASSERT_EQUAL(WifiReason::BEACON_TIMEOUT, m.lastReason());
}

void test_lost_ip_after_connected_reconnects() {
  WifiStateMachine m = connected();
  assertAction(WifiAction::Begin, m.handle(LOST_IP));
  assertState(WifiState::Reconnecting, m);
}

void test_disconnect_while_connected_goes_idle() {
  WifiStateMachine m = connected();
  assertAction(WifiAction::Disconnect, m.handle(DISCONNECT));
  assertState(WifiState::Idle, m);
}

void test_reconnecting_recovers_on_got_ip() {
  WifiStateMachine m = connected();
  m.handle(SIGNAL_LOST);
  m.handle(SIGNAL_LOST);

  m.handle(GOT_IP);

  assertState(WifiState::Connected, m);
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES, m.retriesLeft());
}

void test_reconnecting_fails_after_retries_exhausted() {
  WifiStateMachine m = connected();
  m.handle(SIGNAL_LOST);
  for (int i = 0; i < WifiStateMachine::MAX_RETRIES; i++)
    assertAction(WifiAction::Begin, m.handle(SIGNAL_LOST));

  assertAction(WifiAction::Disconnect, m.handle(SIGNAL_LOST));
  assertState(WifiState::Failed, m);
}

void test_disconnect_while_reconnecting_goes_idle() {
  WifiStateMachine m = connected();
  m.handle(SIGNAL_LOST);
  assertAction(WifiAction::Disconnect, m.handle(DISCONNECT));
  assertState(WifiState::Idle, m);
}

void test_failed_ignores_link_events() {
  WifiStateMachine m = failed();
  assertAction(WifiAction::None, m.handle(WRONG_PASSWORD));
  assertAction(WifiAction::None, m.handle(GOT_IP));
  assertAction(WifiAction::None, m.handle(LOST_IP));
  assertState(WifiState::Failed, m);
}

void test_failed_connect_starts_over() {
  WifiStateMachine m = failed();
  assertAction(WifiAction::Begin, m.handle(CONNECT));
  assertState(WifiState::Connecting, m);
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES, m.retriesLeft());
}

void test_failed_disconnect_goes_idle() {
  WifiStateMachine m = failed();
  assertAction(WifiAction::Disconnect, m.handle(DISCONNECT));
  assertState(WifiState::Idle, m);
}

void test_connect_while_connected_restarts_with_new_attempt() {
  WifiStateMachine m = connected();
  assertAction(WifiAction::Begin, m.handle(CONNECT));
  assertState(WifiState::Connecting, m);
}

void test_own_leave_is_ignored_in_every_state() {
  WifiStateMachine connecting;
  connecting.handle(CONNECT);
  assertAction(WifiAction::None, connecting.handle(OWN_LEAVE));
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES, connecting.retriesLeft());

  WifiStateMachine up = connected();
  assertAction(WifiAction::None, up.handle(OWN_LEAVE));
  assertState(WifiState::Connected, up);

  WifiStateMachine reconnecting = connected();
  reconnecting.handle(SIGNAL_LOST);
  assertAction(WifiAction::None, reconnecting.handle(OWN_LEAVE));
  assertState(WifiState::Reconnecting, reconnecting);
}

void test_stale_leave_after_reconnect_request_keeps_retries() {
  WifiStateMachine m = connected();
  m.handle(DISCONNECT);
  m.handle(CONNECT);

  m.handle(OWN_LEAVE);

  assertState(WifiState::Connecting, m);
  TEST_ASSERT_EQUAL(WifiStateMachine::MAX_RETRIES, m.retriesLeft());
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
  RUN_TEST(test_idle_connect_begins_connecting);
  RUN_TEST(test_idle_ignores_link_events);
  RUN_TEST(test_link_up_alone_does_not_mean_connected);
  RUN_TEST(test_got_ip_while_connecting_means_connected);
  RUN_TEST(test_link_down_while_connecting_retries);
  RUN_TEST(test_connecting_fails_after_retries_exhausted);
  RUN_TEST(test_disconnect_while_connecting_goes_idle);
  RUN_TEST(test_link_down_after_connected_reconnects_with_fresh_retries);
  RUN_TEST(test_lost_ip_after_connected_reconnects);
  RUN_TEST(test_disconnect_while_connected_goes_idle);
  RUN_TEST(test_reconnecting_recovers_on_got_ip);
  RUN_TEST(test_reconnecting_fails_after_retries_exhausted);
  RUN_TEST(test_disconnect_while_reconnecting_goes_idle);
  RUN_TEST(test_failed_ignores_link_events);
  RUN_TEST(test_failed_connect_starts_over);
  RUN_TEST(test_failed_disconnect_goes_idle);
  RUN_TEST(test_connect_while_connected_restarts_with_new_attempt);
  RUN_TEST(test_own_leave_is_ignored_in_every_state);
  RUN_TEST(test_stale_leave_after_reconnect_request_keeps_retries);
  RUN_TEST(test_describes_disconnect_reasons);
  UNITY_END();
}
