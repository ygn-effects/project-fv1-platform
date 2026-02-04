#include <unity.h>
#include "../interaction_fixture.h"
#include "../src/logic/programs.h"
#include "../src/logic/crossfade_handler.cpp"
#include "../src/logic/expr_handler.cpp"
#include "../src/logic/fv1_handler.cpp"
#include "../src/logic/memory_handler.cpp"
#include "../src/logic/menu_handler.cpp"
#include "../src/logic/midi_handler.cpp"
#include "../src/logic/pot_handler.cpp"
#include "../src/logic/preset_handler.cpp"
#include "../src/logic/tempo_handler.cpp"
#include "../src/logic/tap_handler.cpp"
#include "../src/services/fsm_service.cpp"
#include "../src/services/midi_service.cpp"
#include "../src/services/settings_service.cpp"
#include "../src/services/preset_bank_service.cpp"
#include "../src/services/preset_service.cpp"
#include "../src/services/program_mode_service.cpp"
#include "../src/services/program_service.cpp"
#include "../src/services/bypass_service.cpp"
#include "../src/services/expr_service.cpp"
#include "../src/services/pot_service.cpp"
#include "../src/services/tap_service.cpp"
#include "../src/services/tempo_service.cpp"
#include "../src/services/fv1_service.cpp"
#include "../src/services/crossfade_service.cpp"
#include "../src/services/menu_service.cpp"
#include "../src/services/display_service.cpp"
#include "../src/ui/menu_model.cpp"

// =============================================================================
// Test Helpers
// =============================================================================

Event makeBootEvent() {
  Event e{};
  e.m_domain = EventDomain::kSystem;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kBooted;
  return e;
}

Event makeLogicProgramValueChangedEvent() {
  Event e{};
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  return e;
}

Event makeLogicTapValueChangedEvent(uint16_t t_value) {
  Event e{};
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTap;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_value;
  return e;
}

Event makeUITempoValueChangedEvent(int16_t t_delta) {
  Event e{};
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kValueChanged;
  e.m_data.delta = t_delta;
  return e;
}

Event makeUIProgramValueChanged(int16_t t_delta) {
  Event e{};
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_data.delta = t_delta;
  return e;
}

Event makeDriverPotValueChangedEvent(PotId t_id, uint16_t t_value) {
  Event e{};
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.value = t_value;
  return e;
}

void setUp() {

}

void tearDown() {

}

// =============================================================================
// From Tap
// =============================================================================

void test_logic_tap_value_changed_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send tap vent
  fix.publishAndDispatchAllEvents(makeLogicTapValueChangedEvent(500));
  fix.updateAllServices();

  // Test logical state
  TEST_ASSERT_EQUAL(500, fix.logicalState.m_tempo);
}

// =============================================================================
// From Physical Pot0
// =============================================================================

void test_driver_pot_value_changed_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send pot event
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kPot0, 400));
  fix.updateAllServices();

  // Test logical state
  TEST_ASSERT_EQUAL(403, fix.logicalState.m_tempo);
}

// =============================================================================
// From Menu
// =============================================================================

void test_ui_tempo_value_changed_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tempo = 400;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send ui event
  fix.publishAndDispatchAllEvents(makeUITempoValueChangedEvent(100));
  fix.updateAllServices();

  // Test logical state
  TEST_ASSERT_EQUAL(500, fix.logicalState.m_tempo);
}

// =============================================================================
// Program Change
// =============================================================================

void test_logic_program_value_changed_delay_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tempo = 900;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send logic event
  fix.publishAndDispatchAllEvents(makeUIProgramValueChanged(1));
  fix.updateAllServices();

  // Test logical state
  TEST_ASSERT_EQUAL(800, fix.logicalState.m_tempo);
}

void test_logic_program_value_changed_not_delay_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tempo = 900;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send logic event
  fix.publishAndDispatchAllEvents(makeUIProgramValueChanged(6));
  fix.updateAllServices();

  // Test logical state
  TEST_ASSERT_EQUAL(900, fix.logicalState.m_tempo);
}

void test_logic_program_value_changed_not_delay_turns_off_led() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tempo = 900;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send logic event
  fix.publishAndDispatchAllEvents(makeUIProgramValueChanged(6));
  fix.updateAllServices();

  // Test logical state
  TEST_ASSERT_EQUAL(0, fix.mockTapLed.m_value);
}

// =============================================================================
// Hardware Effects
// =============================================================================

void test_tempo_led_pulses_delay_effect() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tempo = 200;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Set clock
  fix.mockClock.advanceBy(500);
  fix.updateAllServices();

  // Test LED state, should be 32 =< m_value =< 255 if blinking
  TEST_ASSERT_GREATER_OR_EQUAL(32, fix.mockTapLed.m_value);
  TEST_ASSERT_LESS_OR_EQUAL(255, fix.mockTapLed.m_value);
}

void test_tempo_led_off_not_delay_effect() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_currentProgram = 7;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Set clock
  fix.mockClock.advanceBy(500);
  fix.updateAllServices();

  // Test LED state
  TEST_ASSERT_EQUAL(0, fix.mockTapLed.m_value);
}

// =============================================================================
// Persistence
// =============================================================================

void test_tempo_persists() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send tap vent
  fix.publishAndDispatchAllEvents(makeLogicTapValueChangedEvent(500));
  fix.updateAllServices();

  // Reset
  fix.init();

  // Test logical state
  TEST_ASSERT_EQUAL(500, fix.logicalState.m_tempo);
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // From Tap
  RUN_TEST(test_logic_tap_value_changed_sets_logical_state);

  // From Physical Pot0
  RUN_TEST(test_driver_pot_value_changed_sets_logical_state);

  // From Menu
  RUN_TEST(test_ui_tempo_value_changed_sets_logical_state);

  // Program Change
  RUN_TEST(test_logic_program_value_changed_delay_sets_logical_state);
  RUN_TEST(test_logic_program_value_changed_not_delay_sets_logical_state);
  RUN_TEST(test_logic_program_value_changed_not_delay_turns_off_led);

  // Hardware Effects
  RUN_TEST(test_tempo_led_pulses_delay_effect);
  RUN_TEST(test_tempo_led_off_not_delay_effect);

  // Persistence
  RUN_TEST(test_tempo_persists);

  return UNITY_END();
}
