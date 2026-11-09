#include <WifiLed.h>
#include <stdint.h>
#include <unity.h>

struct FakeRgb : IRgbOutput {
  int writes = 0;
  uint32_t rgb = 0xDEADBEEF;

  void write(uint8_t red, uint8_t green, uint8_t blue) override {
    writes++;
    rgb = (uint32_t(red) << 16) | (uint32_t(green) << 8) | blue;
  }
};

static const uint32_t GREEN = 0x00FF00;
static const uint32_t RED = 0xFF0000;
static const uint32_t WHITE = 0xFFFFFF;
static const uint32_t BLACK = 0x000000;

void test_first_tick_writes_off() {
  FakeRgb rgb;
  WifiLed led{rgb};

  led.tick(0);

  TEST_ASSERT_EQUAL(1, rgb.writes);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
}

void test_nothing_written_before_tick() {
  FakeRgb rgb;
  WifiLed led{rgb};

  led.turnGreen();

  TEST_ASSERT_EQUAL(0, rgb.writes);
}

void test_steady_color_is_written_on_tick() {
  FakeRgb rgb;
  WifiLed led{rgb};

  led.turnGreen();
  led.tick(0);

  TEST_ASSERT_EQUAL_HEX32(GREEN, rgb.rgb);
}

void test_each_color_is_written() {
  FakeRgb rgb;
  WifiLed led{rgb};

  led.turnRed();
  led.tick(0);
  TEST_ASSERT_EQUAL_HEX32(RED, rgb.rgb);

  led.turnWhite();
  led.tick(1);
  TEST_ASSERT_EQUAL_HEX32(WHITE, rgb.rgb);

  led.turnOff();
  led.tick(2);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
}

void test_steady_color_is_not_rewritten() {
  FakeRgb rgb;
  WifiLed led{rgb};
  led.turnGreen();

  led.tick(0);
  led.tick(500);
  led.tick(1000);

  TEST_ASSERT_EQUAL(1, rgb.writes);
}

void test_steady_color_does_not_blink() {
  FakeRgb rgb;
  WifiLed led{rgb};
  led.turnGreen();

  for (uint32_t t = 0; t <= 3000; t += 50) {
    led.tick(t);
    TEST_ASSERT_EQUAL_HEX32(GREEN, rgb.rgb);
  }
}

void test_blink_alternates_every_half_period() {
  FakeRgb rgb;
  WifiLed led{rgb};
  led.turnGreen();
  led.turnBlinkOn();

  led.tick(1000);
  TEST_ASSERT_EQUAL_HEX32(GREEN, rgb.rgb);
  led.tick(1499);
  TEST_ASSERT_EQUAL_HEX32(GREEN, rgb.rgb);
  led.tick(1500);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
  led.tick(1999);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
  led.tick(2000);
  TEST_ASSERT_EQUAL_HEX32(GREEN, rgb.rgb);
  TEST_ASSERT_EQUAL(3, rgb.writes);
}

void test_blink_starts_lit_at_any_time() {
  FakeRgb rgb;
  WifiLed led{rgb};
  led.turnRed();
  led.turnBlinkOn();

  led.tick(750);

  TEST_ASSERT_EQUAL_HEX32(RED, rgb.rgb);
}

void test_repeated_blink_on_keeps_phase() {
  FakeRgb rgb;
  WifiLed led{rgb};
  led.turnGreen();
  led.turnBlinkOn();
  led.tick(0);
  led.tick(500);

  led.turnBlinkOn();
  led.tick(600);

  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
}

void test_blink_off_in_dark_phase_restores_color() {
  FakeRgb rgb;
  WifiLed led{rgb};
  led.turnGreen();
  led.turnBlinkOn();
  led.tick(0);
  led.tick(500);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);

  led.turnBlinkOff();
  led.tick(600);

  TEST_ASSERT_EQUAL_HEX32(GREEN, rgb.rgb);
  led.tick(1000);
  led.tick(1500);
  TEST_ASSERT_EQUAL_HEX32(GREEN, rgb.rgb);
}

