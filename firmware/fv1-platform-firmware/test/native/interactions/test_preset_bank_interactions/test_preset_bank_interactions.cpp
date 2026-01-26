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

Event makeUiPresetBankValueChangedEvent(int8_t t_delta) {
  Event e{};
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kValueChanged;
  e.m_data.delta = t_delta;
  return e;
}

Event makeMidiPresetBankValueChangedEvent(uint8_t t_value) {
  Event e{};
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_value;
  return e;
}

void setUp() {

}

void tearDown() {

}

// =============================================================================
// UI Bank Change
// =============================================================================

void test_ui_preset_bank_change_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeUiPresetBankValueChangedEvent(1));

  // Check logicalstate
  TEST_ASSERT_EQUAL(3, fix.logicalState.m_currentPresetBank);
}

void test_ui_preset_bank_change_loads_preset_bank() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;

  // Set preset values and save
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_programIndex = 5;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_tapState = TapState::kEnabled;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_interval = 500;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_tempo = 500;
  fix.SyncEepromWithLoadedPresetBank();

  // Change preset bank, save and reset
  fix.logicalState.m_currentPresetBank = 1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeUiPresetBankValueChangedEvent(1));

  // Check loaded preset bank
  TEST_ASSERT_EQUAL(2, fix.logicalState.m_currentPresetBank);
  TEST_ASSERT_EQUAL(5, fix.logicalState.m_loadedPresetBank.m_presets[1].m_programIndex);
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_loadedPresetBank.m_presets[1].m_tapState);
  TEST_ASSERT_EQUAL(500, fix.logicalState.m_loadedPresetBank.m_presets[1].m_interval);
  TEST_ASSERT_EQUAL(500, fix.logicalState.m_loadedPresetBank.m_presets[1].m_tempo);
}

void test_ui_preset_bank_change_wraps_around() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeUiPresetBankValueChangedEvent(-3));

  // Check logical state
  TEST_ASSERT_EQUAL(PresetConstants::c_presetBankCount - 1, fix.logicalState.m_currentPresetBank);

  // Send event
  fix.publishAndDispatchAllEvents(makeUiPresetBankValueChangedEvent(1));

  // Check logical state
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_currentPresetBank);
}

// =============================================================================
// MIDI Bank Change
// =============================================================================

void test_midi_preset_bank_change_valid_index_changes_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeMidiPresetBankValueChangedEvent(5));

  // Check logicalstate
  TEST_ASSERT_EQUAL(5, fix.logicalState.m_currentPresetBank);
}

void test_midi_preset_bank_change_valid_index_loads_present_bank() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;

  // Set preset values and save
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_programIndex = 5;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_tapState = TapState::kEnabled;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_interval = 500;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_tempo = 500;
  fix.SyncEepromWithLoadedPresetBank();

  // Change preset bank, save and reset
  fix.logicalState.m_currentPresetBank = 5;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchAllEvents(makeMidiPresetBankValueChangedEvent(2));

  // Check loaded preset bank
  TEST_ASSERT_EQUAL(2, fix.logicalState.m_currentPresetBank);
  TEST_ASSERT_EQUAL(5, fix.logicalState.m_loadedPresetBank.m_presets[1].m_programIndex);
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_loadedPresetBank.m_presets[1].m_tapState);
  TEST_ASSERT_EQUAL(500, fix.logicalState.m_loadedPresetBank.m_presets[1].m_interval);
  TEST_ASSERT_EQUAL(500, fix.logicalState.m_loadedPresetBank.m_presets[1].m_tempo);
}

void test_midi_preset_bank_change_invalid_index_does_not_change_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchEvent(makeMidiPresetBankValueChangedEvent(PresetConstants::c_presetBankCount + 1));

  // Check loaded preset bank
  TEST_ASSERT_EQUAL(2, fix.logicalState.m_currentPresetBank);

  // No load event
  TEST_ASSERT_FALSE(fix.findEvent(EventDomain::kMemory, EventSubject::kPresetBank, EventAction::kLoad));
}

void test_midi_preset_bank_change_index_already_loaded_changes_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send event
  fix.publishAndDispatchEvent(makeMidiPresetBankValueChangedEvent(2));

  // Check loaded preset bank
  TEST_ASSERT_EQUAL(2, fix.logicalState.m_currentPresetBank);

  // No load event
  TEST_ASSERT_FALSE(fix.findEvent(EventDomain::kMemory, EventSubject::kPresetBank, EventAction::kLoad));
}

// =============================================================================
// Persistence
// =============================================================================

void test_current_preset_bank_persists() {
  InteractionFixture fix;
  // Set bank and save
  fix.logicalState.m_currentPresetBank = 2;
  fix.syncEepromWithState();

  // Modify and reset
  fix.logicalState.m_currentPresetBank = 0;
  fix.init();

  // Check logical state
  TEST_ASSERT_EQUAL(2, fix.logicalState.m_currentPresetBank);
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // UI Bank Change
  RUN_TEST(test_ui_preset_bank_change_sets_logical_state);
  RUN_TEST(test_ui_preset_bank_change_loads_preset_bank);
  RUN_TEST(test_ui_preset_bank_change_wraps_around);

  // MIDI Bank Change
  RUN_TEST(test_midi_preset_bank_change_valid_index_changes_logical_state);
  RUN_TEST(test_midi_preset_bank_change_valid_index_loads_present_bank);
  RUN_TEST(test_midi_preset_bank_change_invalid_index_does_not_change_logical_state);
  RUN_TEST(test_midi_preset_bank_change_index_already_loaded_changes_logical_state);

  // Persistence
  RUN_TEST(test_current_preset_bank_persists);

  return UNITY_END();
}
