#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/tempo_service.h"
#include "mock/mock_clock.h"
#include "mock/mock_led.h"

#include "../src/services/tempo_service.cpp"
#include "../src/logic/tempo_handler.cpp"

// =============================================================================
// Helper functions
// =============================================================================

void clearEventBus() {
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }
}

Event makeTapIntervalChangedEvent(uint16_t t_interval = 500) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTap;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_interval;
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

Event makeTempoInputChangedEvent(uint16_t t_value = 512) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kInputChanged;
  e.m_data.value = t_value;
  return e;
}

Event makeUITempoChangedEvent(int16_t t_delta = 0) {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kValueChanged;
  e.m_data.delta = t_delta;
  return e;
}

void assertTempoSaveEventPublished() {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kTempo, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);
}

void assertTempoChangedEventPublished(uint16_t t_tempo = 0) {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kTempo, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);

  if (t_tempo != 0) {
    TEST_ASSERT_EQUAL (t_tempo, e.m_data.value);
  }
}

void assertEventBusEmpty() {
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

// =============================================================================
// Init Tests
// =============================================================================

void test_init_syncs_handler_from_logical_state() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  // Set specific program and tempo in logicalState
  logicalState.m_currentProgram = 0;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[logicalState.m_currentProgram];
  logicalState.m_tempo = 400;

  tempoService.init();

  // Send a program change event
  tempoService.handleEvent(makeProgramChangedEvent());

  assertTempoSaveEventPublished();
}

void test_init_mock_led_initialized() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  tempoService.init();

  TEST_ASSERT_TRUE(led.initialized);
}

// =============================================================================
// Program Change Tests
// =============================================================================

void test_program_change_to_delay_program_syncs_handler_program_mode() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  // Set specific program and tempo in logicalState
  logicalState.m_currentProgram = 1;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[logicalState.m_currentProgram];
  logicalState.m_tempo = 900;

  tempoService.init();

  // Send a program change event
  tempoService.handleEvent(makeProgramChangedEvent());

  // Save event
  assertTempoSaveEventPublished();

  // Check logicalState
  TEST_ASSERT_EQUAL(800, logicalState.m_tempo);

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_program_change_to_delay_program_syncs_handler_preset_mode() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  // Set specific tempo
  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_tempo = 900;

  tempoService.init();

  // Send a program change event
  tempoService.handleEvent(makeProgramChangedEvent());

  // Check logicalState
  TEST_ASSERT_EQUAL(900, logicalState.m_tempo);

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_program_change_to_not_delay_program_disables_led() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  tempoService.init();

  // Set specific program in logicalState
  logicalState.m_currentProgram = 7;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[logicalState.m_currentProgram];

  // Set a random LED value
  led.m_pinValue = 64;

  // Send a program change event
  tempoService.handleEvent(makeProgramChangedEvent());

  // LED should be disabled
  TEST_ASSERT_EQUAL(0, led.m_pinValue);

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_program_change_clamps_tempo_below_minimum() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  // Set tempo below Program 1's minimum (100ms)
  logicalState.m_currentProgram = 1;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[1];
  logicalState.m_tempo = 50;

  tempoService.init();
  tempoService.handleEvent(makeProgramChangedEvent());

  // Should clamp to minimum
  assertTempoSaveEventPublished();
  TEST_ASSERT_EQUAL(100, logicalState.m_tempo);
}

void test_program_change_in_preset_mode_does_nothing() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  // Set specific program and tempo in logicalState
  logicalState.m_currentProgram = 1;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[logicalState.m_currentProgram];
  logicalState.m_tempo = 900;

  tempoService.init();

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// Current Program Tests
// =============================================================================

void test_current_program_not_delay_does_nothing() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  // Set specific program in logicalState
  logicalState.m_currentProgram = 7;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[logicalState.m_currentProgram];

  // Tap interval change
  tempoService.handleEvent(makeTapIntervalChangedEvent());

  // Event bus should be empty
  assertEventBusEmpty();

  // Tempo input changed
  tempoService.handleEvent(makeTempoInputChangedEvent());

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// Tap interval tests
// =============================================================================

void test_tap_interval_triggers_tempo_event() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  // Set specific program and tempo in logicalState
  logicalState.m_currentProgram = 1;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[logicalState.m_currentProgram];
  logicalState.m_tempo = 512;

  tempoService.init();

  // Tap interval change
  tempoService.handleEvent(makeTapIntervalChangedEvent(400));

  // Check logicalState
  TEST_ASSERT_EQUAL(400, logicalState.m_tempo);

  // Program 1 is 100 to 800ms so event tempo shouldn' change
  assertTempoChangedEventPublished(400);
  // Save event
  assertTempoSaveEventPublished();

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// Tempo Input tests
// =============================================================================

void test_tempo_input_changed_triggers_tempo_event() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  // Set specific program and tempo in logicalState
  logicalState.m_currentProgram = 1;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[logicalState.m_currentProgram];
  logicalState.m_tempo = 400;

  tempoService.init();

  // Tempo input changed (from PotService, normalized 0-1023)
  tempoService.handleEvent(makeTempoInputChangedEvent(512));

  // Check logicalState - Program 1 is 100 to 800ms, 512/1023 maps to ~450ms
  TEST_ASSERT_EQUAL(450, logicalState.m_tempo);

  // Should publish tempo changed event
  assertTempoChangedEventPublished(450);
  // Save event
  assertTempoSaveEventPublished();

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// Menu tempo tests
// =============================================================================

