#include <unity.h>
#include "../interaction_fixture.h"
#include "../src/logic/programs.h"
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

Event makeLogicExprValueChangedEvent(PotId t_id, uint16_t t_value = 0) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.value = t_value;
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

Event makeUIPotValueChangedEvent(PotId t_id, int16_t t_delta) {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.delta = t_delta;
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

void makeMidiCCPotValueChangedMessage(MidiHandler* t_handler, PotId t_id, uint8_t t_value) {
  t_handler->pushByte(0xB0);
  t_handler->pushByte(static_cast<uint8_t>(t_id));
  t_handler->pushByte(t_value);
}

void setUp() {

}

void tearDown() {

}

// =============================================================================
// Physical Pot Input
// =============================================================================

void test_driver_pot_value_changed_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_currentProgram = 7;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Sent events
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kPot0, 128));
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kPot1, 256));
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kPot2, 384));
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kMixPot, 512));

  // Test logical state
  TEST_ASSERT_EQUAL(128, fix.logicalState.m_potParams[7][0].m_value);
  TEST_ASSERT_EQUAL(256, fix.logicalState.m_potParams[7][1].m_value);
  TEST_ASSERT_EQUAL(384, fix.logicalState.m_potParams[7][2].m_value);
  TEST_ASSERT_EQUAL(512, fix.logicalState.m_potParams[7][3].m_value);
}

void test_driver_pot_value_changed_not_sets_logical_state_disabled_pot() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_potParams[7][1].m_state = PotState::kDisabled;
  fix.logicalState.m_potParams[7][3].m_state = PotState::kDisabled;
  fix.logicalState.m_currentProgram = 7;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Sent events
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kPot0, 128));
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kPot1, 256));
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kPot2, 384));
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kMixPot, 512));

  // Test logical state
  TEST_ASSERT_EQUAL(128, fix.logicalState.m_potParams[7][0].m_value);
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_potParams[7][1].m_value);
  TEST_ASSERT_EQUAL(384, fix.logicalState.m_potParams[7][2].m_value);
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_potParams[7][3].m_value);
}

void test_driver_pot0_value_changed_not_sets_logical_state_delay_effect() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_currentProgram = 0;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Sent event
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kPot0, 128));

  // Test logical state
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_potParams[0][0].m_value);
}

// =============================================================================
// Main
// =============================================================================

void test_logic_expr_value_changed_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_currentProgram = 7;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Sent events
  fix.publishAndDispatchAllEvents(makeLogicExprValueChangedEvent(PotId::kPot0, 128));
  fix.publishAndDispatchAllEvents(makeLogicExprValueChangedEvent(PotId::kPot1, 256));
  fix.publishAndDispatchAllEvents(makeLogicExprValueChangedEvent(PotId::kPot2, 384));
  fix.publishAndDispatchAllEvents(makeLogicExprValueChangedEvent(PotId::kMixPot, 512));

  // Expression Pedal Input
  TEST_ASSERT_EQUAL(128, fix.logicalState.m_potParams[7][0].m_value);
  TEST_ASSERT_EQUAL(256, fix.logicalState.m_potParams[7][1].m_value);
  TEST_ASSERT_EQUAL(384, fix.logicalState.m_potParams[7][2].m_value);
  TEST_ASSERT_EQUAL(512, fix.logicalState.m_potParams[7][3].m_value);
}

void test_logic_expr_value_changed_sets_logical_state_disabled_pot() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_potParams[7][1].m_state = PotState::kDisabled;
  fix.logicalState.m_potParams[7][3].m_state = PotState::kDisabled;
  fix.logicalState.m_currentProgram = 7;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Sent events
  fix.publishAndDispatchAllEvents(makeLogicExprValueChangedEvent(PotId::kPot0, 128));
  fix.publishAndDispatchAllEvents(makeLogicExprValueChangedEvent(PotId::kPot1, 256));
  fix.publishAndDispatchAllEvents(makeLogicExprValueChangedEvent(PotId::kPot2, 384));
  fix.publishAndDispatchAllEvents(makeLogicExprValueChangedEvent(PotId::kMixPot, 512));

  // Test logical state
  TEST_ASSERT_EQUAL(128, fix.logicalState.m_potParams[7][0].m_value);
  TEST_ASSERT_EQUAL(256, fix.logicalState.m_potParams[7][1].m_value);
  TEST_ASSERT_EQUAL(384, fix.logicalState.m_potParams[7][2].m_value);
  TEST_ASSERT_EQUAL(512, fix.logicalState.m_potParams[7][3].m_value);
}

