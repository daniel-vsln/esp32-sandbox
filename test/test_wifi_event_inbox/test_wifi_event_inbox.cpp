#include <WifiEventInbox.h>
#include <unity.h>

template <size_t N> struct FakeQueue : IWifiEventQueue {
  WifiEvent items[N];
  size_t head = 0;
  size_t count = 0;

  bool trySend(const WifiEvent &event) override {
    if (count == N)
      return false;
    items[(head + count) % N] = event;
    count++;
    return true;
  }

  bool tryReceive(WifiEvent &event) override {
    if (count == 0)
      return false;
    event = items[head];
    head = (head + 1) % N;
    count--;
    return true;
  }
};

static void assertEvent(WifiEventKind kind, uint8_t reason,
                        const WifiEvent &event) {
  TEST_ASSERT_EQUAL(static_cast<int>(kind), static_cast<int>(event.kind));
  TEST_ASSERT_EQUAL(reason, event.reason);
}

void test_empty_inbox_has_nothing_to_take() {
  FakeQueue<4> queue;
  WifiEventInbox inbox{queue};
  WifiEvent event{};

  TEST_ASSERT_FALSE(inbox.take(event));
  TEST_ASSERT_EQUAL(0, inbox.takeDroppedCount());
}

void test_every_posted_event_is_taken_in_order() {
  FakeQueue<4> queue;
  WifiEventInbox inbox{queue};

  inbox.post({WifiEventKind::LinkUp, 0});
  inbox.post({WifiEventKind::GotIp, 0});
  inbox.post({WifiEventKind::LinkDown, WifiReason::BEACON_TIMEOUT});

  WifiEvent event{};
  TEST_ASSERT_TRUE(inbox.take(event));
  assertEvent(WifiEventKind::LinkUp, 0, event);
  TEST_ASSERT_TRUE(inbox.take(event));
  assertEvent(WifiEventKind::GotIp, 0, event);
  TEST_ASSERT_TRUE(inbox.take(event));
  assertEvent(WifiEventKind::LinkDown, WifiReason::BEACON_TIMEOUT, event);
  TEST_ASSERT_FALSE(inbox.take(event));
}

void test_overflow_counts_dropped_events() {
  FakeQueue<2> queue;
  WifiEventInbox inbox{queue};

  inbox.post({WifiEventKind::LinkUp, 0});
  inbox.post({WifiEventKind::GotIp, 0});
  inbox.post({WifiEventKind::LostIp, 0});
  inbox.post({WifiEventKind::LinkDown, WifiReason::BEACON_TIMEOUT});

  TEST_ASSERT_EQUAL(2, inbox.takeDroppedCount());

  WifiEvent event{};
  TEST_ASSERT_TRUE(inbox.take(event));
  assertEvent(WifiEventKind::LinkUp, 0, event);
  TEST_ASSERT_TRUE(inbox.take(event));
  assertEvent(WifiEventKind::GotIp, 0, event);
  TEST_ASSERT_FALSE(inbox.take(event));
}

void test_dropped_count_resets_after_take() {
  FakeQueue<1> queue;
  WifiEventInbox inbox{queue};

  inbox.post({WifiEventKind::LinkUp, 0});
  inbox.post({WifiEventKind::GotIp, 0});

  TEST_ASSERT_EQUAL(1, inbox.takeDroppedCount());
  TEST_ASSERT_EQUAL(0, inbox.takeDroppedCount());
}

void test_inbox_accepts_events_again_after_drain() {
  FakeQueue<1> queue;
  WifiEventInbox inbox{queue};
  WifiEvent event{};

  inbox.post({WifiEventKind::LinkUp, 0});
  inbox.take(event);
  inbox.post({WifiEventKind::GotIp, 0});

  TEST_ASSERT_TRUE(inbox.take(event));
  assertEvent(WifiEventKind::GotIp, 0, event);
  TEST_ASSERT_EQUAL(0, inbox.takeDroppedCount());
}

void runTests() {
  UNITY_BEGIN();
  RUN_TEST(test_empty_inbox_has_nothing_to_take);
  RUN_TEST(test_every_posted_event_is_taken_in_order);
  RUN_TEST(test_overflow_counts_dropped_events);
  RUN_TEST(test_dropped_count_resets_after_take);
  RUN_TEST(test_inbox_accepts_events_again_after_drain);
  UNITY_END();
}
