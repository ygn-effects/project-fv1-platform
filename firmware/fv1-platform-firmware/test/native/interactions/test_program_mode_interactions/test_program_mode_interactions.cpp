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

Event makeDriverSwitchLongPressedEvent(SwitchId t_id) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kLongPressed;
  e.m_id = static_cast<uint8_t>(t_id);
  return e;
}

Event makeLogicProgramModeToggledEvent() {
  Event e{};
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kToggled;
  return e;
}

Event makeUIMenuUnlockedEvent() {
  Event e{};
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMenu;
  e.m_action = EventAction::kUnlocked;
  return e;
}

void setUp() {

}

void tearDown() {

}

// =============================================================================
// Physical program mode long press
// =============================================================================

void test_driver_program_mode_long_press_toggles_logical_state_preset() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kProgramMode));

  // Test LogicalState
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, fix.logicalState.m_programMode);
}

void test_driver_program_mode_long_press_toggles_logical_state_program() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kProgramMode));

  // Test LogicalState
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, fix.logicalState.m_programMode);
}

void test_driver_program_mode_long_press_toggles_fsm_preset_idle() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kProgramMode));

  // Test FSM state
  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fix.fsmService.getAppState());
}

void test_driver_program_mode_long_press_toggles_fsm_program_idle() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kProgramMode));

  // Test FSM state
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fix.fsmService.getAppState());
}

// =============================================================================
// Persistence
// =============================================================================

void test_program_mode_persists() {
  InteractionFixture fix;
  // Set state and save
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  // Change state and reset
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Test FSM state
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fix.fsmService.getAppState());
}

void test_preset_mode_persists() {
  InteractionFixture fix;
  // Set state and save
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  // Change state and reset
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Test FSM state
  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fix.fsmService.getAppState());
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

// Physical program mode long press
RUN_TEST(test_driver_program_mode_long_press_toggles_logical_state_preset);
RUN_TEST(test_driver_program_mode_long_press_toggles_logical_state_program);
RUN_TEST(test_driver_program_mode_long_press_toggles_fsm_preset_idle);
RUN_TEST(test_driver_program_mode_long_press_toggles_fsm_program_idle);

// Persistence
RUN_TEST(test_program_mode_persists);
RUN_TEST(test_preset_mode_persists);

  return UNITY_END();
}