void test_ui_tempo_value_changed_triggers_tempo_event() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  // Set specific program and tempo in logicalState
  logicalState.m_currentProgram = 1;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[logicalState.m_currentProgram];
  logicalState.m_tempo = 400;

  tempoService.init();

  // Menu tempo delta changed
  tempoService.handleEvent(makeUITempoChangedEvent(100));

  // Check logicalState
  TEST_ASSERT_EQUAL(500, logicalState.m_tempo);

  // Program 1 is 100 to 800ms so event tempo should change
  assertTempoChangedEventPublished(500);
  // Save event
  assertTempoSaveEventPublished();

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// Tempo LED tests
// =============================================================================

void test_update_sets_led_when_delay_effect_and_tempo_set() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  // Set up delay effect with tempo
  logicalState.m_currentProgram = 1;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[1];
  logicalState.m_tempo = 500;

  tempoService.init();
  tempoService.update();

  // Verify LED was set
  TEST_ASSERT_GREATER_OR_EQUAL(32, led.m_pinValue);
  TEST_ASSERT_LESS_OR_EQUAL(255, led.m_pinValue);
}

void test_update_does_not_set_led_when_not_delay_effect() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  // Set up non-delay effect
  logicalState.m_currentProgram = 7;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[7];
  logicalState.m_tempo = 500;

  // Set to known value
  led.m_pinValue = 100;

  tempoService.init();
  tempoService.update();

  // LED value should be unchanged
  TEST_ASSERT_EQUAL(100, led.m_pinValue);
}

void test_update_does_not_set_led_when_tempo_is_zero() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  // Set up delay effect but no tempo
  logicalState.m_currentProgram = 1;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[1];
  logicalState.m_tempo = 0;

  led.m_pinValue = 100;

  tempoService.init();
  tempoService.update();

  TEST_ASSERT_EQUAL(100, led.m_pinValue);
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_logic_program_changed() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(tempoService.interestedIn(e));
}

void test_interested_in_tap_value_changed_event() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTap;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(tempoService.interestedIn(e));
}

void test_interested_in_logic_tempo_input_changed() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kInputChanged;

  TEST_ASSERT_TRUE(tempoService.interestedIn(e));
}

void test_interested_in_ui_tempo() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(tempoService.interestedIn(e));
}

void test_not_interested_in_other_events() {
  LogicalState logicalState;
  MackAdjustbleLed led;
  MockedClock clock;
  TempoService tempoService(logicalState, led, clock);

  Event e;

  // Not interested in physical pot events (PotService handles these now)
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(PotId::kPot0);
  TEST_ASSERT_FALSE(tempoService.interestedIn(e));

  // Not interested in other pots either
  e.m_id = static_cast<uint8_t>(PotId::kPot1);
  TEST_ASSERT_FALSE(tempoService.interestedIn(e));

  // Not interested in other ui events
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMenu;
  TEST_ASSERT_FALSE(tempoService.interestedIn(e));

  // Not interested in memory events
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kTap;
  TEST_ASSERT_FALSE(tempoService.interestedIn(e));

  // Not interested in logic tempo kValueChanged events (outputs them)
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kValueChanged;
  TEST_ASSERT_FALSE(tempoService.interestedIn(e));
}

int main() {
  UNITY_BEGIN();

  // Init
  RUN_TEST(test_init_syncs_handler_from_logical_state);
  RUN_TEST(test_init_mock_led_initialized);

  // Program Change
  RUN_TEST(test_program_change_to_delay_program_syncs_handler_program_mode);
  RUN_TEST(test_program_change_to_delay_program_syncs_handler_preset_mode);
  RUN_TEST(test_program_change_to_not_delay_program_disables_led);
  RUN_TEST(test_program_change_clamps_tempo_below_minimum);
  RUN_TEST(test_program_change_in_preset_mode_does_nothing);

  // Current program
  RUN_TEST(test_current_program_not_delay_does_nothing);

  // Tap interval tests
  RUN_TEST(test_tap_interval_triggers_tempo_event);

  // Tempo Input tests (from PotService via kInputChanged)
  RUN_TEST(test_tempo_input_changed_triggers_tempo_event);

  // Menu tempo tests
  RUN_TEST(test_ui_tempo_value_changed_triggers_tempo_event);

  // Tempo LED tests
  RUN_TEST(test_update_sets_led_when_delay_effect_and_tempo_set);
  RUN_TEST(test_update_does_not_set_led_when_not_delay_effect);
  RUN_TEST(test_update_does_not_set_led_when_tempo_is_zero);

  // Tests interestedIn
  RUN_TEST(test_interested_in_logic_program_changed);
  RUN_TEST(test_interested_in_tap_value_changed_event);
  RUN_TEST(test_interested_in_logic_tempo_input_changed);
  RUN_TEST(test_interested_in_ui_tempo);
  RUN_TEST(test_not_interested_in_other_events);

  UNITY_END();
}