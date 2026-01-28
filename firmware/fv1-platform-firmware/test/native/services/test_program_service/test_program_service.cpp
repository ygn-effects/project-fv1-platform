#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/program_service.h"
#include "ui/inputs.h"

#include "../src/services/program_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_transition_relative() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  Event prg;
  prg.m_domain = EventDomain::kUI;
  prg.m_subject = EventSubject::kProgram;
  prg.m_action = EventAction::kValueChanged;
  Event e;

  // Program + 1
  prg.m_data.delta = 1;
  programService.handleEvent(prg);

  // Check LogicalState
  TEST_ASSERT_EQUAL(1, logicalState.m_currentProgram);

  // First event should be program changed
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);

  // Second event should be program save
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Program + 1
  prg.m_data.delta = 1;
  programService.handleEvent(prg);

  // Check LogicalState
  TEST_ASSERT_EQUAL(2, logicalState.m_currentProgram);

  // First event should be program changed
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);

  // Second event should be program save
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Program - 1
  prg.m_data.delta = -1;
  programService.handleEvent(prg);

  // Check LogicalState
  TEST_ASSERT_EQUAL(1, logicalState.m_currentProgram);

  // First event should be program changed
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);

  // Second event should be program save
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Program - 1
  prg.m_data.delta = -1;
  programService.handleEvent(prg);

  // Check LogicalState
  TEST_ASSERT_EQUAL(0, logicalState.m_currentProgram);

  // First event should be program changed
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);

  // Second event should be program save
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);

  // Event bus should be empty
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_wrap_around() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  Event prg;
  prg.m_domain = EventDomain::kUI;
  prg.m_subject = EventSubject::kProgram;
  prg.m_action = EventAction::kValueChanged;

  // Program - 1
  prg.m_data.delta = -1;
  programService.handleEvent(prg);

  // Check LogicalState
  TEST_ASSERT_EQUAL(7, logicalState.m_currentProgram);

  // Program + 1
  prg.m_data.delta = 1;
  programService.handleEvent(prg);

  // Check LogicalState
  TEST_ASSERT_EQUAL(0, logicalState.m_currentProgram);
}

void test_out_of_bounds() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  Event prg;
  prg.m_domain = EventDomain::kUI;
  prg.m_subject = EventSubject::kProgram;
  prg.m_action = EventAction::kValueChanged;

  // Program + 1
  prg.m_data.delta = 5;
  programService.handleEvent(prg);

  // Check LogicalState
  TEST_ASSERT_EQUAL(5, logicalState.m_currentProgram);

  // Program + 100
  prg.m_data.delta = 100;
  programService.handleEvent(prg);

  // Check LogicalState
  TEST_ASSERT_EQUAL(5, logicalState.m_currentProgram);

  // Program - 100
  prg.m_data.delta = -100;
  programService.handleEvent(prg);

  // Check LogicalState
  TEST_ASSERT_EQUAL(5, logicalState.m_currentProgram);
}

void test_init_program() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.init();
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[0], logicalState.m_activeProgram);
}

void test_pointer_transition() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  Event prg;
  prg.m_domain = EventDomain::kUI;
  prg.m_subject = EventSubject::kProgram;
  prg.m_action = EventAction::kValueChanged;

  // Set the pointer
  programService.init();

  // Program + 1
  prg.m_data.delta = +1;
  programService.handleEvent(prg);

  // Check Logical State
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[1], logicalState.m_activeProgram);

  // Program + 1
  prg.m_data.delta = +2;
  programService.handleEvent(prg);

  // Check LogicalState
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[3], logicalState.m_activeProgram);
}

void test_preset_event_no_save() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  // Manually set the program to 3
  logicalState.m_currentProgram = 3;
  programService.init();

  Event presetEvent;
  presetEvent.m_domain = EventDomain::kUI;
  presetEvent.m_subject = EventSubject::kPreset;
  presetEvent.m_action = EventAction::kValueChanged;
  presetEvent.m_data.delta = 1;

  programService.handleEvent(presetEvent);

  Event e;

  // Should publish program changed event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);

  // Should NOT publish save event (event bus should be empty)
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Active program pointer should be synced
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[3], logicalState.m_activeProgram);
}

void test_program_mode_event_no_save() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  // Manually set the program to 5
  logicalState.m_currentProgram = 5;
  programService.init();

  Event modeEvent;
  modeEvent.m_domain = EventDomain::kLogic;
  modeEvent.m_subject = EventSubject::kProgramMode;
  modeEvent.m_action = EventAction::kToggled;

  programService.handleEvent(modeEvent);

  Event e;

  // Should publish program changed event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);

  // Should NOT publish save event (event bus should be empty)
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Active program pointer should be synced
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[5], logicalState.m_activeProgram);
}

void test_preset_bank_load_event_no_save() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  // Manually set the program to 2
  logicalState.m_currentProgram = 2;
  programService.init();

  Event bankEvent;
  bankEvent.m_domain = EventDomain::kMemory;
  bankEvent.m_subject = EventSubject::kPresetBank;
  bankEvent.m_action = EventAction::kValueChanged;

  programService.handleEvent(bankEvent);

  Event e;

  // Should publish program changed event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);

  // Should NOT publish save event (event bus should be empty)
  TEST_ASSERT_FALSE(EventBus::hasEvent());

  // Active program pointer should be synced
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[2], logicalState.m_activeProgram);
}

void test_interested_in() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  // Menu program change
  Event e {EventDomain::kUI, EventSubject::kProgram, EventAction::kValueChanged,0 , 0, {}};
  TEST_ASSERT_TRUE(programService.interestedIn(e));

  // Menu preset change
  e = {EventDomain::kUI, EventSubject::kPreset, EventAction::kValueChanged,0 , 0, {}};
  TEST_ASSERT_TRUE(programService.interestedIn(e));

  // Program mode toggle
  e = {EventDomain::kLogic, EventSubject::kProgramMode, EventAction::kToggled,0 , 0, {}};
  TEST_ASSERT_TRUE(programService.interestedIn(e));

  // Preset bank load
  e = {EventDomain::kLogic, EventSubject::kPresetBank, EventAction::kValueChanged,0 , 0, {}};
  TEST_ASSERT_TRUE(programService.interestedIn(e));

  // Unrelated event
  e = {EventDomain::kLogic, EventSubject::kTempo, EventAction::kValueChanged, 0, 0, {}};
  TEST_ASSERT_FALSE(programService.interestedIn(e));

  // Nonsensical event
  e = {EventDomain::kSystem, EventSubject::kTempo, EventAction::kPressed, static_cast<uint8_t>(SwitchId::kTap), 0, {}};
  TEST_ASSERT_FALSE(programService.interestedIn(e));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_transition_relative);
  RUN_TEST(test_wrap_around);
  RUN_TEST(test_out_of_bounds);
  RUN_TEST(test_init_program);
  RUN_TEST(test_pointer_transition);
  RUN_TEST(test_preset_event_no_save);
  RUN_TEST(test_program_mode_event_no_save);
  RUN_TEST(test_preset_bank_load_event_no_save);
  RUN_TEST(test_interested_in);
  UNITY_END();
}
