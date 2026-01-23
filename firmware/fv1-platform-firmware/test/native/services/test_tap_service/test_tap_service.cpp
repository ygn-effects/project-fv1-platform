#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/tap_service.h"

#include "../src/services/tap_service.cpp"
#include "../src/logic/tap_handler.cpp"

// =============================================================================
// Helper functions
// =============================================================================

void assertTapIntervalEventPublished(uint16_t t_expectedInterval) {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kTap, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_EQUAL(t_expectedInterval, e.m_data.value);
}

void assertTapSaveEventPublished() {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kTap, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);
}

void assertEventBusEmpty() {
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void clearEventBus() {
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }
}

Event makeTapPressEvent(uint32_t t_timestamp) {
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kTap);
  e.m_timestamp = t_timestamp;
  return e;
}

Event makeTapLongPressEvent(uint32_t t_timestamp = 0) {
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kLongPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kTap);
  e.m_timestamp = t_timestamp;
  return e;
}

Event makeMidiTapEvent(uint16_t t_value) {
  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(SwitchId::kTap);
  e.m_data.value = t_value;
  return e;
}

Event makeProgramChangedEvent(uint8_t t_programId = 0) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_id = t_programId;
  return e;
}

Event makePot0ValueChangedEvent(uint16_t t_value = 512) {
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(PotId::kPot0);
  e.m_data.value = t_value;
  return e;
}

Event makeUITempoChangedEvent() {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kValueChanged;
  return e;
}

void setUp() {
  clearEventBus();
}

void tearDown() {

}

// =============================================================================
// Init Tests
// =============================================================================

void test_init_syncs_handler_from_logical_state() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  // Set specific state in logicalState
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kEight;
  logicalState.m_interval = 600;
  logicalState.m_divInterval = 300;

  tapService.init();

  // Verify by doing a long press - it should cycle from kEight to kSixteenth
  tapService.handleEvent(makeTapLongPressEvent());

  TEST_ASSERT_EQUAL(DivValue::kSixteenth, logicalState.m_divValue);
  TEST_ASSERT_EQUAL(150, logicalState.m_divInterval); // 600 / 4
}

// =============================================================================
// Tap Press Tests
// =============================================================================

void test_tap_single_press_does_not_enable_tap() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  // Program 0 supports tap
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  tapService.init();

  // Single tap doesn't set interval
  tapService.handleEvent(makeTapPressEvent(1000));

  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(0, logicalState.m_interval);
  assertEventBusEmpty();
}

void test_tap_two_presses_sets_interval_and_publishes_events() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  tapService.init();

  // Two taps 500ms apart
  tapService.handleEvent(makeTapPressEvent(1000));
  tapService.handleEvent(makeTapPressEvent(1500));

  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(500, logicalState.m_interval);

  // Should publish interval event and save event
  assertTapIntervalEventPublished(500);
  assertTapSaveEventPublished();
  assertEventBusEmpty();
}

void test_tap_ignored_when_program_does_not_support_tap() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  // Program 5 (Plate reverb) doesn't support tap
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[5];
  tapService.init();

  tapService.handleEvent(makeTapPressEvent(1000));
  tapService.handleEvent(makeTapPressEvent(1500));

  // Tap should still be disabled
  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(0, logicalState.m_interval);
}

// =============================================================================
// Long Press / Div Value Tests
// =============================================================================

void test_long_press_enables_div_and_updates_state() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_interval = 600;
  tapService.init();

  // Quarter -> Eight
  tapService.handleEvent(makeTapLongPressEvent());

  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, logicalState.m_divValue);
  TEST_ASSERT_EQUAL(300, logicalState.m_divInterval);

  // Should publish interval event with div interval and save event
  assertTapIntervalEventPublished(300);
  assertTapSaveEventPublished();
  assertEventBusEmpty();
}

void test_long_press_ignored_when_tap_disabled() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tapState = TapState::kDisabled;
  tapService.init();

  tapService.handleEvent(makeTapLongPressEvent());

  // Should remain at defaults
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, logicalState.m_divValue);
  assertEventBusEmpty();
}

void test_tap_ignored_when_in_preset_mode() {
  LogicalState logicalState;
  logicalState.m_programMode = ProgramMode::kPreset;
  TapService tapService(logicalState);

  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tapState = TapState::kDisabled;
  tapService.init();

  tapService.handleEvent(makeTapLongPressEvent());

  // Should remain at defaults
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, logicalState.m_divValue);
  assertEventBusEmpty();
}

// =============================================================================
// Pot0 / UI Tempo Disabling Tap
// =============================================================================

void test_pot0_change_disables_tap() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kEight;
  logicalState.m_interval = 500;
  tapService.init();

  tapService.handleEvent(makePot0ValueChangedEvent(512));

  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, logicalState.m_divValue);

  // Should publish save event
  assertTapSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_tempo_change_disables_tap() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kEight;
  tapService.init();

  tapService.handleEvent(makeUITempoChangedEvent());

  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, logicalState.m_divValue);

  // Should publish save event
  assertTapSaveEventPublished();
  assertEventBusEmpty();
}

