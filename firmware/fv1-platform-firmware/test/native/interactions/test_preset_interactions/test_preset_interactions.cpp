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

Event makeDriverSwitchPressedEvent(SwitchId t_id) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;
  e.m_id = static_cast<uint8_t>(t_id);
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

Event makeLogicPresetBankLoadEvent() {
  Event e{};
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kLoad;
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

Event makeMidiPresetValueChangedEvent(uint8_t t_value) {
  Event e{};
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_value;
  return e;
}

Event makeLogicProgramModeToggledEvent() {
  Event e{};
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kToggled;
  return e;
}

Event makeMemoryLoadLogicalStateEvent() {
  Event e{};
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kLoad;
  return e;
}

void populatePresetBank(LogicalState& t_state) {
  t_state.m_loadedPresetBank.m_presets[0].m_programIndex = 1;
  t_state.m_loadedPresetBank.m_presets[0].m_tapState = TapState::kEnabled;
  t_state.m_loadedPresetBank.m_presets[0].m_divState = DivState::kDisabled;
  t_state.m_loadedPresetBank.m_presets[0].m_interval = 300;
  t_state.m_loadedPresetBank.m_presets[0].m_tempo = 300;
  t_state.m_loadedPresetBank.m_presets[1].m_programIndex = 3;
  t_state.m_loadedPresetBank.m_presets[1].m_tapState = TapState::kEnabled;
  t_state.m_loadedPresetBank.m_presets[1].m_divState = DivState::kEnabled;
  t_state.m_loadedPresetBank.m_presets[1].m_divValue = DivValue::kEight;
  t_state.m_loadedPresetBank.m_presets[1].m_interval = 300;
  t_state.m_loadedPresetBank.m_presets[1].m_tempo = 150;
  t_state.m_loadedPresetBank.m_presets[2].m_programIndex = 4;
  t_state.m_loadedPresetBank.m_presets[2].m_tapState = TapState::kDisabled;
  t_state.m_loadedPresetBank.m_presets[2].m_divState = DivState::kDisabled;
  t_state.m_loadedPresetBank.m_presets[2].m_interval = 500;
  t_state.m_loadedPresetBank.m_presets[2].m_tempo = 500;
  t_state.m_loadedPresetBank.m_presets[3].m_programIndex = 7;
  t_state.m_loadedPresetBank.m_presets[3].m_tapState = TapState::kDisabled;
  t_state.m_loadedPresetBank.m_presets[3].m_divState = DivState::kDisabled;
  t_state.m_loadedPresetBank.m_presets[3].m_interval = 0;
  t_state.m_loadedPresetBank.m_presets[3].m_tempo = 0;
}

void setUp() {

}

void tearDown() {

}

// =============================================================================
// Physical Tap Press
// =============================================================================

void test_driver_tap_switch_press_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPreset = 1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kTap));

  // Test logical state
  TEST_ASSERT_EQUAL(2, fix.logicalState.m_currentPreset);
}

void test_driver_tap_switch_long_press_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPreset = 3;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kTap));

  // Test logical state
  TEST_ASSERT_EQUAL(2, fix.logicalState.m_currentPreset);
}

void test_driver_tap_switch_press_wraps_around_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPreset = 3;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kTap));

  // Test logical state
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_currentPreset);
}

void test_driver_tap_switch_long_press_wraps_around_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPreset = 0;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kTap));

  // Test logical state
  TEST_ASSERT_EQUAL(3, fix.logicalState.m_currentPreset);
}

void test_driver_tap_switch_press_ignored_program_mode() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.logicalState.m_currentPreset = 3;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kTap));

  // Test logical state
  TEST_ASSERT_EQUAL(3, fix.logicalState.m_currentPreset);
}

void test_driver_tap_switch_long_press_ignored_program_mode() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.logicalState.m_currentPreset = 3;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kTap));

  // Test logical state
  TEST_ASSERT_EQUAL(3, fix.logicalState.m_currentPreset);
}

void test_driver_tap_switch_press_applies_preset() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 2;
  populatePresetBank(fix.logicalState);
  fix.SyncEepromWithLoadedPresetBank();
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kTap));

  // Test logical state
  TEST_ASSERT_EQUAL(3, fix.logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(7, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(TapState::kDisabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_tempo);
}

void test_driver_tap_switch_long_press_applies_preset() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 2;
  populatePresetBank(fix.logicalState);
  fix.SyncEepromWithLoadedPresetBank();
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kTap));

  // Test logical state
  TEST_ASSERT_EQUAL(1, fix.logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(3, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivValue::kEight, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(300, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(150, fix.logicalState.m_tempo);
}

// =============================================================================
// Menu Preset Change
// =============================================================================

void test_ui_menu_preset_change_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPreset = 1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeUIPresetValueChangedEvent(1));

  // Test logical state
  TEST_ASSERT_EQUAL(2, fix.logicalState.m_currentPreset);
}

void test_ui_menu_preset_change_wraps_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPreset = 0;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeUIPresetValueChangedEvent(-1));

  // Test logical state
  TEST_ASSERT_EQUAL(PresetConstants::c_presetPerBank - 1, fix.logicalState.m_currentPreset);

  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeUIPresetValueChangedEvent(1));

  // Test logical state
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_currentPreset);
}

