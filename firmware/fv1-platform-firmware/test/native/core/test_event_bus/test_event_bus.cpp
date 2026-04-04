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
  Event published;
  published.m_domain = EventDomain::kLogic;
  published.m_subject = EventSubject::kTempo;
  published.m_action = EventAction::kValueChanged;
  published.m_timestamp = 500;
  published.m_data.value = 300;

  EventBus::publish(published);
  TEST_ASSERT_TRUE(EventBus::hasEvent());

  Event recalled;
  EventBus::recall(recalled);

  TEST_ASSERT_EQUAL(EventDomain::kLogic, recalled.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kTempo, recalled.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, recalled.m_action);
  TEST_ASSERT_EQUAL(300, recalled.m_data.value);

  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_clear() {
  Event published;
  published.m_domain = EventDomain::kLogic;
  published.m_subject = EventSubject::kTempo;
  published.m_action = EventAction::kValueChanged;
  published.m_timestamp = 500;
  published.m_data.value = 300;

  EventBus::publish(published);
  EventBus::clear();

  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_fifo() {
  Event first;
  first.m_domain = EventDomain::kPhysical;
  first.m_subject = EventSubject::kTap;
  first.m_action = EventAction::kPressed;
  first.m_timestamp = 100;

  Event second;
  second.m_domain = EventDomain::kPhysical;
  second.m_subject = EventSubject::kTap;
  second.m_action = EventAction::kPressed;
  second.m_timestamp = 200;

  EventBus::publish(first);
  EventBus::publish(second);

  Event one, two;
  EventBus::recall(one);
  EventBus::recall(two);

  TEST_ASSERT_EQUAL(100, one.m_timestamp);
  TEST_ASSERT_EQUAL(200, two.m_timestamp);
}

void test_empty_recall() {
  Event event;
  event.m_domain = EventDomain::kPhysical;
  event.m_subject = EventSubject::kTap;
  event.m_action = EventAction::kPressed;
  event.m_timestamp = 0;

  TEST_ASSERT_FALSE(EventBus::recall(event));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_publish_one_recall_one);
  RUN_TEST(test_clear);
  RUN_TEST(test_fifo);
  RUN_TEST(test_empty_recall);
  UNITY_END();
}
