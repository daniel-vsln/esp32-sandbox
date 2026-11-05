#include <CliCommandProcessor.h>
#include <unity.h>

struct RecordingExecutor : ICommandExecutor {
  int calls = 0;
  std::vector<String> lastParams;

  void execute(std::vector<String> params) override {
    calls++;
    lastParams = params;
  }
};

void test_passes_params_to_registered_command() {
  CliCommandProcessor processor;
  RecordingExecutor executor;
  processor.registerCommand("ping", executor);

  TEST_ASSERT_TRUE(processor.process("ping hello world"));

  TEST_ASSERT_EQUAL(1, executor.calls);
  TEST_ASSERT_EQUAL(2, executor.lastParams.size());
  TEST_ASSERT_EQUAL_STRING("hello", executor.lastParams[0].c_str());
  TEST_ASSERT_EQUAL_STRING("world", executor.lastParams[1].c_str());
}

void test_command_without_params_gets_empty_list() {
  CliCommandProcessor processor;
  RecordingExecutor executor;
  processor.registerCommand("ping", executor);

  TEST_ASSERT_TRUE(processor.process("ping"));

  TEST_ASSERT_EQUAL(0, executor.lastParams.size());
}

void test_line_ending_with_crlf_finds_command() {
  CliCommandProcessor processor;
  RecordingExecutor executor;
  processor.registerCommand("ping", executor);

  TEST_ASSERT_TRUE(processor.process("ping\r"));

  TEST_ASSERT_EQUAL(1, executor.calls);
}

void test_unknown_command_is_not_executed() {
  CliCommandProcessor processor;
  RecordingExecutor executor;
  processor.registerCommand("ping", executor);

  TEST_ASSERT_FALSE(processor.process("nope hello"));

  TEST_ASSERT_EQUAL(0, executor.calls);
}

void test_repeated_registration_keeps_first_executor() {
  CliCommandProcessor processor;
  RecordingExecutor first;
  RecordingExecutor second;
  processor.registerCommand("ping", first);
  processor.registerCommand("ping", second);

  processor.process("ping");

  TEST_ASSERT_EQUAL(1, first.calls);
  TEST_ASSERT_EQUAL(0, second.calls);
}

void test_has_command_reflects_registration() {
  CliCommandProcessor processor;
  RecordingExecutor executor;

  TEST_ASSERT_FALSE(processor.hasCommand("ping"));
  processor.registerCommand("ping", executor);
  TEST_ASSERT_TRUE(processor.hasCommand("ping"));
}

void test_unregistered_command_is_not_executed() {
  CliCommandProcessor processor;
  RecordingExecutor executor;
  processor.registerCommand("ping", executor);

  processor.unregisterCommand("ping");

  TEST_ASSERT_FALSE(processor.hasCommand("ping"));
  TEST_ASSERT_FALSE(processor.process("ping"));
  TEST_ASSERT_EQUAL(0, executor.calls);
}

void test_unregistering_unknown_command_is_safe() {
  CliCommandProcessor processor;
  RecordingExecutor executor;
  processor.registerCommand("ping", executor);

  processor.unregisterCommand("nope");

  TEST_ASSERT_TRUE(processor.hasCommand("ping"));
}

void runTests() {
  UNITY_BEGIN();
  RUN_TEST(test_passes_params_to_registered_command);
  RUN_TEST(test_command_without_params_gets_empty_list);
  RUN_TEST(test_line_ending_with_crlf_finds_command);
  RUN_TEST(test_unknown_command_is_not_executed);
  RUN_TEST(test_repeated_registration_keeps_first_executor);
  RUN_TEST(test_has_command_reflects_registration);
  RUN_TEST(test_unregistered_command_is_not_executed);
  RUN_TEST(test_unregistering_unknown_command_is_safe);
  UNITY_END();
}
