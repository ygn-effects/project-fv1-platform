#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/program_mode_service.h"

#include "../src/services/program_mode_service.cpp"

// =============================================================================
// Helper Functions
// =============================================================================

void clearEventBus() {
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }
}

Event makePhysicalProgramModeLongPressEvent() {
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kLongPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kProgramMode);
  return e;
}

Event makeLogicProgramModeToggledEvent() {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kToggled;
  return e;
}

Event makeMidiProgramModeValueChangedEvent(uint8_t t_value) {
  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_value;
  return e;
}

void assertSaveLogicalStateEventPublished() {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected save logical state event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kGeneral, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);
}

void assertLoadLogicalStateEventPublished() {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected load logical state event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kGeneral, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kLoad, e.m_action);
}

void assertProgramModeToggledEventPublished() {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected program mode toggled event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgramMode, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kToggled, e.m_action);
}

void assertSaveProgramModeEventPublished() {
  TEST_ASSERT_TRUE_MESSAGE(EventBus::hasEvent(), "Expected save program mode event but bus was empty");
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgramMode, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);
}

void assertNoMoreEvents() {
  TEST_ASSERT_FALSE_MESSAGE(EventBus::hasEvent(), "Unexpected event on bus");
}

void setUp() {
  clearEventBus();
}

void tearDown() {
}

// =============================================================================
// Init Tests
// =============================================================================

void test_init_does_nothing() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  service.init();

  // init() is empty, state should remain unchanged
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);
  assertNoMoreEvents();
}

// =============================================================================
// Physical Long Press Tests (Program -> Preset)
// =============================================================================

void test_physical_long_press_when_program_mode_publishes_save_then_toggled_then_save_mode() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Start in program mode (default)
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);

  service.handleEvent(makePhysicalProgramModeLongPressEvent());

  // When in kProgram mode, should save logical state first (before toggling)
  assertSaveLogicalStateEventPublished();
  assertProgramModeToggledEventPublished();
  assertSaveProgramModeEventPublished();
  assertNoMoreEvents();

  // State not yet updated - that happens when the toggled event is processed
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);
}

void test_physical_long_press_when_preset_mode_publishes_load_then_toggled_then_save_mode() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Start in preset mode
  logicalState.m_programMode = ProgramMode::kPreset;

  service.handleEvent(makePhysicalProgramModeLongPressEvent());

  // When in kPreset mode, should load logical state first (before toggling)
  assertLoadLogicalStateEventPublished();
  assertProgramModeToggledEventPublished();
  assertSaveProgramModeEventPublished();
  assertNoMoreEvents();

  // State not yet updated - that happens when the toggled event is processed
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);
}

// =============================================================================
// Logic Toggled Event Tests (State Update)
// =============================================================================

void test_logic_toggled_event_toggles_program_to_preset() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Start in program mode (default)
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);

  service.handleEvent(makeLogicProgramModeToggledEvent());

  // State should now be updated
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);
  assertNoMoreEvents();
}

void test_logic_toggled_event_toggles_preset_to_program() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Start in preset mode
  logicalState.m_programMode = ProgramMode::kPreset;

  service.handleEvent(makeLogicProgramModeToggledEvent());

  // State should now be updated
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);
  assertNoMoreEvents();
}

// =============================================================================
// MIDI Value Changed Event
// =============================================================================

void test_midi_program_mode_changed_to_preset_when_program_mode_publishes_save_then_toggled_then_save_mode() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Start in program mode (default)
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);

  service.handleEvent(makeMidiProgramModeValueChangedEvent(MidiCCValues::c_presetMode));

  // When in kProgram mode, should save logical state first (before toggling)
  assertSaveLogicalStateEventPublished();
  assertProgramModeToggledEventPublished();
  assertSaveProgramModeEventPublished();
  assertNoMoreEvents();

  // State not yet updated - that happens when the toggled event is processed
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);
}

void test_midi_program_mode_changed_to_program_when_program_mode_no_events() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Start in program mode (default)
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);

  service.handleEvent(makeMidiProgramModeValueChangedEvent(MidiCCValues::c_programMode));

  // Already in program mode so no events published
  assertNoMoreEvents();
}

void test_midi_program_mode_changed_to_program_when_preset_mode_publishes_save_then_toggled_then_save_mode() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Start in preset mode
  logicalState.m_programMode = ProgramMode::kPreset;

  service.handleEvent(makePhysicalProgramModeLongPressEvent());

  // When in kPreset mode, should load logical state first (before toggling)
  assertLoadLogicalStateEventPublished();
  assertProgramModeToggledEventPublished();
  assertSaveProgramModeEventPublished();
  assertNoMoreEvents();

  // State not yet updated - that happens when the toggled event is processed
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);
}

void test_midi_program_mode_changed_to_preset_when_preset_mode_no_events() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Start in preset mode
  logicalState.m_programMode = ProgramMode::kPreset;

  service.handleEvent(makeMidiProgramModeValueChangedEvent(MidiCCValues::c_presetMode));

  // Already in preset mode so no events published
  assertNoMoreEvents();
}

void test_midi_value_changed_invalid_value_not_changed() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Start in program mode (default)
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);

  service.handleEvent(makeMidiProgramModeValueChangedEvent(64));

  // State should not be updated
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);
  assertNoMoreEvents();
}

// =============================================================================
// Full Toggle Sequence Tests
// =============================================================================

