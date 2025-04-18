#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/bypass_service.h"
#include "services/fsm_service.h"

#include "../src/services/fsm_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_boot_transition() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  Event bootCompletedEvent{EventType::kBootCompleted, 1, {}};
  fsmService.handleEvent(bootCompletedEvent);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event stateChangedProgramIdle;
  EventBus::recall(stateChangedProgramIdle);

  TEST_ASSERT_EQUAL(EventType::kStateChanged, stateChangedProgramIdle.m_type);
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, static_cast<AppState>(stateChangedProgramIdle.m_data.value));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_boot_transition);
  UNITY_END();
}