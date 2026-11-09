#include <ArduinoFake.h>
#include <SerialCommandReader.h>
#include <unity.h>

using namespace fakeit;

struct RecordingProcessor : ICommandProcessor {
  int calls = 0;
  String lastLine;

  bool process(const String &line) override {
    calls++;
    lastLine = line;
    return true;
  }
};

void setUp() {
  ArduinoFakeReset();
  When(Method(ArduinoFake(Serial), available)).AlwaysReturn(1);
  When(Method(ArduinoFake(Serial), readStringUntil))
      .AlwaysDo([](char) { return String("ping hello"); });
}

void test_passes_line_to_processor() {
  RecordingProcessor processor;
  SerialCommandReader reader{Serial, processor};

  reader.poll(101);

  TEST_ASSERT_EQUAL(1, processor.calls);
  TEST_ASSERT_EQUAL_STRING("ping hello", processor.lastLine.c_str());
  Verify(Method(ArduinoFake(Serial), readStringUntil).Using('\n')).Once();
}

void test_empty_buffer_is_not_read() {
  When(Method(ArduinoFake(Serial), available)).AlwaysReturn(0);
  RecordingProcessor processor;
  SerialCommandReader reader{Serial, processor};

  reader.poll(101);

  TEST_ASSERT_EQUAL(0, processor.calls);
  VerifyNoOtherInvocations(Method(ArduinoFake(Serial), readStringUntil));
}

void test_does_not_read_before_period_elapsed() {
  RecordingProcessor processor;
  SerialCommandReader reader{Serial, processor};

  reader.poll(100);
  TEST_ASSERT_EQUAL(0, processor.calls);

  reader.poll(101);
  TEST_ASSERT_EQUAL(1, processor.calls);

  reader.poll(201);
  TEST_ASSERT_EQUAL(1, processor.calls);

  reader.poll(202);
  TEST_ASSERT_EQUAL(2, processor.calls);
}

struct RejectingProcessor : ICommandProcessor {
  bool process(const String &) override { return false; }
};

void test_unknown_command_is_reported() {
  static String printed;
  printed = "";
  When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char *)))
      .AlwaysDo([](const char *text) {
        printed += text;
        return strlen(text);
      });
  When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const String &)))
      .AlwaysDo([](const String &text) {
        printed += text;
        return text.length();
      });
  RejectingProcessor processor;
  SerialCommandReader reader{Serial, processor};

  reader.poll(101);

  TEST_ASSERT_EQUAL_STRING("Unknown command: ping hello", printed.c_str());
}

void test_empty_line_is_ignored() {
  When(Method(ArduinoFake(Serial), readStringUntil))
      .AlwaysDo([](char) { return String("\r"); });
  RecordingProcessor processor;
  SerialCommandReader reader{Serial, processor};

  reader.poll(101);

  TEST_ASSERT_EQUAL(0, processor.calls);
}

void runTests() {
  UNITY_BEGIN();
  RUN_TEST(test_passes_line_to_processor);
  RUN_TEST(test_empty_buffer_is_not_read);
  RUN_TEST(test_does_not_read_before_period_elapsed);
  RUN_TEST(test_unknown_command_is_reported);
  RUN_TEST(test_empty_line_is_ignored);
  UNITY_END();
}