void test_ui_menu_preset_change_applies_preset() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  populatePresetBank(fix.logicalState);
  fix.SyncEepromWithLoadedPresetBank();
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeUIPresetValueChangedEvent(1));

  // Test logical state
  TEST_ASSERT_EQUAL(2, fix.logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(4, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(TapState::kDisabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(500, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(500, fix.logicalState.m_tempo);
}

// =============================================================================
// MIDI Preset Change
// =============================================================================

void test_midi_preset_change_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPreset = 1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeMidiPresetValueChangedEvent(3));

  // Test logical state
  TEST_ASSERT_EQUAL(3, fix.logicalState.m_currentPreset);
}

void test_midi_preset_change_invalid_value_does_not_set_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPreset = 1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeMidiPresetValueChangedEvent(10));

  // Test logical state
  TEST_ASSERT_EQUAL(1, fix.logicalState.m_currentPreset);
}

void test_midi_preset_change_applies_preset() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 3;
  populatePresetBank(fix.logicalState);
  fix.SyncEepromWithLoadedPresetBank();
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeMidiPresetValueChangedEvent(0));

  // Test logical state
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(1, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(300, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(300, fix.logicalState.m_tempo);
}

// =============================================================================
// Program Mode Toggled
// =============================================================================

void test_program_mode_toggled_to_preset_applies_preset() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  populatePresetBank(fix.logicalState);
  fix.SyncEepromWithLoadedPresetBank();
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeLogicProgramModeToggledEvent());

  // Test logical state
  TEST_ASSERT_EQUAL(1, fix.logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(3, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivValue::kEight, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(300, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(150, fix.logicalState.m_tempo);
}

void test_program_mode_toggled_to_program_does_not_apply_preset() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentProgram = 6;
  fix.logicalState.m_tapState = TapState::kDisabled;
  fix.logicalState.m_divState = DivState::kDisabled;
  fix.logicalState.m_divValue = DivValue::kQuarter;
  fix.logicalState.m_interval = 0;
  fix.logicalState.m_tempo = 0;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  populatePresetBank(fix.logicalState);
  fix.SyncEepromWithLoadedPresetBank();
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Toggle mode event is sent only after the logical state is restored
  fix.publishAndDispatchAllEvents(makeMemoryLoadLogicalStateEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeLogicProgramModeToggledEvent());

  // Test logical state
  TEST_ASSERT_EQUAL(6, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(TapState::kDisabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_tempo);
}

// =============================================================================
// Preset Bank Loaded
// =============================================================================

void test_preset_bank_loaded_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPreset = 1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeLogicPresetBankLoadEvent());

  // Test logical state
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_currentPreset);
}

void test_preset_bank_loaded_applies_preset() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 3;
  populatePresetBank(fix.logicalState);
  fix.SyncEepromWithLoadedPresetBank();
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send program mode switch event
  fix.publishAndDispatchAllEvents(makeLogicPresetBankLoadEvent());

  // Test logical state
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(1, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(300, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(300, fix.logicalState.m_tempo);
}

// =============================================================================
// Persistence
// =============================================================================

void test_current_preset_persists() {
  InteractionFixture fix;
  // Set preset and save
  fix.logicalState.m_currentPreset = 3;
  fix.syncEepromWithState();

  // Reset
  fix.init();

  // Check logical state
  TEST_ASSERT_EQUAL(3, fix.logicalState.m_currentPreset);
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Physical Tap Press
  RUN_TEST(test_driver_tap_switch_press_sets_logical_state);
  RUN_TEST(test_driver_tap_switch_long_press_sets_logical_state);
  RUN_TEST(test_driver_tap_switch_press_wraps_around_logical_state);
  RUN_TEST(test_driver_tap_switch_long_press_wraps_around_logical_state);
  RUN_TEST(test_driver_tap_switch_press_ignored_program_mode);
  RUN_TEST(test_driver_tap_switch_long_press_ignored_program_mode);
  RUN_TEST(test_driver_tap_switch_press_applies_preset);
  RUN_TEST(test_driver_tap_switch_long_press_applies_preset);

  // Menu Preset Change
  RUN_TEST(test_ui_menu_preset_change_sets_logical_state);
  RUN_TEST(test_ui_menu_preset_change_wraps_logical_state);
  RUN_TEST(test_ui_menu_preset_change_applies_preset);

  // MIDI Preset Change
  RUN_TEST(test_midi_preset_change_sets_logical_state);
  RUN_TEST(test_midi_preset_change_sets_logical_state);
  RUN_TEST(test_midi_preset_change_applies_preset);

  // Program Mode Toggled
  RUN_TEST(test_program_mode_toggled_to_preset_applies_preset);
  RUN_TEST(test_program_mode_toggled_to_program_does_not_apply_preset);

  // Preset Bank Loaded
  RUN_TEST(test_preset_bank_loaded_sets_logical_state);
  RUN_TEST(test_preset_bank_loaded_applies_preset);

  // Persistence
  RUN_TEST(test_current_preset_persists);

  return UNITY_END();
}


