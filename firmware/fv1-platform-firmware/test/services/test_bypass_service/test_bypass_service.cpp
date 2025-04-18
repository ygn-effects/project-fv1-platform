#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/bypass_service.h"

#include "../src/services/bypass_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_bypass() {
  LogicalState logicalState;
  BypassService bypassService(logicalState);

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  Event bypassOffEvent{EventType::kBypassPressed, 500, {}};
  bypassService.handleEvent(bypassOffEvent);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event BypassDisabledEvent;
  EventBus::recall(BypassDisabledEvent);
  TEST_ASSERT_EQUAL(EventType::kBypassDisabled, BypassDisabledEvent.m_type);
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);

  Event bypassOnEvent{EventType::kBypassPressed, 600, {}};
  bypassService.handleEvent(bypassOnEvent);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event BypassEnabledEvent;
  EventBus::recall(BypassEnabledEvent);
  TEST_ASSERT_EQUAL(EventType::kBypassEnabled, BypassEnabledEvent.m_type);
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_bypass);
  UNITY_END();
}
