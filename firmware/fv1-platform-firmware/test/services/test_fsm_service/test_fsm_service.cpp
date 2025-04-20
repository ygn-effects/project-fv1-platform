#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/bypass_service.h"
#include "services/fsm_service.h"
#include "services/bypass_service.h"

#include "../src/services/fsm_service.cpp"
#include "../src/services/bypass_service.cpp"

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

void test_switch_bypass() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);
  BypassService bypassService(logicalState);

  Event bootCompletedEvent{EventType::kBootCompleted, 1, {}};
  fsmService.handleEvent(bootCompletedEvent);
  Event stateChangedProgramIdle;
  EventBus::recall(stateChangedProgramIdle);

  Event rawTapSwitchPressed{EventType::kRawTapPressed, 2, {}};
  fsmService.handleEvent(rawTapSwitchPressed);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event tapSwitchPressed;
  EventBus::recall(tapSwitchPressed);

  TEST_ASSERT_EQUAL(EventType::kTapPressed, tapSwitchPressed.m_type);

  Event bypassOffEvent{EventType::kBypassPressed, 500, {}};
  bypassService.handleEvent(bypassOffEvent);

  Event BypassDisabledEvent;
  EventBus::recall(BypassDisabledEvent);

  fsmService.handleEvent(rawTapSwitchPressed);
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_menu_idle_edit_transition() {
  LogicalState logicalState;
  FsmService fsmService(logicalState);

  fsmService.handleEvent({EventType::kBootCompleted, 0, {}});
  Event stateChanged;
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(stateChanged);

  fsmService.handleEvent({EventType::kRawMenuEncoderPressed, 0, {}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(stateChanged);

  TEST_ASSERT_EQUAL(AppState::kProgramEdit, static_cast<AppState>(stateChanged.m_data.value));

  fsmService.handleEvent({EventType::kRawMenuEncoderLongPressed, 0, {}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(stateChanged);

  TEST_ASSERT_EQUAL(AppState::kProgramIdle, static_cast<AppState>(stateChanged.m_data.value));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_boot_transition);
  RUN_TEST(test_switch_bypass);
  RUN_TEST(test_menu_idle_edit_transition);
  UNITY_END();
}