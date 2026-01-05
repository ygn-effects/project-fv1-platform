#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/program_mode_service.h"

#include "../src/services/program_mode_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_program_mode_toggle() {
  LogicalState logicalState;
  ProgramModeService programModeService(logicalState);

  Event toggle;
  toggle.m_domain = EventDomain::kPhysical;
  toggle.m_subject = EventSubject::kProgramMode;
  toggle.m_action = EventAction::kToggled;
  Event e;

  // Process the toggle
  programModeService.handleEvent(toggle);

  // First event should be the toggle event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgramMode, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kToggled, e.m_action);

  // LogicialState should be updated
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);

  // Second event should be the save event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgramMode, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);

  // Process the toggle
  programModeService.handleEvent(toggle);

  // First event should be the toggle event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgramMode, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kToggled, e.m_action);

  // LogicialState should be updated
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);

  // Second event should be the save event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgramMode, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);
}

void test_interested_in() {
  LogicalState logicalState;
  ProgramModeService programModeService(logicalState);

  // Program mode switch long press
  Event e {EventDomain::kPhysical, EventSubject::kSwitch, EventAction::kLongPressed, static_cast<uint8_t>(SwitchId::kProgramMode), 0, {}};
  TEST_ASSERT_TRUE(programModeService.interestedIn(e));

  // Nonsensical event
  e = {EventDomain::kSystem, EventSubject::kTempo, EventAction::kPressed, static_cast<uint8_t>(SwitchId::kTap), 0, {}};
  TEST_ASSERT_FALSE(programModeService.interestedIn(e));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_program_mode_toggle);
  RUN_TEST(test_interested_in);
  UNITY_END();
}
