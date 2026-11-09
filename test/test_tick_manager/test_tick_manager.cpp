#include <TickManager.h>
#include <unity.h>

class MockExecutor : public ITickable {
public:
  int calls = 0;
  void tick(uint32_t) override { calls++; }
};

void test_tick_handler_called_after_interval() {
  TickManager<5> manager;
  MockExecutor executor;
  manager.every(1000, executor, 0);

  manager.tick(1000);

  TEST_ASSERT_EQUAL(1, executor.calls);
}

void test_tick_handler_not_called_before_interval() {
  TickManager<5> manager;
  MockExecutor executor;
  manager.every(1000, executor, 0);

  manager.tick(500);

  TEST_ASSERT_EQUAL(0, executor.calls);
}

void test_tick_handler_called_multiple_times() {
  TickManager<5> manager;
  MockExecutor executor;
  manager.every(1000, executor, 0);

  manager.tick(1000);
  manager.tick(2000);
  manager.tick(3000);

  TEST_ASSERT_EQUAL(3, executor.calls);
}

void test_tick_handler_called_with_different_intervals() {
  TickManager<5> manager;
  MockExecutor executor1;
  MockExecutor executor2;
  manager.every(1000, executor1, 0);
  manager.every(2000, executor2, 0);

  manager.tick(1000);
  manager.tick(2000);
  manager.tick(3000);
  manager.tick(4000);

  TEST_ASSERT_EQUAL(4, executor1.calls);
  TEST_ASSERT_EQUAL(2, executor2.calls);
}

void runTests() {
  UNITY_BEGIN();
  RUN_TEST(test_tick_handler_called_after_interval);
  RUN_TEST(test_tick_handler_not_called_before_interval);
  RUN_TEST(test_tick_handler_called_multiple_times);
  RUN_TEST(test_tick_handler_called_with_different_intervals);
  UNITY_END();
}
