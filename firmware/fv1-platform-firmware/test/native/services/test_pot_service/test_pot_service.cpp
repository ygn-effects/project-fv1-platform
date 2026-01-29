#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/pot_service.h"

#include "../src/services/pot_service.cpp"
#include "../src/logic/pot_handler.cpp"

// =============================================================================
// Helper functions
// =============================================================================

Event makePhysicalPotValueChangedEvent(PotId t_id, uint16_t t_value = 512) {
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.value = t_value;
  return e;
}

Event makeLogicProgramChangedEvent(uint8_t t_programId) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_id = t_programId;
  return e;
}

Event makeLogicExprValueChangedEvent(PotId t_id, uint16_t t_value = 0) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.value = t_value;
  return e;
}

Event makeUIPotSettingChangedEvent(PotId t_id, PotParam t_setting, int16_t t_delta = 0) {
  uint8_t id = 0;
  Utils::unpack8(static_cast<uint8_t>(t_id), static_cast<uint8_t>(t_setting), id);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kSettingChanged;
  e.m_id = id;
  e.m_data.delta = t_delta;
  return e;
}

Event makeMenuPotValueChangedEvent(PotId t_id, int16_t t_delta) {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.delta = t_delta;
  return e;
}

Event makeMidiPotValueChanged(PotId t_id, uint16_t t_value) {
  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.value = t_value;
  return e;
}

void assertPotValueChangedEventPublished(PotId t_id) {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPot, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_EQUAL(static_cast<uint8_t>(t_id), e.m_id);
}

void assertSavePotEventPublished(PotId t_id) {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPot, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);
  TEST_ASSERT_EQUAL(static_cast<uint8_t>(t_id), e.m_id);
}

void assertTempoInputChangedEventPublished(uint16_t t_expectedValue) {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kTempo, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kInputChanged, e.m_action);
  TEST_ASSERT_EQUAL(t_expectedValue, e.m_data.value);
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
  PotService potService(logicalState);

  // Set to a non delay effect
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[7];

  // Set pot states
  logicalState.m_currentProgram = 0;
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_state = PotState::kActive;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_state = PotState::kDisabled;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_state = PotState::kDisabled;
  logicalState.m_potParams[logicalState.m_currentProgram][3].m_state = PotState::kActive;

  // Set pot values
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_value = 0;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_value = 0;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_value = 0;
  logicalState.m_potParams[logicalState.m_currentProgram][3].m_value = 0;

  // Init
  potService.init();

  // Send physical events
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kPot0, 512));
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kPot1, 128));
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kPot2, 64));
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kMixPot, 768));

  // Check logicalState values
  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
  TEST_ASSERT_EQUAL(0, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);
  TEST_ASSERT_EQUAL(0, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);
  TEST_ASSERT_EQUAL(768, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);
}

// =============================================================================
// Physical pots tests
// =============================================================================

void test_physical_pot_value_changed_changes_logical_state() {
  LogicalState logicalState;
  PotService potService(logicalState);

  // Set to a non delay effect
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[7];

  // Init
  potService.init();

  // Send physical events
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kPot0, 512));
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kPot1, 128));
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kPot2, 64));
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kMixPot, 768));

  // Check logicalState values, all pots are active and 0-1023 by default
  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
  TEST_ASSERT_EQUAL(128, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);
  TEST_ASSERT_EQUAL(64, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);
  TEST_ASSERT_EQUAL(768, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);

  // Assert published events
  assertPotValueChangedEventPublished(PotId::kPot0);
  assertPotValueChangedEventPublished(PotId::kPot1);
  assertPotValueChangedEventPublished(PotId::kPot2);
  assertPotValueChangedEventPublished(PotId::kMixPot);

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_pot0_publishes_tempo_input_when_using_delay_effect() {
  LogicalState logicalState;
  PotService potService(logicalState);

  // Default program is delay effect (program 0)
  // Init
  potService.init();

  // Send physical event for POT0
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kPot0, 512));

  // Should publish tempo input event with raw ADC value
  assertTempoInputChangedEventPublished(512);

  // Event bus should be empty now
  assertEventBusEmpty();

  // m_potParams should NOT be updated for POT0 on delay effects
  TEST_ASSERT_EQUAL(0, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
}

