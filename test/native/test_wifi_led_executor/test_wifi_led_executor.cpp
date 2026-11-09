#include <ArduinoFake.h>
#include <WifiLedExecutor.h>
#include <unity.h>

using namespace fakeit;

struct FakeLed : ILed {
  String calls;

  void turnOff() override { calls += "off;"; }
  void turnGreen() override { calls += "green;"; }
  void turnRed() override { calls += "red;"; }
  void turnWhite() override { calls += "white;"; }
  void turnBlinkOn() override { calls += "blink_on;"; }
  void turnBlinkOff() override { calls += "blink_off;"; }
};

static String printed;

void setUp() {
  ArduinoFakeReset();
  printed = "";
  When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char *)))
      .AlwaysDo([](const char *text) {
        printed += text;
        return strlen(text);
      });
  When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const String &)))
      .AlwaysDo([](const String &text) {
        printed += text;
        return text.length();
      });
}

static void assertActionCalls(const char *action, const char *expectedCalls) {
  FakeLed led;
  WifiLedExecutor executor{led, Serial};

  executor.execute({action});

  TEST_ASSERT_EQUAL_STRING(expectedCalls, led.calls.c_str());
  TEST_ASSERT_EQUAL_STRING("", printed.c_str());
}

void test_green_turns_led_green() { assertActionCalls("green", "green;"); }

void test_red_turns_led_red() { assertActionCalls("red", "red;"); }

void test_white_turns_led_white() { assertActionCalls("white", "white;"); }

void test_off_turns_led_off() { assertActionCalls("off", "off;"); }

void test_blink_on_enables_blinking() {
  assertActionCalls("blink_on", "blink_on;");
}

void test_blink_off_disables_blinking() {
  assertActionCalls("blink_off", "blink_off;");
}

void test_no_params_prints_usage() {
  FakeLed led;
  WifiLedExecutor executor{led, Serial};

  executor.execute({});

  TEST_ASSERT_EQUAL_STRING("", led.calls.c_str());
  TEST_ASSERT_EQUAL_STRING("Usage: led <color>", printed.c_str());
}

void test_unknown_color_is_reported() {
  FakeLed led;
  WifiLedExecutor executor{led, Serial};

  executor.execute({"purple"});

  TEST_ASSERT_EQUAL_STRING("", led.calls.c_str());
  TEST_ASSERT_EQUAL_STRING("Unknown color: purple", printed.c_str());
}

void test_action_is_case_sensitive() {
  FakeLed led;
  WifiLedExecutor executor{led, Serial};

  executor.execute({"Green"});

  TEST_ASSERT_EQUAL_STRING("", led.calls.c_str());
  TEST_ASSERT_EQUAL_STRING("Unknown color: Green", printed.c_str());
}

void test_extra_params_are_ignored() {
  FakeLed led;
  WifiLedExecutor executor{led, Serial};

  executor.execute({"red", "extra"});

  TEST_ASSERT_EQUAL_STRING("red;", led.calls.c_str());
  TEST_ASSERT_EQUAL_STRING("", printed.c_str());
}

void runTests() {
  UNITY_BEGIN();
  RUN_TEST(test_green_turns_led_green);
  RUN_TEST(test_red_turns_led_red);
  RUN_TEST(test_white_turns_led_white);
  RUN_TEST(test_off_turns_led_off);
  RUN_TEST(test_blink_on_enables_blinking);
  RUN_TEST(test_blink_off_disables_blinking);
  RUN_TEST(test_no_params_prints_usage);
  RUN_TEST(test_unknown_color_is_reported);
  RUN_TEST(test_action_is_case_sensitive);
  RUN_TEST(test_extra_params_are_ignored);
  UNITY_END();
}