// =============================================================================
// Menu Pot Edit
// =============================================================================

void test_ui_pot_value_changed_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Sent events
  fix.publishAndDispatchAllEvents(makeUIPotValueChangedEvent(PotId::kPot0, 10));
  fix.publishAndDispatchAllEvents(makeUIPotValueChangedEvent(PotId::kPot1, 20));
  fix.publishAndDispatchAllEvents(makeUIPotValueChangedEvent(PotId::kPot2, 30));
  fix.publishAndDispatchAllEvents(makeUIPotValueChangedEvent(PotId::kMixPot, 40));

  // Test logical state
  TEST_ASSERT_EQUAL(10, fix.logicalState.m_potParams[0][0].m_value);
  TEST_ASSERT_EQUAL(20, fix.logicalState.m_potParams[0][1].m_value);
  TEST_ASSERT_EQUAL(30, fix.logicalState.m_potParams[0][2].m_value);
  TEST_ASSERT_EQUAL(40, fix.logicalState.m_potParams[0][3].m_value);
}

// =============================================================================
// MIDI Pot Control
// =============================================================================

void test_midi_cc_pot_value_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send CC message
  makeMidiCCPotValueChangedMessage(fix.midiService.getMidiHandler(), PotId::kPot0, 16);

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test logicals tate
  TEST_ASSERT_EQUAL(128, fix.logicalState.m_potParams[0][0].m_value);

  // Send CC message
  makeMidiCCPotValueChangedMessage(fix.midiService.getMidiHandler(), PotId::kPot1, 32);

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test logicals tate
  TEST_ASSERT_EQUAL(257, fix.logicalState.m_potParams[0][1].m_value);

  // Send CC message
  makeMidiCCPotValueChangedMessage(fix.midiService.getMidiHandler(), PotId::kPot2, 48);

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test logicals tate
  TEST_ASSERT_EQUAL(386, fix.logicalState.m_potParams[0][2].m_value);

  // Send CC message
  makeMidiCCPotValueChangedMessage(fix.midiService.getMidiHandler(), PotId::kMixPot, 64);

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test logicals tate
  TEST_ASSERT_EQUAL(515, fix.logicalState.m_potParams[0][3].m_value);
}

// =============================================================================
// Persistence
// =============================================================================

void test_pot_params_persists() {
  InteractionFixture fix;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Set settings
  fix.publishAndDispatchAllEvents(makeUIPotSettingChangedEvent(PotId::kPot0, PotParam::kState));
  fix.publishAndDispatchAllEvents(makeUIPotSettingChangedEvent(PotId::kPot2, PotParam::kState));
  fix.publishAndDispatchAllEvents(makeUIPotSettingChangedEvent(PotId::kPot1, PotParam::kMinValue, 20));
  fix.publishAndDispatchAllEvents(makeUIPotSettingChangedEvent(PotId::kMixPot, PotParam::kMaxValue, -100));

  // Reset
  fix.init();

  // Test logical state
  TEST_ASSERT_EQUAL(PotState::kDisabled, fix.logicalState.m_potParams[0][0].m_state);
  TEST_ASSERT_EQUAL(PotState::kDisabled, fix.logicalState.m_potParams[0][2].m_state);
  TEST_ASSERT_EQUAL(20, fix.logicalState.m_potParams[0][1].m_minValue);
  TEST_ASSERT_EQUAL(923, fix.logicalState.m_potParams[0][3].m_maxValue);
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Physical Pot Input
  RUN_TEST(test_driver_pot_value_changed_sets_logical_state);
  RUN_TEST(test_driver_pot_value_changed_not_sets_logical_state_disabled_pot);
  RUN_TEST(test_driver_pot0_value_changed_not_sets_logical_state_delay_effect);

  // Expression Pedal Input
  RUN_TEST(test_logic_expr_value_changed_sets_logical_state);
  RUN_TEST(test_logic_expr_value_changed_sets_logical_state_disabled_pot);

  // Menu Pot Edit
  RUN_TEST(test_ui_pot_value_changed_sets_logical_state);

  // MIDI Pot Control
  RUN_TEST(test_midi_cc_pot_value_sets_logical_state);

  // Persistence
  RUN_TEST(test_pot_params_persists);

  return UNITY_END();
}