void test_midi_pot0_publishes_tempo_input_when_using_delay_effect() {
  LogicalState logicalState;
  PotService potService(logicalState);

  // Default program is delay effect (program 0)
  // Init
  potService.init();

  // Send MIDI event for POT0 (value 64 = half range)
  potService.handleEvent(makeMidiPotValueChanged(PotId::kPot0, 64));

  // Should publish tempo input event with scaled value (64 * 1023 / 127 ≈ 515)
  assertTempoInputChangedEventPublished(515);

  // Event bus should be empty now
  assertEventBusEmpty();

  // m_potParams should NOT be updated for POT0 on delay effects
  TEST_ASSERT_EQUAL(0, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
}

void test_expr_pot0_publishes_tempo_input_when_using_delay_effect() {
  LogicalState logicalState;
  PotService potService(logicalState);

  // Default program is delay effect (program 0)
  // Init
  potService.init();

  // Send expression event mapped to POT0
  potService.handleEvent(makeLogicExprValueChangedEvent(PotId::kPot0, 768));

  // Should publish tempo input event with the mapped value
  assertTempoInputChangedEventPublished(768);

  // Event bus should be empty now
  assertEventBusEmpty();

  // m_potParams should NOT be updated for POT0 on delay effects
  TEST_ASSERT_EQUAL(0, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
}

void test_disabled_pot_ignores_physical_input() {
  LogicalState logicalState;
  PotService potService(logicalState);

  // Set to a non delay effect
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[7];

  // Disable POT0 for program0
  logicalState.m_potParams[0][0].m_state = PotState::kDisabled;
  logicalState.m_potParams[0][0].m_value = 0;

  // Init
  potService.init();

  // Send physical event
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kPot0, 512));

  // Event bus should be empty
  assertEventBusEmpty();

  // Check logicalState value
  TEST_ASSERT_EQUAL(0, logicalState.m_potParams[0][0].m_value);
}

// =============================================================================
// Program Change Tests
// =============================================================================

void test_program_change_syncs_handler_from_logical_state() {
  LogicalState logicalState;
  PotService potService(logicalState);

  // Set current program
  logicalState.m_currentProgram = 0;

  // Set pot states for program 0
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_state = PotState::kActive;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_state = PotState::kDisabled;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_state = PotState::kActive;
  logicalState.m_potParams[logicalState.m_currentProgram][3].m_state = PotState::kActive;

  // Set pot values for program 0
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_value = 0;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_value = 768;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_value = 1023;
  logicalState.m_potParams[logicalState.m_currentProgram][3].m_value = 0;

  // Init
  potService.init();

  // Set current program
  logicalState.m_currentProgram = 7;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[7];

  // Set pot states for program 1
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_state = PotState::kActive;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_state = PotState::kActive;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_state = PotState::kDisabled;
  logicalState.m_potParams[logicalState.m_currentProgram][3].m_state = PotState::kActive;

  // Init
  potService.init();

  // Send physical events
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kPot0, 512));
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kPot1, 128));
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kPot2, 64));
  potService.handleEvent(makePhysicalPotValueChangedEvent(PotId::kMixPot, 768));

  // Set pot values for program 0
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_value = 512;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_value = 128;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_value = 1023;
  logicalState.m_potParams[logicalState.m_currentProgram][3].m_value = 768;

  // Assert published events, POT2 is disabled so no event
  assertPotValueChangedEventPublished(PotId::kPot0);
  assertPotValueChangedEventPublished(PotId::kPot1);
  assertPotValueChangedEventPublished(PotId::kMixPot);

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// Midi tests
// =============================================================================