void test_blink_on_again_restarts_lit() {
  FakeRgb rgb;
  WifiLed led{rgb};
  led.turnGreen();
  led.turnBlinkOn();
  led.tick(0);
  led.turnBlinkOff();
  led.tick(100);

  led.turnBlinkOn();
  led.tick(1700);

  TEST_ASSERT_EQUAL_HEX32(GREEN, rgb.rgb);
  led.tick(2200);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
}

void test_color_change_while_blinking_keeps_blinking() {
  FakeRgb rgb;
  WifiLed led{rgb};
  led.turnGreen();
  led.turnBlinkOn();
  led.tick(0);

  led.turnRed();
  led.tick(100);
  TEST_ASSERT_EQUAL_HEX32(RED, rgb.rgb);
  led.tick(500);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
  led.tick(1000);
  TEST_ASSERT_EQUAL_HEX32(RED, rgb.rgb);
  TEST_ASSERT_TRUE(led.isBlinking());
}

void test_color_change_in_dark_phase_waits_for_lit_phase() {
  FakeRgb rgb;
  WifiLed led{rgb};
  led.turnGreen();
  led.turnBlinkOn();
  led.tick(0);
  led.tick(500);

  led.turnWhite();
  led.tick(600);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
  led.tick(1000);
  TEST_ASSERT_EQUAL_HEX32(WHITE, rgb.rgb);
}

void test_off_while_blinking_stays_dark_and_keeps_blink_mode() {
  FakeRgb rgb;
  WifiLed led{rgb};
  led.turnGreen();
  led.turnBlinkOn();
  led.tick(0);

  led.turnOff();
  led.tick(100);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
  led.tick(1000);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
  TEST_ASSERT_TRUE(led.isBlinking());

  led.turnGreen();
  led.tick(1100);
  TEST_ASSERT_EQUAL_HEX32(GREEN, rgb.rgb);
  led.tick(1500);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
}

void test_color_is_kept_when_blink_toggles() {
  FakeRgb rgb;
  WifiLed led{rgb};
  led.turnRed();

  led.turnBlinkOn();
  led.turnBlinkOff();

  TEST_ASSERT_EQUAL_HEX32(RED, led.color());
  TEST_ASSERT_FALSE(led.isBlinking());
}

void test_blink_survives_millis_overflow() {
  FakeRgb rgb;
  WifiLed led{rgb};
  led.turnGreen();
  led.turnBlinkOn();
  uint32_t start = UINT32_MAX - 699;
  led.tick(start);
  TEST_ASSERT_EQUAL_HEX32(GREEN, rgb.rgb);

  led.tick(start + 500);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
  led.tick(start + 1000);
  TEST_ASSERT_EQUAL_HEX32(GREEN, rgb.rgb);
  led.tick(start + 1500);
  TEST_ASSERT_EQUAL_HEX32(BLACK, rgb.rgb);
}

void runTests() {
  UNITY_BEGIN();
  RUN_TEST(test_first_tick_writes_off);
  RUN_TEST(test_nothing_written_before_tick);
  RUN_TEST(test_steady_color_is_written_on_tick);
  RUN_TEST(test_each_color_is_written);
  RUN_TEST(test_steady_color_is_not_rewritten);
  RUN_TEST(test_steady_color_does_not_blink);
  RUN_TEST(test_blink_alternates_every_half_period);
  RUN_TEST(test_blink_starts_lit_at_any_time);
  RUN_TEST(test_repeated_blink_on_keeps_phase);
  RUN_TEST(test_blink_off_in_dark_phase_restores_color);
  RUN_TEST(test_blink_on_again_restarts_lit);
  RUN_TEST(test_color_change_while_blinking_keeps_blinking);
  RUN_TEST(test_color_change_in_dark_phase_waits_for_lit_phase);
  RUN_TEST(test_off_while_blinking_stays_dark_and_keeps_blink_mode);
  RUN_TEST(test_color_is_kept_when_blink_toggles);
  RUN_TEST(test_blink_survives_millis_overflow);
  UNITY_END();
}
