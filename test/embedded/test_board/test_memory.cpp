#include <Arduino.h>
#include <unity.h>

void test_psram_is_available() {
  TEST_ASSERT_TRUE(psramFound());
  TEST_ASSERT_EQUAL_UINT32(8 * 1024 * 1024, ESP.getPsramSize());
}

void test_flash_is_16mb() {
  TEST_ASSERT_EQUAL_UINT32(16 * 1024 * 1024, ESP.getFlashChipSize());
}

void test_psram_is_writable() {
  const size_t size = 1024;
  auto *buffer = static_cast<uint8_t *>(ps_malloc(size));
  TEST_ASSERT_NOT_NULL(buffer);

  memset(buffer, 0xA5, size);
  TEST_ASSERT_EACH_EQUAL_UINT8(0xA5, buffer, size);

  free(buffer);
}

void runTests() {
  UNITY_BEGIN();
  RUN_TEST(test_psram_is_available);
  RUN_TEST(test_flash_is_16mb);
  RUN_TEST(test_psram_is_writable);
  UNITY_END();
}
