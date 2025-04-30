#include <unity.h>
#include "core/event_bus.h"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_publish_one_recall_one() {
  Event publishTempoEvent{EventType::kTempoChanged, 500, {}};
  publishTempoEvent.m_data.value = 300;

  EventBus::publish(publishTempoEvent);
  TEST_ASSERT_TRUE(EventBus::hasEvent());

  Event recallTempoEvent;
  EventBus::recall(recallTempoEvent);

  TEST_ASSERT_EQUAL(EventType::kTempoChanged, recallTempoEvent.m_type);
  TEST_ASSERT_EQUAL(300, recallTempoEvent.m_data.value);

  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_clear() {
  Event publishTempoEvent{EventType::kTempoChanged, 500, {}};
  publishTempoEvent.m_data.value = 300;

  EventBus::publish(publishTempoEvent);
  EventBus::clear();

  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_fifo() {
  EventBus::publish({EventType::kTapPressed, 100, {}});
  EventBus::publish({EventType::kTapPressed, 200, {}});

  Event one, two;
  EventBus::recall(one);
  EventBus::recall(two);

  TEST_ASSERT_EQUAL(100, one.m_timestamp);
  TEST_ASSERT_EQUAL(200, two.m_timestamp);
}

void test_empty_recall() {
  Event event {EventType::kTapPressed, 0, {}};

  TEST_ASSERT_FALSE(EventBus::recall(event));
}

void test_event_category() {
  TEST_ASSERT_EQUAL(EventCategory::kRawPhysicalEvent, eventToCategory(EventType::kRawBypassPressed));
  TEST_ASSERT_EQUAL(EventCategory::kMenuEvent, eventToCategory(EventType::kMenuLocked));
  TEST_ASSERT_EQUAL(EventCategory::kPhysicalEvent, eventToCategory(EventType::kTapPressed));
}

void test_event_sub_category() {
  TEST_ASSERT_EQUAL(EventSubCategory::kBypassEvent, eventToCategory(EventType::kRawBypassPressed));
  TEST_ASSERT_EQUAL(EventSubCategory::kMenuLockEvent, eventToCategory(EventType::kMenuLocked));
  TEST_ASSERT_EQUAL(EventSubCategory::kTapEvent, eventToCategory(EventType::kTapPressed));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_publish_one_recall_one);
  RUN_TEST(test_clear);
  RUN_TEST(test_fifo);
  RUN_TEST(test_empty_recall);
  RUN_TEST(test_event_category);
  UNITY_END();
}
