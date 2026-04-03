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

Event makeMemoryBypassSaveEvent() {
  Event e{};
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kBypass;
  e.m_action = EventAction::kSave;
  return e;
}

Event makeMemoryProgramModeSaveEvent() {
  Event e{};
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kSave;
  return e;
}

Event makeMemoryProgramSaveEvent() {
  Event e{};
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kSave;
  return e;
}

Event makeMemoryPresetSaveEvent() {
  Event e{};
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kSave;
  return e;
}

Event makeMemoryPresetBankSaveEvent() {
  Event e{};
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kSave;
  return e;
}

Event makeMemoryTapSaveEvent() {
  Event e{};
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kTap;
  e.m_action = EventAction::kSave;
  return e;
}

Event makeMemoryTempoSaveEvent() {
  Event e{};
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kSave;
  return e;
}

Event makeMemoryExprSaveEvent() {
  Event e{};
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kSave;
  return e;
}

Event makeMemoryPotSaveEvent(uint8_t t_potId = 0) {
  Event e{};
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kSave;
  e.m_id = t_potId;
  return e;
}

Event makeMemoryGeneralSaveEvent() {
  Event e{};
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kSave;
  return e;
}

Event makeMemoryGeneralLoadEvent() {
  Event e{};
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kLoad;
  return e;
}

void setUp() {

}

void tearDown() {

}

// =============================================================================
// Bypass Save Events
// =============================================================================

void test_bypass_save_persists_on_reinit() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();
  fix.clearEventBus();

  // Change to active and save
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.publishAndDispatchAllEvents(makeMemoryBypassSaveEvent());

  // Reinit and verify state loaded from EEPROM
  fix.init();
  TEST_ASSERT_EQUAL(BypassState::kActive, fix.logicalState.m_bypassState);
}

// =============================================================================
// Program Mode Save Events
// =============================================================================

void test_program_mode_save_persists_on_reinit() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.clearEventBus();

  // Change to preset mode and save
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.publishAndDispatchAllEvents(makeMemoryProgramModeSaveEvent());

  // Reinit and verify state loaded from EEPROM
  fix.init();
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, fix.logicalState.m_programMode);
}

// =============================================================================
// Current Program Save Events
// =============================================================================

void test_current_program_save_persists_on_reinit() {
  InteractionFixture fix;
  fix.logicalState.m_currentProgram = 0;
  fix.syncEepromWithState();
  fix.init();
  fix.clearEventBus();

  // Change program and save
  fix.logicalState.m_currentProgram = 7;
  fix.publishAndDispatchAllEvents(makeMemoryProgramSaveEvent());

  // Reinit and verify state loaded from EEPROM
  fix.init();
  TEST_ASSERT_EQUAL(7, fix.logicalState.m_currentProgram);
}

// =============================================================================
// Tap Save Events
// =============================================================================

void test_tap_save_persists_on_reinit() {
  InteractionFixture fix;
  fix.logicalState.m_tapState = TapState::kDisabled;
  fix.syncEepromWithState();
  fix.init();
  fix.clearEventBus();

  // Change tap state and save
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_interval = 600;
  fix.publishAndDispatchAllEvents(makeMemoryTapSaveEvent());

  // Reinit and verify state loaded from EEPROM
  fix.init();
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(600, fix.logicalState.m_interval);
}

// =============================================================================
// Tempo Save Events
// =============================================================================

void test_tempo_save_persists_on_reinit() {
  InteractionFixture fix;
  fix.logicalState.m_tempo = 0;
  fix.syncEepromWithState();
  fix.init();
  fix.clearEventBus();

  // Change tempo and save
  fix.logicalState.m_tempo = 800;
  fix.publishAndDispatchAllEvents(makeMemoryTempoSaveEvent());

  // Reinit and verify state loaded from EEPROM
  fix.init();
  TEST_ASSERT_EQUAL(800, fix.logicalState.m_tempo);
}

// =============================================================================
// Expression Save Events
// =============================================================================

