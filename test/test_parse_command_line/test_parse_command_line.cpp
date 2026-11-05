#include <CommandLine.h>
#include <unity.h>

static void assertParsed(const String &line, const String &name,
                         const std::vector<String> &params) {
  auto parsed = parseCommandLine(line);
  TEST_ASSERT_EQUAL_STRING(name.c_str(), parsed.name.c_str());
  TEST_ASSERT_EQUAL(params.size(), parsed.params.size());
  for (size_t i = 0; i < params.size(); i++) {
    TEST_ASSERT_EQUAL_STRING(params[i].c_str(), parsed.params[i].c_str());
  }
}

void test_command_without_params() { assertParsed("ping", "ping", {}); }

void test_command_with_one_param() {
  assertParsed("ping hello", "ping", {"hello"});
}

void test_command_with_several_params() {
  assertParsed("ping hello big world", "ping", {"hello", "big", "world"});
}

void test_empty_line() { assertParsed("", "", {}); }

void test_line_of_spaces() { assertParsed("   ", "", {}); }

void test_repeated_spaces_are_collapsed() {
  assertParsed("ping   hello  world", "ping", {"hello", "world"});
}

void test_surrounding_spaces_are_ignored() {
  assertParsed("  ping hello  ", "ping", {"hello"});
}

void test_trailing_cr_is_removed() {
  assertParsed("ping\r", "ping", {});
  assertParsed("ping hello\r", "ping", {"hello"});
}

void runTests() {
  UNITY_BEGIN();
  RUN_TEST(test_command_without_params);
  RUN_TEST(test_command_with_one_param);
  RUN_TEST(test_command_with_several_params);
  RUN_TEST(test_empty_line);
  RUN_TEST(test_line_of_spaces);
  RUN_TEST(test_repeated_spaces_are_collapsed);
  RUN_TEST(test_surrounding_spaces_are_ignored);
  RUN_TEST(test_trailing_cr_is_removed);
  UNITY_END();
}