// =============================================================================
// Program Change Tests
// =============================================================================

void test_program_change_to_non_delay_disables_tap() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  // Start with delay program and tap enabled
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kEight;
  logicalState.m_interval = 500;
  tapService.init();

  // Change to program 5 (Plate reverb - not delay, doesn't support tap)
  logicalState.m_currentProgram = 5;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[5];
  tapService.handleEvent(makeProgramChangedEvent(5));

  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, logicalState.m_divValue);

  // Should publish save event
  assertTapSaveEventPublished();
  assertEventBusEmpty();
}

void test_program_change_to_delay_keeps_tap_state() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  // Start with delay program and tap enabled
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kEight;
  logicalState.m_interval = 500;
  logicalState.m_divInterval = 250;
  tapService.init();

  // Change to program 1 (Analog delay - supports tap)
  logicalState.m_currentProgram = 1;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[1];
  tapService.handleEvent(makeProgramChangedEvent(1));

  // Tap state should be preserved
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, logicalState.m_divValue);

  // No save event published for program change to supporting program
  assertEventBusEmpty();
}

// =============================================================================
// MIDI Tap Tests
// =============================================================================

void test_midi_tap_short_press() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  tapService.init();

  // MIDI short press works like physical press
  Event e1 = makeMidiTapEvent(MidiCCValues::c_tapShortPress);
  e1.m_timestamp = 1000;
  tapService.handleEvent(e1);

  Event e2 = makeMidiTapEvent(MidiCCValues::c_tapShortPress);
  e2.m_timestamp = 1500;
  tapService.handleEvent(e2);

  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(500, logicalState.m_interval);

  // Should publish events
  assertTapIntervalEventPublished(500);
  assertTapSaveEventPublished();
  assertEventBusEmpty();
}

void test_midi_tap_long_press() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_interval = 600;
  tapService.init();

  tapService.handleEvent(makeMidiTapEvent(MidiCCValues::c_tapLongPress));

  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, logicalState.m_divValue);
  TEST_ASSERT_EQUAL(300, logicalState.m_divInterval);

  // Should publish events
  assertTapIntervalEventPublished(300);
  assertTapSaveEventPublished();
  assertEventBusEmpty();
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_physical_tap_switch() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_id = static_cast<uint8_t>(SwitchId::kTap);

  TEST_ASSERT_TRUE(tapService.interestedIn(e));
}

void test_interested_in_physical_pot0() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(PotId::kPot0);

  TEST_ASSERT_TRUE(tapService.interestedIn(e));
}

void test_interested_in_midi_tap_switch() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kSwitch;
  e.m_id = static_cast<uint8_t>(SwitchId::kTap);

  TEST_ASSERT_TRUE(tapService.interestedIn(e));
}

void test_interested_in_logic_program_changed() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(tapService.interestedIn(e));
}

void test_interested_in_ui_tempo() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(tapService.interestedIn(e));
}

void test_not_interested_in_other_events() {
  LogicalState logicalState;
  TapService tapService(logicalState);

  Event e;

  // Not interested in other switches
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_id = static_cast<uint8_t>(SwitchId::kBypass);
  TEST_ASSERT_FALSE(tapService.interestedIn(e));

  // Not interested in other pots
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(PotId::kPot1);
  TEST_ASSERT_FALSE(tapService.interestedIn(e));

  // Not interested in memory events
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kTap;
  TEST_ASSERT_FALSE(tapService.interestedIn(e));

  // Not interested in logic tap events (outputs them)
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTap;
  TEST_ASSERT_FALSE(tapService.interestedIn(e));
}

int main() {
  UNITY_BEGIN();

  // Init
  RUN_TEST(test_init_syncs_handler_from_logical_state);

  // Tap Press
  RUN_TEST(test_tap_single_press_does_not_enable_tap);
  RUN_TEST(test_tap_two_presses_sets_interval_and_publishes_events);
  RUN_TEST(test_tap_ignored_when_program_does_not_support_tap);
  RUN_TEST(test_tap_ignored_when_in_preset_mode);

  // Long Press / Div
  RUN_TEST(test_long_press_enables_div_and_updates_state);
  RUN_TEST(test_long_press_ignored_when_tap_disabled);

  // Pot0 / UI Tempo Disabling Tap
  RUN_TEST(test_pot0_change_disables_tap);
  RUN_TEST(test_ui_tempo_change_disables_tap);

  // Program Change
  RUN_TEST(test_program_change_to_non_delay_disables_tap);
  RUN_TEST(test_program_change_to_delay_keeps_tap_state);

  // MIDI Tap
  RUN_TEST(test_midi_tap_short_press);
  RUN_TEST(test_midi_tap_long_press);

  // interestedIn
  RUN_TEST(test_interested_in_physical_tap_switch);
  RUN_TEST(test_interested_in_physical_pot0);
  RUN_TEST(test_interested_in_midi_tap_switch);
  RUN_TEST(test_interested_in_logic_program_changed);
  RUN_TEST(test_interested_in_ui_tempo);
  RUN_TEST(test_not_interested_in_other_events);

  UNITY_END();
}
