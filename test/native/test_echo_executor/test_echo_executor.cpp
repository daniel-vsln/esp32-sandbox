#include <ArduinoFake.h>
#include <EchoExecutor.h>
#include <unity.h>

using namespace fakeit;

static String printed;

void setUp() {
  ArduinoFakeReset();
  printed = "";
  When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const String &)))
      .AlwaysDo([](const String &text) {
        printed += text;
        return text.length();
      });
}

void test_repeats_argument_three_times() {
  EchoExecutor echo{Serial};

  echo.execute({"hello"});

  TEST_ASSERT_EQUAL_STRING("hello... hello... hello...", printed.c_str());
}

void test_joins_several_params_with_space() {
  EchoExecutor echo{Serial};

  echo.execute({"hello", "world"});

  TEST_ASSERT_EQUAL_STRING("hello world... hello world... hello world...",
                           printed.c_str());
}

void test_handles_no_params() {
  EchoExecutor echo{Serial};

  echo.execute({});

  TEST_ASSERT_EQUAL_STRING("... ... ...", printed.c_str());
}

void runTests() {
  UNITY_BEGIN();
  RUN_TEST(test_repeats_argument_three_times);
  RUN_TEST(test_joins_several_params_with_space);
  RUN_TEST(test_handles_no_params);
  UNITY_END();
}
