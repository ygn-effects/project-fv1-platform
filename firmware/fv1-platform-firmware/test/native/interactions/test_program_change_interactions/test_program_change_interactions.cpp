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

Event makeUiProgramValueChangedEvent(int8_t t_delta) {
  Event e{};
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_data.delta = t_delta;
  return e;
}

Event makeUIPresetValueChangedEvent(int8_t t_delta) {
  Event e{};
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;
  e.m_data.delta = t_delta;
  return e;
}

Event makeLogicProgramModeToggledEvent() {
  Event e{};
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kToggled;
  return e;
}

Event makeLogicPresetBankValueChangedEvent() {
  Event e{};
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kValueChanged;
  return e;
}

Event makeMidiProgramValueChangedEvent(uint8_t t_program) {
  Event e{};
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_program;
  return e;
}

Event makeMidiPresetValueChangedEvent(uint8_t t_preset) {
  Event e{};
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_preset;
  return e;
}

void setUp() {

}

void tearDown() {

}

// =============================================================================
// UI Program Change
// =============================================================================

void test_ui_program_change_sets_logical_state() {
  InteractionFixture fix;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeUiProgramValueChangedEvent(1));

  // Check logicalstate
  TEST_ASSERT_EQUAL(1, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[1], fix.logicalState.m_activeProgram);
}

// =============================================================================
// UI Program Change
// =============================================================================

void test_ui_preset_change_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  fix.logicalState.m_loadedPresetBank.m_presets[2].m_programIndex = 3;
  fix.syncEepromWithState();
  fix.SyncEepromWithLoadedPresetBank();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeUIPresetValueChangedEvent(1));

  // Check logicalstate
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[3], fix.logicalState.m_activeProgram);
}

// =============================================================================
// Program Mode Toggled
// =============================================================================

void test_program_mode_toggled_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_programIndex = 3;
  fix.syncEepromWithState();
  fix.SyncEepromWithLoadedPresetBank();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeLogicProgramModeToggledEvent());

  // Check logicalstate, the pointer shouldn't have changed
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[3], fix.logicalState.m_activeProgram);
}

// =============================================================================
// Preset Bank Loaded
// =============================================================================

void test_preset_bank_load_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  // Preset 0 is applied on bank load
  fix.logicalState.m_loadedPresetBank.m_presets[0].m_programIndex = 3;
  fix.syncEepromWithState();
  fix.SyncEepromWithLoadedPresetBank();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeLogicPresetBankValueChangedEvent());

  // Check logicalstate, the pointer shouldn't have changed
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[3], fix.logicalState.m_activeProgram);
}

// =============================================================================
// MIDI Preset Change
// =============================================================================

void test_midi_preset_change_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  fix.logicalState.m_loadedPresetBank.m_presets[2].m_programIndex = 3;
  fix.syncEepromWithState();
  fix.SyncEepromWithLoadedPresetBank();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeMidiPresetValueChangedEvent(10));

  // Check logicalstate, the pointer should have changed
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[3], fix.logicalState.m_activeProgram);

}

// =============================================================================
// MIDI Program Change
// =============================================================================

void test_midi_program_change_sets_logical_state() {
  InteractionFixture fix;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeMidiProgramValueChangedEvent(1));

  // Check logicalstate
  TEST_ASSERT_EQUAL(1, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[1], fix.logicalState.m_activeProgram);
}

// =============================================================================
// Persistence
// =============================================================================

void test_current_program_persistence() {
  InteractionFixture fix;
  // Set value and save
  fix.logicalState.m_currentProgram = 4;
  fix.syncEepromWithState();

  // Set and reset
  fix.logicalState.m_currentProgram = 2;
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Check logicalstate
  TEST_ASSERT_EQUAL(4, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[4], fix.logicalState.m_activeProgram);
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // UI Program Change
  RUN_TEST(test_ui_program_change_sets_logical_state);

  // UI Preset Change
  RUN_TEST(test_ui_preset_change_sets_logical_state);

  // Program Mode Toggled
  RUN_TEST(test_program_mode_toggled_sets_logical_state);

  // Preset Bank Loaded
  RUN_TEST(test_preset_bank_load_sets_logical_state);

  // MIDI Preset/Program Change
  RUN_TEST(test_midi_preset_change_sets_logical_state);
  RUN_TEST(test_midi_program_change_sets_logical_state);

  // Persistence
  RUN_TEST(test_current_program_persistence);

  return UNITY_END();
}
