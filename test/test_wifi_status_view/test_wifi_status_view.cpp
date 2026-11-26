#include <WifiStatusView.h>
#include <string.h>
#include <unity.h>

struct FakeLed : ILed {
  const char *color = "unset";
  bool blinking = true;

  void turnOff() override { color = "off"; }
  void turnGreen() override { color = "green"; }
  void turnRed() override { color = "red"; }
  void turnWhite() override { color = "white"; }
  void turnBlinkOn() override { blinking = true; }
  void turnBlinkOff() override { blinking = false; }
};

static void assertShows(WifiState state, const char *color, bool blinking) {
  FakeLed led;
  led.blinking = !blinking;

  showWifiState(led, state);

  TEST_ASSERT_EQUAL_STRING(color, led.color);
  TEST_ASSERT_EQUAL(blinking, led.blinking);
}

void test_idle_is_off() { assertShows(WifiState::Idle, "off", false); }

void test_connecting_blinks_green() {
  assertShows(WifiState::Connecting, "green", true);
}

void test_reconnecting_blinks_green() {
  assertShows(WifiState::Reconnecting, "green", true);
}

void test_connected_is_solid_green() {
  assertShows(WifiState::Connected, "green", false);
}

void test_failed_is_solid_red() { assertShows(WifiState::Failed, "red", false); }

void runTests() {
  UNITY_BEGIN();
  RUN_TEST(test_idle_is_off);
  RUN_TEST(test_connecting_blinks_green);
  RUN_TEST(test_reconnecting_blinks_green);
  RUN_TEST(test_connected_is_solid_green);
  RUN_TEST(test_failed_is_solid_red);
  UNITY_END();
}