void test_midi_pot_value_changed_changes_logical_state() {
  LogicalState logicalState;
  PotService potService(logicalState);

  // Set to a non-delay effect so POT0 is handled as a regular pot
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[7];

  // Set pot values for program 0
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_value = 0;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_value = 768;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_value = 1023;
  logicalState.m_potParams[logicalState.m_currentProgram][3].m_value = 0;

  potService.init();

  potService.handleEvent(makeMidiPotValueChanged(PotId::kPot0, 16));
  potService.handleEvent(makeMidiPotValueChanged(PotId::kPot1, 32));
  potService.handleEvent(makeMidiPotValueChanged(PotId::kPot2, 64));
  potService.handleEvent(makeMidiPotValueChanged(PotId::kMixPot, 127));

  // Check logicalState values, all pots are active and 0-1023 by default
  TEST_ASSERT_NOT_EQUAL(0, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
  TEST_ASSERT_NOT_EQUAL(768, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);
  TEST_ASSERT_NOT_EQUAL(1023, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);
  TEST_ASSERT_NOT_EQUAL(0, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);

  // Assert published events
  assertPotValueChangedEventPublished(PotId::kPot0);
  assertPotValueChangedEventPublished(PotId::kPot1);
  assertPotValueChangedEventPublished(PotId::kPot2);
  assertPotValueChangedEventPublished(PotId::kMixPot);

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// Expr tests
// =============================================================================

void test_expr_value_changed_changes_logical_state() {
  LogicalState logicalState;
  PotService potService(logicalState);

  // Set to a non-delay effect so POT0 is handled as a regular pot
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[7];

  // Set pot values for program 0
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_value = 0;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_value = 768;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_value = 1023;
  logicalState.m_potParams[logicalState.m_currentProgram][3].m_value = 0;

  potService.init();

  // Send expr events
  potService.handleEvent(makeLogicExprValueChangedEvent(PotId::kPot0, 128));
  potService.handleEvent(makeLogicExprValueChangedEvent(PotId::kPot1, 384));
  potService.handleEvent(makeLogicExprValueChangedEvent(PotId::kPot2, 768));
  potService.handleEvent(makeLogicExprValueChangedEvent(PotId::kMixPot, 1023));

  // Check logicalState values, all pots are active and 0-1023 by default
  TEST_ASSERT_EQUAL(128, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
  TEST_ASSERT_EQUAL(384, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);
  TEST_ASSERT_EQUAL(768, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);
  TEST_ASSERT_EQUAL(1023, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);

  // Assert published events
  assertPotValueChangedEventPublished(PotId::kPot0);
  assertPotValueChangedEventPublished(PotId::kPot1);
  assertPotValueChangedEventPublished(PotId::kPot2);
  assertPotValueChangedEventPublished(PotId::kMixPot);

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// Menu tests
// =============================================================================

void test_menu_pot_value_changed_changes_logical_state() {
  LogicalState logicalState;
  PotService potService(logicalState);

  // Set pot values for program 0
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_value = 0;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_value = 768;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_value = 512;
  logicalState.m_potParams[logicalState.m_currentProgram][3].m_value = 128;

  potService.init();

  // Send menu events
  potService.handleEvent(makeMenuPotValueChangedEvent(PotId::kPot0, 10));
  potService.handleEvent(makeMenuPotValueChangedEvent(PotId::kPot1, 10));
  potService.handleEvent(makeMenuPotValueChangedEvent(PotId::kPot2, 10));
  potService.handleEvent(makeMenuPotValueChangedEvent(PotId::kMixPot, 10));

  // Check logicalState values, all pots are active and 0-1023 by default
  TEST_ASSERT_EQUAL(10, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
  TEST_ASSERT_EQUAL(778, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);
  TEST_ASSERT_EQUAL(522, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);
  TEST_ASSERT_EQUAL(138, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);

  // Assert published events
  assertPotValueChangedEventPublished(PotId::kPot0);
  assertPotValueChangedEventPublished(PotId::kPot1);
  assertPotValueChangedEventPublished(PotId::kPot2);
  assertPotValueChangedEventPublished(PotId::kMixPot);

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_ui_toggle_state() {
  LogicalState logicalState;
  PotService potService(logicalState);

  // Set pot states
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_state = PotState::kActive;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_state = PotState::kDisabled;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_state = PotState::kDisabled;
  logicalState.m_potParams[logicalState.m_currentProgram][3].m_state = PotState::kActive;

  potService.init();

  // Send UI toggle events
  potService.handleEvent(makeUIPotSettingChangedEvent(PotId::kPot0, PotParam::kState));
  potService.handleEvent(makeUIPotSettingChangedEvent(PotId::kPot1, PotParam::kState));
  potService.handleEvent(makeUIPotSettingChangedEvent(PotId::kPot2, PotParam::kState));
  potService.handleEvent(makeUIPotSettingChangedEvent(PotId::kMixPot, PotParam::kState));

  // Check logicalState states
  TEST_ASSERT_EQUAL(PotState::kDisabled, logicalState.m_potParams[logicalState.m_currentProgram][0].m_state);
  TEST_ASSERT_EQUAL(PotState::kActive, logicalState.m_potParams[logicalState.m_currentProgram][1].m_state);
  TEST_ASSERT_EQUAL(PotState::kActive, logicalState.m_potParams[logicalState.m_currentProgram][2].m_state);
  TEST_ASSERT_EQUAL(PotState::kDisabled, logicalState.m_potParams[logicalState.m_currentProgram][3].m_state);

  // Assert published event
  assertSavePotEventPublished(PotId::kPot0);
  assertSavePotEventPublished(PotId::kPot1);
  assertSavePotEventPublished(PotId::kPot2);
  assertSavePotEventPublished(PotId::kMixPot);

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_ui_change_min_value() {
  LogicalState logicalState;
  PotService potService(logicalState);

  // Set pot states
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_minValue = 10;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_minValue = 20;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_minValue = 30;
  logicalState.m_potParams[logicalState.m_currentProgram][3].m_minValue = 40;

  potService.init();

  // Send UI toggle events
  potService.handleEvent(makeUIPotSettingChangedEvent(PotId::kPot0, PotParam::kMinValue, 10));
  potService.handleEvent(makeUIPotSettingChangedEvent(PotId::kPot1, PotParam::kMinValue, 10));
  potService.handleEvent(makeUIPotSettingChangedEvent(PotId::kPot2, PotParam::kMinValue, 10));
  potService.handleEvent(makeUIPotSettingChangedEvent(PotId::kMixPot, PotParam::kMinValue, 10));

  // Check logicalState states
  TEST_ASSERT_EQUAL(20, logicalState.m_potParams[logicalState.m_currentProgram][0].m_minValue);
  TEST_ASSERT_EQUAL(30, logicalState.m_potParams[logicalState.m_currentProgram][1].m_minValue);
  TEST_ASSERT_EQUAL(40, logicalState.m_potParams[logicalState.m_currentProgram][2].m_minValue);
  TEST_ASSERT_EQUAL(50, logicalState.m_potParams[logicalState.m_currentProgram][3].m_minValue);

  // Assert published event
  assertSavePotEventPublished(PotId::kPot0);
  assertSavePotEventPublished(PotId::kPot1);
  assertSavePotEventPublished(PotId::kPot2);
  assertSavePotEventPublished(PotId::kMixPot);

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_ui_change_max_value() {
  LogicalState logicalState;
  PotService potService(logicalState);

  // Set pot states
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_maxValue = 600;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_maxValue = 700;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_maxValue = 800;
  logicalState.m_potParams[logicalState.m_currentProgram][3].m_maxValue = 900;

  potService.init();

  // Send UI toggle events
  potService.handleEvent(makeUIPotSettingChangedEvent(PotId::kPot0, PotParam::kMaxValue, 10));
  potService.handleEvent(makeUIPotSettingChangedEvent(PotId::kPot1, PotParam::kMaxValue, 10));
  potService.handleEvent(makeUIPotSettingChangedEvent(PotId::kPot2, PotParam::kMaxValue, 10));
  potService.handleEvent(makeUIPotSettingChangedEvent(PotId::kMixPot, PotParam::kMaxValue, 10));

  // Check logicalState states
  TEST_ASSERT_EQUAL(610, logicalState.m_potParams[logicalState.m_currentProgram][0].m_maxValue);
  TEST_ASSERT_EQUAL(710, logicalState.m_potParams[logicalState.m_currentProgram][1].m_maxValue);
  TEST_ASSERT_EQUAL(810, logicalState.m_potParams[logicalState.m_currentProgram][2].m_maxValue);
  TEST_ASSERT_EQUAL(910, logicalState.m_potParams[logicalState.m_currentProgram][3].m_maxValue);

  // Assert published event
  assertSavePotEventPublished(PotId::kPot0);
  assertSavePotEventPublished(PotId::kPot1);
  assertSavePotEventPublished(PotId::kPot2);
  assertSavePotEventPublished(PotId::kMixPot);

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_physical_pot_value_changed() {
  LogicalState logicalState;
  PotService potService(logicalState);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(potService.interestedIn(e));
}

void test_interested_in_logic_program_changed() {
  LogicalState logicalState;
  PotService potService(logicalState);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(potService.interestedIn(e));
}

void test_interested_in_logic_expr_value_changed() {
  LogicalState logicalState;
  PotService potService(logicalState);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(potService.interestedIn(e));
}

void test_interested_in_ui_pot() {
  LogicalState logicalState;
  PotService potService(logicalState);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPot;

  TEST_ASSERT_TRUE(potService.interestedIn(e));
}

void test_interested_in_midi_pot() {
  LogicalState logicalState;
  PotService potService(logicalState);

  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPot;

  TEST_ASSERT_TRUE(potService.interestedIn(e));
}

void test_not_interested_in_other_events() {
  LogicalState logicalState;
  PotService potService(logicalState);

  Event e;

  // Not interested in physical switch events
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  TEST_ASSERT_FALSE(potService.interestedIn(e));

  // Not interested in logic expr events (outputs them, doesn't consume)
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPot;
  TEST_ASSERT_FALSE(potService.interestedIn(e));

  // Not interested in other MIDI events
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(potService.interestedIn(e));

  // Not interested in memory events
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(potService.interestedIn(e));

  // Not interested in logic program with different action
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kSave;
  TEST_ASSERT_FALSE(potService.interestedIn(e));

  // Not interested in nonsensical event
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kPressed;
  TEST_ASSERT_FALSE(potService.interestedIn(e));
}

int main() {
  UNITY_BEGIN();

  // Init
  RUN_TEST(test_init_syncs_handler_from_logical_state);

  // Physical pots tests
  RUN_TEST(test_physical_pot_value_changed_changes_logical_state);
  RUN_TEST(test_pot0_publishes_tempo_input_when_using_delay_effect);
  RUN_TEST(test_midi_pot0_publishes_tempo_input_when_using_delay_effect);
  RUN_TEST(test_expr_pot0_publishes_tempo_input_when_using_delay_effect);
  RUN_TEST(test_disabled_pot_ignores_physical_input);

  // Program Change Tests
  RUN_TEST(test_program_change_syncs_handler_from_logical_state);

  // Midi tests
  RUN_TEST(test_midi_pot_value_changed_changes_logical_state);

  // Expr test
  RUN_TEST(test_expr_value_changed_changes_logical_state);

  // Menu tests
  RUN_TEST(test_menu_pot_value_changed_changes_logical_state);
  RUN_TEST(test_ui_toggle_state);
  RUN_TEST(test_ui_change_min_value);
  RUN_TEST(test_ui_change_max_value);

  // interestedIn Tests
  RUN_TEST(test_interested_in_physical_pot_value_changed);
  RUN_TEST(test_interested_in_logic_program_changed);
  RUN_TEST(test_interested_in_logic_expr_value_changed);
  RUN_TEST(test_interested_in_ui_pot);
  RUN_TEST(test_interested_in_midi_pot);
  RUN_TEST(test_not_interested_in_other_events);

  UNITY_END();
}