void test_full_toggle_sequence_program_to_preset() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Start in program mode (default)
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);

  // Simulate physical long press
  service.handleEvent(makePhysicalProgramModeLongPressEvent());

  // Drain events and verify
  assertSaveLogicalStateEventPublished();
  assertProgramModeToggledEventPublished();
  assertSaveProgramModeEventPublished();
  assertNoMoreEvents();

  // Now simulate service receiving its own toggled event
  service.handleEvent(makeLogicProgramModeToggledEvent());

  // State should now be preset
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);
}

void test_full_toggle_sequence_preset_to_program() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Start in preset mode
  logicalState.m_programMode = ProgramMode::kPreset;

  // Simulate physical long press
  service.handleEvent(makePhysicalProgramModeLongPressEvent());

  // Drain events and verify
  assertLoadLogicalStateEventPublished();
  assertProgramModeToggledEventPublished();
  assertSaveProgramModeEventPublished();
  assertNoMoreEvents();

  // Now simulate service receiving its own toggled event
  service.handleEvent(makeLogicProgramModeToggledEvent());

  // State should now be program
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);
}

void test_multiple_toggles() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Start in program mode
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);

  // First toggle: program -> preset
  service.handleEvent(makePhysicalProgramModeLongPressEvent());
  clearEventBus();
  service.handleEvent(makeLogicProgramModeToggledEvent());
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);

  // Second toggle: preset -> program
  service.handleEvent(makePhysicalProgramModeLongPressEvent());
  clearEventBus();
  service.handleEvent(makeLogicProgramModeToggledEvent());
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);

  // Third toggle: program -> preset
  service.handleEvent(makePhysicalProgramModeLongPressEvent());
  clearEventBus();
  service.handleEvent(makeLogicProgramModeToggledEvent());
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_physical_program_mode_long_press() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kLongPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kProgramMode);

  TEST_ASSERT_TRUE(service.interestedIn(e));
}

void test_interested_in_logic_program_mode_toggled() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kToggled;

  TEST_ASSERT_TRUE(service.interestedIn(e));
}

void test_interested_in_midi_program_mode_value_changed() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(service.interestedIn(e));
}

void test_not_interested_in_physical_program_mode_short_press() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;  // Not kLongPressed
  e.m_id = static_cast<uint8_t>(SwitchId::kProgramMode);

  TEST_ASSERT_FALSE(service.interestedIn(e));
}

void test_not_interested_in_other_switch_long_press() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kLongPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kBypass);  // Wrong switch

  TEST_ASSERT_FALSE(service.interestedIn(e));
}

void test_not_interested_in_memory_events() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  // Not interested in memory events (outputs them)
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kSave;

  TEST_ASSERT_FALSE(service.interestedIn(e));

  e.m_subject = EventSubject::kGeneral;
  TEST_ASSERT_FALSE(service.interestedIn(e));
}

void test_not_interested_in_other_logic_events() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;  // Wrong subject
  e.m_action = EventAction::kToggled;

  TEST_ASSERT_FALSE(service.interestedIn(e));
}

void test_not_interested_in_unrelated_events() {
  LogicalState logicalState;
  ProgramModeService service(logicalState);

  Event e;

  // Not interested in pot events
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  TEST_ASSERT_FALSE(service.interestedIn(e));

  // Not interested in system events
  e.m_domain = EventDomain::kSystem;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kPressed;
  TEST_ASSERT_FALSE(service.interestedIn(e));

  // Not interested in Memory events
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kSave;
  TEST_ASSERT_FALSE(service.interestedIn(e));
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Init
  RUN_TEST(test_init_does_nothing);

  // Physical Long Press
  RUN_TEST(test_physical_long_press_when_program_mode_publishes_save_then_toggled_then_save_mode);
  RUN_TEST(test_physical_long_press_when_preset_mode_publishes_load_then_toggled_then_save_mode);

  // Logic Toggled Event
  RUN_TEST(test_logic_toggled_event_toggles_program_to_preset);
  RUN_TEST(test_logic_toggled_event_toggles_preset_to_program);

  // Midi Value Changed Event
  RUN_TEST(test_midi_program_mode_changed_to_preset_when_preset_mode_no_events);
  RUN_TEST(test_midi_program_mode_changed_to_preset_when_program_mode_publishes_save_then_toggled_then_save_mode);
  RUN_TEST(test_midi_program_mode_changed_to_program_when_preset_mode_publishes_save_then_toggled_then_save_mode);
  RUN_TEST(test_midi_program_mode_changed_to_program_when_program_mode_no_events);
  RUN_TEST(test_midi_value_changed_invalid_value_not_changed);

  // Full Toggle Sequence
  RUN_TEST(test_full_toggle_sequence_program_to_preset);
  RUN_TEST(test_full_toggle_sequence_preset_to_program);
  RUN_TEST(test_multiple_toggles);

  // interestedIn
  RUN_TEST(test_interested_in_physical_program_mode_long_press);
  RUN_TEST(test_interested_in_logic_program_mode_toggled);
  RUN_TEST(test_interested_in_midi_program_mode_value_changed);
  RUN_TEST(test_not_interested_in_physical_program_mode_short_press);
  RUN_TEST(test_not_interested_in_other_switch_long_press);
  RUN_TEST(test_not_interested_in_memory_events);
  RUN_TEST(test_not_interested_in_other_logic_events);
  RUN_TEST(test_not_interested_in_unrelated_events);

  UNITY_END();
}
