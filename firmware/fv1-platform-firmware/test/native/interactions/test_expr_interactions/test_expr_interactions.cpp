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

Event makeDriverExprConnectedEvent() {
  Event e;
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kConnected;
  return e;
}

Event makeDriverExprDisconnectedEvent() {
  Event e;
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kDisconnected;
  return e;
}

Event makeDriverExprValueChangedEvent(uint16_t t_value) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_value;
  return e;
}

Event makeUIExprEvent(ExprParam t_param, int16_t t_delta = 0) {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kDeltaChanged;
  e.m_id = static_cast<uint8_t>(t_param);
  e.m_data.delta = t_delta;
  return e;
}

void setUp() {

}

void tearDown() {

}

// =============================================================================
// Connection / Disconnection
// =============================================================================

void test_driver_expr_connected_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_exprConnected = false;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeDriverExprConnectedEvent());

  // Test logical state
  TEST_ASSERT_TRUE(fix.logicalState.m_exprConnected);
}

void test_driver_expr_disconnected_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_exprConnected = true;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeDriverExprDisconnectedEvent());

  // Test logical state
  TEST_ASSERT_FALSE(fix.logicalState.m_exprConnected);
}

// =============================================================================
// Physical Pedal Movement
// =============================================================================

void test_driver_expr_value_changed_connected_active_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_exprConnected = true;
  // Use non-delay program so POT0 is handled as a regular pot
  fix.logicalState.m_currentProgram = 7;
  fix.logicalState.m_exprParams[7].m_state = ExprState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeDriverExprValueChangedEvent(512));

  // Test logical state
  TEST_ASSERT_EQUAL(512, fix.logicalState.m_potParams[7][0].m_value);
}

void test_driver_expr_value_changed_connected_disabled_not_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_exprConnected = true;
  fix.logicalState.m_currentProgram = 0;
  fix.logicalState.m_exprParams[0].m_state = ExprState::kInactive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeDriverExprValueChangedEvent(512));

  // Test logical state
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_potParams[0][0].m_value);
}

void test_driver_expr_value_changed_disconnected_active_not_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_exprConnected = false;
  // Use non-delay program so POT0 is handled as a regular pot
  fix.logicalState.m_currentProgram = 7;
  fix.logicalState.m_exprParams[7].m_state = ExprState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeDriverExprValueChangedEvent(512));

  // Test logical state
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_potParams[7][0].m_value);
}

void test_driver_expr_value_changed_disconnected_disabled_not_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_exprConnected = false;
  fix.logicalState.m_currentProgram = 0;
  fix.logicalState.m_exprParams[0].m_state = ExprState::kInactive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeDriverExprValueChangedEvent(512));

  // Test logical state
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_potParams[0][0].m_value);
}

// =============================================================================
// Persistence
// =============================================================================

void test_expr_params_persist() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_currentProgram = 0;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Set some params
  fix.publishAndDispatchAllEvents(makeUIExprEvent(ExprParam::kState));

  // Advance clock and update
  fix.mockClock.advanceBy(SettingsServiceConstants::c_editTimeout);
  fix.updateAllServices();

  fix.logicalState.m_currentProgram = 2;
  fix.publishAndDispatchAllEvents(makeUIExprEvent(ExprParam::kDirection));

  // Advance clock and update
  fix.mockClock.advanceBy(SettingsServiceConstants::c_editTimeout);
  fix.updateAllServices();

  fix.logicalState.m_currentProgram = 4;
  fix.publishAndDispatchAllEvents(makeUIExprEvent(ExprParam::kHeel, 20));

  // Advance clock and update
  fix.mockClock.advanceBy(SettingsServiceConstants::c_editTimeout);
  fix.updateAllServices();

  fix.logicalState.m_currentProgram = 5;
  fix.publishAndDispatchAllEvents(makeUIExprEvent(ExprParam::kToe, -20));

  // Advance clock and update
  fix.mockClock.advanceBy(SettingsServiceConstants::c_editTimeout);
  fix.updateAllServices();

  fix.logicalState.m_currentProgram = 7;
  fix.publishAndDispatchAllEvents(makeUIExprEvent(ExprParam::kMappedPot, 2));

  // Advance clock and update
  fix.mockClock.advanceBy(SettingsServiceConstants::c_editTimeout);
  fix.updateAllServices();

  // Reset
  fix.init();

  // Test logical state
  TEST_ASSERT_EQUAL(ExprState::kActive, fix.logicalState.m_exprParams[0].m_state);
  TEST_ASSERT_EQUAL(Direction::kInverted, fix.logicalState.m_exprParams[2].m_direction);
  TEST_ASSERT_EQUAL(20, fix.logicalState.m_exprParams[4].m_heelValue);
  TEST_ASSERT_EQUAL(1003, fix.logicalState.m_exprParams[5].m_toeValue);
  TEST_ASSERT_EQUAL(MappedPot::kPot2, fix.logicalState.m_exprParams[7].m_mappedPot);
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Connection / Disconnection
  RUN_TEST(test_driver_expr_connected_sets_logical_state);
  RUN_TEST(test_driver_expr_disconnected_sets_logical_state);

  // Physical Pedal Movement
  RUN_TEST(test_driver_expr_value_changed_connected_active_sets_logical_state);
  RUN_TEST(test_driver_expr_value_changed_connected_disabled_not_sets_logical_state);
  RUN_TEST(test_driver_expr_value_changed_disconnected_active_not_sets_logical_state);
  RUN_TEST(test_driver_expr_value_changed_disconnected_disabled_not_sets_logical_state);

  // Persistence
  RUN_TEST(test_expr_params_persist);

  return UNITY_END();
}