void test_expr_save_persists_on_reinit() {
  InteractionFixture fix;
  fix.logicalState.m_currentProgram = 3;
  fix.logicalState.m_exprParams[3].m_state = ExprState::kInactive;
  fix.syncEepromWithState();
  fix.init();
  fix.clearEventBus();

  // Change expr params and save
  fix.logicalState.m_exprParams[3].m_state = ExprState::kActive;
  fix.logicalState.m_exprParams[3].m_heelValue = 200;
  fix.publishAndDispatchAllEvents(makeMemoryExprSaveEvent());

  // Reinit and verify state loaded from EEPROM
  fix.init();
  TEST_ASSERT_EQUAL(ExprState::kActive, fix.logicalState.m_exprParams[3].m_state);
  TEST_ASSERT_EQUAL(200, fix.logicalState.m_exprParams[3].m_heelValue);
}

// =============================================================================
// Pot Save Events
// =============================================================================

void test_pot_save_persists_on_reinit() {
  InteractionFixture fix;
  fix.logicalState.m_currentProgram = 4;
  fix.logicalState.m_potParams[4][1].m_value = 0;
  fix.syncEepromWithState();
  fix.init();
  fix.clearEventBus();

  // Change pot value and save
  fix.logicalState.m_potParams[4][1].m_value = 768;
  fix.publishAndDispatchAllEvents(makeMemoryPotSaveEvent(1));

  // Reinit and verify state loaded from EEPROM
  fix.init();
  TEST_ASSERT_EQUAL(768, fix.logicalState.m_potParams[4][1].m_value);
}

// =============================================================================
// General Save Events
// =============================================================================

void test_general_save_persists_on_reinit() {
  InteractionFixture fix;
  fix.syncEepromWithState();
  fix.init();
  fix.clearEventBus();

  // Change multiple fields and do full save
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.logicalState.m_currentProgram = 3;
  fix.logicalState.m_potParams[3][0].m_value = 555;
  fix.publishAndDispatchAllEvents(makeMemoryGeneralSaveEvent());

  // Reinit and verify all state loaded
  fix.init();
  TEST_ASSERT_EQUAL(BypassState::kBypassed, fix.logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(3, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(555, fix.logicalState.m_potParams[3][0].m_value);
}

// =============================================================================
// General Load Events
// =============================================================================

void test_general_load_deserializes_logical_state() {
  InteractionFixture fix;

  // Set up specific state and sync to EEPROM
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentProgram = 4;
  fix.logicalState.m_tempo = 400;
  fix.syncEepromWithState();

  // Now change state in memory
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.logicalState.m_currentProgram = 0;
  fix.logicalState.m_tempo = 0;

  // Trigger load
  fix.publishAndDispatchAllEvents(makeMemoryGeneralLoadEvent());

  // Verify state restored from EEPROM
  TEST_ASSERT_EQUAL(BypassState::kBypassed, fix.logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, fix.logicalState.m_programMode);
  TEST_ASSERT_EQUAL(4, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_tempo);
}

void test_save_after_load_round_trip() {
  InteractionFixture fix;

  // Initial state
  fix.logicalState.m_tempo = 500;
  fix.syncEepromWithState();

  // Modify in memory
  fix.logicalState.m_tempo = 0;

  // Load from EEPROM
  fix.publishAndDispatchAllEvents(makeMemoryGeneralLoadEvent());
  TEST_ASSERT_EQUAL(500, fix.logicalState.m_tempo);

  // Modify and save
  fix.logicalState.m_tempo = 750;
  fix.publishAndDispatchAllEvents(makeMemoryTempoSaveEvent());

  // Reset state and load again
  fix.logicalState.m_tempo = 0;
  fix.publishAndDispatchAllEvents(makeMemoryGeneralLoadEvent());
  TEST_ASSERT_EQUAL(750, fix.logicalState.m_tempo);
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Bypass Save Events
  RUN_TEST(test_bypass_save_persists_on_reinit);

  // Program Mode Save Events
  RUN_TEST(test_program_mode_save_persists_on_reinit);

  // Current Program Save Events
  RUN_TEST(test_current_program_save_persists_on_reinit);

  // Tap Save Events
  RUN_TEST(test_tap_save_persists_on_reinit);

  // Tempo Save Events
  RUN_TEST(test_tempo_save_persists_on_reinit);

  // Expression Save Events
  RUN_TEST(test_expr_save_persists_on_reinit);

  // Pot Save Events
  RUN_TEST(test_pot_save_persists_on_reinit);

  // General Save Events
  RUN_TEST(test_general_save_persists_on_reinit);

  // General Load Events
  RUN_TEST(test_general_load_deserializes_logical_state);
  RUN_TEST(test_save_after_load_round_trip);

  return UNITY_END();
}
