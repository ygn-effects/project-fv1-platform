#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/program_service.h"
#include "ui/inputs.h"

#include "../src/services/program_service.cpp"

// =============================================================================
// Helper Functions
// =============================================================================

void clearEventBus() {
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }
}

Event makeUIProgramDeltaEvent(int16_t t_delta) {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_data.delta = t_delta;
  return e;
}


Event makeLogicProgramModeToggledEvent() {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kToggled;
  return e;
}

Event makeLogicPresetValueChangedEvent() {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;
  return e;
}

Event makeMidiPresetValueChangedEvent() {
  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;
  return e;
}

Event makeMidiProgramValueChangedEvent(uint8_t t_program) {
  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_program;
  return e;
}

void assertProgramChangedEventPublished() {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected program changed event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
}

void assertProgramSaveEventPublished() {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected program save event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);
}

void assertEventBusEmpty() {
  TEST_ASSERT_FALSE_MESSAGE(EventBus::hasEvent(), "Expected empty event bus but found event");
}

void setUp() {
  clearEventBus();
}

void tearDown() {

}

// =============================================================================
// Init Tests
// =============================================================================

void test_init_sets_active_program_pointer() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.init();

  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[0], logicalState.m_activeProgram);
}

void test_init_with_non_zero_program() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  logicalState.m_currentProgram = 3;
  programService.init();

  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[3], logicalState.m_activeProgram);
}

// =============================================================================
// UI Program Change Events
// =============================================================================

void test_ui_program_increment_updates_state_and_publishes_events() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.handleEvent(makeUIProgramDeltaEvent(1));

  TEST_ASSERT_EQUAL(1, logicalState.m_currentProgram);
  assertProgramChangedEventPublished();
  assertProgramSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_program_decrement_updates_state_and_publishes_events() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  logicalState.m_currentProgram = 2;
  programService.handleEvent(makeUIProgramDeltaEvent(-1));

  TEST_ASSERT_EQUAL(1, logicalState.m_currentProgram);
  assertProgramChangedEventPublished();
  assertProgramSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_program_change_multiple_increments() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.handleEvent(makeUIProgramDeltaEvent(1));
  TEST_ASSERT_EQUAL(1, logicalState.m_currentProgram);
  clearEventBus();

  programService.handleEvent(makeUIProgramDeltaEvent(1));
  TEST_ASSERT_EQUAL(2, logicalState.m_currentProgram);
  clearEventBus();

  programService.handleEvent(makeUIProgramDeltaEvent(-1));
  TEST_ASSERT_EQUAL(1, logicalState.m_currentProgram);
}

void test_ui_program_change_updates_active_program_pointer() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.init();

  programService.handleEvent(makeUIProgramDeltaEvent(1));
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[1], logicalState.m_activeProgram);
  clearEventBus();

  programService.handleEvent(makeUIProgramDeltaEvent(2));
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[3], logicalState.m_activeProgram);
}

// =============================================================================
// Boundary / Wrap Around Tests
// =============================================================================

void test_ui_program_decrement_wraps_from_zero_to_max() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.handleEvent(makeUIProgramDeltaEvent(-1));

  TEST_ASSERT_EQUAL(7, logicalState.m_currentProgram);
}

void test_ui_program_increment_wraps_from_max_to_zero() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  logicalState.m_currentProgram = 7;
  programService.handleEvent(makeUIProgramDeltaEvent(1));

  TEST_ASSERT_EQUAL(0, logicalState.m_currentProgram);
}

void test_ui_program_large_positive_delta_ignored() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  logicalState.m_currentProgram = 5;
  programService.handleEvent(makeUIProgramDeltaEvent(100));

  TEST_ASSERT_EQUAL(5, logicalState.m_currentProgram);
}

void test_ui_program_large_negative_delta_ignored() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  logicalState.m_currentProgram = 5;
  programService.handleEvent(makeUIProgramDeltaEvent(-100));

  TEST_ASSERT_EQUAL(5, logicalState.m_currentProgram);
}

// =============================================================================
// UI Preset Change Events (No Save)
// =============================================================================

void test_logic_preset_change_syncs_pointer_without_save() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  logicalState.m_currentProgram = 3;
  programService.init();

  programService.handleEvent(makeLogicPresetValueChangedEvent());

  assertProgramChangedEventPublished();
  assertEventBusEmpty();  // No save event
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[3], logicalState.m_activeProgram);
}

