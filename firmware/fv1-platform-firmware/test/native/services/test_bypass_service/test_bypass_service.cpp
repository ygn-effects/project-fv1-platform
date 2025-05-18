#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/bypass_service.h"
#include "mock/mock_bypass.h"

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
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  Event e{EventType::kBypassPressed, 500, {}};
  bypassService.handleEvent(e);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kBypassDisabled, e.m_type);
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);

  e = {EventType::kBypassPressed, 600, {}};
  bypassService.handleEvent(e);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kBypassEnabled, e.m_type);
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
}

void test_interested_in() {
  LogicalState logicalState;
  MockBypass mockBypass;
  BypassService bypassService(logicalState, mockBypass);

  Event e{EventType::kBypassPressed, 500, {}};
  TEST_ASSERT_TRUE(bypassService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kPot0Moved, 500, {.delta=1}};
  TEST_ASSERT_FALSE(bypassService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kMenuExprMappedPotMoved, 500, {.delta=1}};
  TEST_ASSERT_FALSE(bypassService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_bypass);
  RUN_TEST(test_interested_in);
  UNITY_END();
}