// =============================================================================
// MIDI Program Change Events
// =============================================================================

void test_midi_program_change_state_and_publishes_events() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.init();

  programService.handleEvent(makeMidiProgramValueChangedEvent(3));

  // Check event bus
  assertProgramChangedEventPublished();
  assertProgramSaveEventPublished();
  assertEventBusEmpty();

  // Test logical state
  TEST_ASSERT_EQUAL(3, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[3], logicalState.m_activeProgram);
}

void test_midi_program_change_invalid_value_not_change_logical_state() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.init();

  programService.handleEvent(makeMidiProgramValueChangedEvent(10));

  // Check event bus
  assertEventBusEmpty();

  // Test logical state
  TEST_ASSERT_EQUAL(0, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[0], logicalState.m_activeProgram);
}

// =============================================================================
// Program Mode Toggle Events (No Save)
// =============================================================================

void test_program_mode_toggle_syncs_pointer_without_save() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  logicalState.m_currentProgram = 5;
  programService.init();

  programService.handleEvent(makeLogicProgramModeToggledEvent());

  assertProgramChangedEventPublished();
  assertEventBusEmpty();  // No save event
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[5], logicalState.m_activeProgram);
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_ui_program_change() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(programService.interestedIn(e));
}

void test_interested_in_logic_program_mode_toggle() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kToggled;

  TEST_ASSERT_TRUE(programService.interestedIn(e));
}

void test_interested_in_logic_preset_value_changed() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(programService.interestedIn(e));
}

void test_interested_in_midi_program_value_change() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(programService.interestedIn(e));
}

void test_not_interested_in_events_it_publishes() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  Event e;

  // Not interested in kLogic kProgram (service publishes this)
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  TEST_ASSERT_FALSE(programService.interestedIn(e));

  // Not interested in kMemory kProgram (service publishes this)
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kSave;
  TEST_ASSERT_FALSE(programService.interestedIn(e));
}

void test_not_interested_in_unrelated_events() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  Event e;

  // Unrelated logic event
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kValueChanged;
  TEST_ASSERT_FALSE(programService.interestedIn(e));

  // Unrelated system event
  e.m_domain = EventDomain::kSystem;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kPressed;
  TEST_ASSERT_FALSE(programService.interestedIn(e));

  // Physical events (not handled by this service)
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  TEST_ASSERT_FALSE(programService.interestedIn(e));

  // Events the service was previously listening to
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_FALSE(programService.interestedIn(e));

  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_FALSE(programService.interestedIn(e));

  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_FALSE(programService.interestedIn(e));
}

int main() {
  UNITY_BEGIN();

  // Init
  RUN_TEST(test_init_sets_active_program_pointer);
  RUN_TEST(test_init_with_non_zero_program);

  // UI Program Change
  RUN_TEST(test_ui_program_increment_updates_state_and_publishes_events);
  RUN_TEST(test_ui_program_decrement_updates_state_and_publishes_events);
  RUN_TEST(test_ui_program_change_multiple_increments);
  RUN_TEST(test_ui_program_change_updates_active_program_pointer);

  // Boundary / Wrap Around
  RUN_TEST(test_ui_program_decrement_wraps_from_zero_to_max);
  RUN_TEST(test_ui_program_increment_wraps_from_max_to_zero);
  RUN_TEST(test_ui_program_large_positive_delta_ignored);
  RUN_TEST(test_ui_program_large_negative_delta_ignored);

  // Preset Change (No Save)
  RUN_TEST(test_logic_preset_change_syncs_pointer_without_save);

  // MIDI Program Change
  RUN_TEST(test_midi_program_change_state_and_publishes_events);
  RUN_TEST(test_midi_program_change_invalid_value_not_change_logical_state);

  // Program Mode Toggle (No Save)
  RUN_TEST(test_program_mode_toggle_syncs_pointer_without_save);

  // interestedIn
  RUN_TEST(test_interested_in_ui_program_change);
  RUN_TEST(test_interested_in_logic_program_mode_toggle);
  RUN_TEST(test_interested_in_logic_preset_value_changed);
  RUN_TEST(test_interested_in_midi_program_value_change);
  RUN_TEST(test_not_interested_in_events_it_publishes);
  RUN_TEST(test_not_interested_in_unrelated_events);

  UNITY_END();
}
