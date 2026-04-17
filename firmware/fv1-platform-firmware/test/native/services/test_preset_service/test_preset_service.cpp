#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/preset.h"
#include "services/preset_service.h"
#include "ui/settings.h"
#include "mock/mock_eeprom.h"

#include "../src/logic/memory_handler.cpp"
#include "../src/logic/preset_handler.cpp"
#include "../src/services/preset_service.cpp"

// =============================================================================
// Helper functions
// =============================================================================

Event makeMidiPresetValueChangeEvent(uint8_t t_bank) {
  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_bank;
  return e;
}

Event makeUIPresetValueChangeEvent(int8_t t_delta) {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;
  e.m_data.delta = t_delta;
  return e;
}

Event makeLogicPresetBankValueChangedEvent() {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kValueChanged;
  return e;
}

Event makePhysicalTapPressEvent() {
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kTap);
  return e;
}

Event makePhysicalTapLongPressEvent() {
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kLongPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kTap);
  return e;
}

Event makeLogicPresetSaveEvent(uint8_t t_bank) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kSave;
  return e;
}

Event makeLogicProgramModeToggleEvent(uint8_t t_bank) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kToggled;
  return e;
}

Event makeUIPresetSettingChangeEvent(SavePresetParam t_param, int16_t t_delta = 0) {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kSettingChanged;
  e.m_id = static_cast<uint8_t>(t_param);
  e.m_data.delta = t_delta;
  return e;
}

Event makeUiSavePresetEvent() {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kSave;
  return e;
}

void assertPresetValueChangedEventPublished() {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPreset, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
}

void assertPresetSaveEventPublished() {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPreset, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);
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

void test_init_preset_mode_changes_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Set program mode, current preset and some preset values
  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_currentPreset = 2;
  logicalState.m_loadedPresetBank.m_presets[logicalState.m_currentPreset].m_divState = DivState::kEnabled;
  logicalState.m_loadedPresetBank.m_presets[logicalState.m_currentPreset].m_tempo = 512;
  logicalState.m_loadedPresetBank.m_presets[logicalState.m_currentPreset].m_exprState = ExprState::kActive;
  logicalState.m_loadedPresetBank.m_presets[logicalState.m_currentPreset].m_direction = Direction::kInverted;

  // Init
  presetService.init();

  // Check logicalState
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(512, logicalState.m_tempo);
  TEST_ASSERT_EQUAL(ExprState::kActive, logicalState.m_exprParams[logicalState.m_currentProgram].m_state);
  TEST_ASSERT_EQUAL(Direction::kInverted, logicalState.m_exprParams[logicalState.m_currentProgram].m_direction);

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_init_program_mode_does_not_apply_preset() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Set program mode to kProgram (not kPreset)
  logicalState.m_programMode = ProgramMode::kProgram;
  logicalState.m_currentPreset = 2;

  // Set different values in preset and current state
  logicalState.m_loadedPresetBank.m_presets[logicalState.m_currentPreset].m_divState = DivState::kEnabled;
  logicalState.m_loadedPresetBank.m_presets[logicalState.m_currentPreset].m_tempo = 512;
  logicalState.m_divState = DivState::kDisabled;
  logicalState.m_tempo = 256;

  // Init
  presetService.init();

  // Check logicalState did NOT change
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(256, logicalState.m_tempo);

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_init_syncs_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Set logical state
  logicalState.m_currentPreset = 2;

  // Init
  presetService.init();

  // Check logicalState
  TEST_ASSERT_EQUAL(2, logicalState.m_saveTargetPreset);

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// Preset change tests
// =============================================================================

void test_ui_preset_loading_changes_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  presetService.handleEvent(makeUIPresetValueChangeEvent(1));

  // Check logicalState and event bus
  TEST_ASSERT_EQUAL(1, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(1, logicalState.m_saveTargetPreset);
  assertPresetValueChangedEventPublished();
  assertPresetSaveEventPublished();

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_ui_preset_loading_wraps_around() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  presetService.handleEvent(makeUIPresetValueChangeEvent(-1));

  // Check logicalState and event bus
  TEST_ASSERT_EQUAL(PresetConstants::c_presetPerBank - 1, logicalState.m_currentPreset);
  assertPresetValueChangedEventPublished();
  assertPresetSaveEventPublished();

  // Event bus should be empty
  assertEventBusEmpty();

  presetService.handleEvent(makeUIPresetValueChangeEvent(1));

  // Check logicalState and event bus
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  assertPresetValueChangedEventPublished();
  assertPresetSaveEventPublished();

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_midi_preset_loading_changes_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  presetService.handleEvent(makeMidiPresetValueChangeEvent(9));

  // Check logicalState and event bus
  TEST_ASSERT_EQUAL(1, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(1, logicalState.m_saveTargetPreset);
  assertPresetValueChangedEventPublished();
  assertPresetSaveEventPublished();

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_preset_bank_value_changed_sets_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  logicalState.m_currentPreset = 2;

  presetService.handleEvent(makeLogicPresetBankValueChangedEvent());

  // Check logicalState and event bus
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_saveTargetPreset);
  assertPresetValueChangedEventPublished();
  assertPresetSaveEventPublished();

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_physical_tap_press_changes_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // +1
  presetService.handleEvent(makePhysicalTapPressEvent());

  // Check logicalState and event bus
  TEST_ASSERT_EQUAL(1, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(1, logicalState.m_saveTargetPreset);
  assertPresetValueChangedEventPublished();
  assertPresetSaveEventPublished();

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_physical_tap_long_press_changes_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Set preset
  logicalState.m_currentPreset = 2;

  // -1
  presetService.handleEvent(makePhysicalTapLongPressEvent());

  // Check logicalState and event bus
  TEST_ASSERT_EQUAL(1, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(1, logicalState.m_saveTargetPreset);
  assertPresetValueChangedEventPublished();
  assertPresetSaveEventPublished();

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_physical_tap_wraps_around() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // -1
  presetService.handleEvent(makePhysicalTapLongPressEvent());

  // Check logicalState and event bus
  TEST_ASSERT_EQUAL(PresetConstants::c_presetPerBank - 1, logicalState.m_currentPreset);
  assertPresetValueChangedEventPublished();
  assertPresetSaveEventPublished();

  // Event bus should be empty
  assertEventBusEmpty();

  // +1
  presetService.handleEvent(makePhysicalTapPressEvent());

  // Check logicalState and event bus
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_saveTargetPreset);
  assertPresetValueChangedEventPublished();
  assertPresetSaveEventPublished();

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// Preset setting change tests
// =============================================================================

void test_ui_preset_setting_changed_sets_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Send the event
  presetService.handleEvent(makeUIPresetSettingChangeEvent(SavePresetParam::kTargetPreset, 1));

  // Check logical state
  TEST_ASSERT_EQUAL(1, logicalState.m_saveTargetPreset);

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// Apply preset tests
// =============================================================================

void test_ui_preset_change_applies_preset_data_to_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Set up preset 1 with specific data
  logicalState.m_currentPreset = 0;
  logicalState.m_loadedPresetBank.m_presets[1].m_divState = DivState::kEnabled;
  logicalState.m_loadedPresetBank.m_presets[1].m_tempo = 768;
  logicalState.m_loadedPresetBank.m_presets[1].m_exprState = ExprState::kActive;
  logicalState.m_loadedPresetBank.m_presets[1].m_direction = Direction::kInverted;

  // Change to preset 1
  presetService.handleEvent(makeUIPresetValueChangeEvent(1));

  // Verify preset data was applied to LogicalState
  TEST_ASSERT_EQUAL(1, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(768, logicalState.m_tempo);
  TEST_ASSERT_EQUAL(ExprState::kActive, logicalState.m_exprParams[logicalState.m_currentProgram].m_state);
  TEST_ASSERT_EQUAL(Direction::kInverted, logicalState.m_exprParams[logicalState.m_currentProgram].m_direction);

  // Clear event bus
  clearEventBus();
}

void test_midi_preset_change_applies_preset_data_to_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Set up preset 3 with specific data
  logicalState.m_currentPreset = 0;
  logicalState.m_loadedPresetBank.m_presets[3].m_divState = DivState::kDisabled;
  logicalState.m_loadedPresetBank.m_presets[3].m_tempo = 128;
  logicalState.m_loadedPresetBank.m_presets[3].m_exprState = ExprState::kInactive;
  logicalState.m_loadedPresetBank.m_presets[3].m_direction = Direction::kNormal;

  // Change to preset 3 via MIDI
  presetService.handleEvent(makeMidiPresetValueChangeEvent(3));

  // Verify preset data was applied to LogicalState
  TEST_ASSERT_EQUAL(3, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(128, logicalState.m_tempo);
  TEST_ASSERT_EQUAL(ExprState::kInactive, logicalState.m_exprParams[logicalState.m_currentProgram].m_state);
  TEST_ASSERT_EQUAL(Direction::kNormal, logicalState.m_exprParams[logicalState.m_currentProgram].m_direction);

  // Clear event bus
  clearEventBus();
}

void test_physical_tap_applies_preset_data_to_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Set up preset 1 with specific data
  logicalState.m_currentPreset = 0;
  logicalState.m_loadedPresetBank.m_presets[1].m_divState = DivState::kEnabled;
  logicalState.m_loadedPresetBank.m_presets[1].m_tempo = 999;
  logicalState.m_loadedPresetBank.m_presets[1].m_exprState = ExprState::kActive;
  logicalState.m_loadedPresetBank.m_presets[1].m_direction = Direction::kInverted;

  // Press tap to increment to preset 1
  presetService.handleEvent(makePhysicalTapPressEvent());

  // Verify preset data was applied to LogicalState
  TEST_ASSERT_EQUAL(1, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(999, logicalState.m_tempo);
  TEST_ASSERT_EQUAL(ExprState::kActive, logicalState.m_exprParams[logicalState.m_currentProgram].m_state);
  TEST_ASSERT_EQUAL(Direction::kInverted, logicalState.m_exprParams[logicalState.m_currentProgram].m_direction);

  // Clear event bus
  clearEventBus();
}

// =============================================================================
// Save preset tests
// =============================================================================

void test_ui_save_preset_sets_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Set logical state
  logicalState.m_saveTargetPreset = 3;

  // Send the preset save event
  presetService.handleEvent(makeUiSavePresetEvent());

  // Check logical state
  TEST_ASSERT_EQUAL(logicalState.m_saveTargetPreset, logicalState.m_currentPreset);

  assertPresetSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_save_preset_invalid_value_not_sets_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Set logical state
  logicalState.m_saveTargetPreset = 12;

  // Send the preset save event
  presetService.handleEvent(makeUiSavePresetEvent());

  // Check logical state
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);

  assertEventBusEmpty();
}

void test_ui_save_preset_saves_logical_state_to_preset_bank() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Set logical state
  logicalState.m_saveTargetPreset = 3;
  logicalState.m_currentProgram = 2;
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_tempo = 300;

  // Send the preset save event
  presetService.handleEvent(makeUiSavePresetEvent());

  // Check logical state
  TEST_ASSERT_EQUAL(logicalState.m_currentProgram, logicalState.m_loadedPresetBank.m_presets[3].m_programIndex);
  TEST_ASSERT_EQUAL(logicalState.m_tapState, logicalState.m_loadedPresetBank.m_presets[3].m_tapState);
  TEST_ASSERT_EQUAL(logicalState.m_tempo, logicalState.m_loadedPresetBank.m_presets[3].m_tempo);

  assertPresetSaveEventPublished();
  assertEventBusEmpty();
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_ui_preset_value_change() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(presetService.interestedIn(e));
}

void test_interested_in_midi_preset_value_change() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(presetService.interestedIn(e));
}

void test_interested_in_physical_tap_switch_press() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Event only processed in preset mode
  logicalState.m_programMode = ProgramMode::kPreset;

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kTap);

  TEST_ASSERT_TRUE(presetService.interestedIn(e));
}

void test_interested_in_physical_tap_switch_long_press() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  // Event only processed in preset mode
  logicalState.m_programMode = ProgramMode::kPreset;

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kLongPressed;
  e.m_id = static_cast<uint8_t>(SwitchId::kTap);

  TEST_ASSERT_TRUE(presetService.interestedIn(e));
}

void test_interested_in_ui_preset_save() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kSave;

  TEST_ASSERT_TRUE(presetService.interestedIn(e));
}

void test_interested_in_ui_preset_setting_changed() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kSettingChanged;

  TEST_ASSERT_TRUE(presetService.interestedIn(e));
}

void test_interested_in_logic_program_mode_toggle() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kToggled;

  TEST_ASSERT_TRUE(presetService.interestedIn(e));
}

void test_interested_in_logic_preset_bank_value_changed() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(presetService.interestedIn(e));
}

void test_not_interested_in_physical_tap_program_mode() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kTap;
  e.m_action = EventAction::kLongPressed;

  TEST_ASSERT_FALSE(presetService.interestedIn(e));
}

void test_not_interested_in_other_events() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  Event e;

  // Not interested in physical switch events
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kEncoder;
  TEST_ASSERT_FALSE(presetService.interestedIn(e));

  // Not interested in memory preset banks events (outputs them, doesn't consume)
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPreset;
  TEST_ASSERT_FALSE(presetService.interestedIn(e));

  // Not interested in other MIDI events
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(presetService.interestedIn(e));

  // Not interested in logic Expr events
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(presetService.interestedIn(e));

  // Not interested in logic program with different action
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kSave;
  TEST_ASSERT_FALSE(presetService.interestedIn(e));

  // Not interested in nonsensical event
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kPressed;
  TEST_ASSERT_FALSE(presetService.interestedIn(e));
}

// =============================================================================
// Preset Dirty Flag tests
// =============================================================================

void test_apply_preset_clears_dirty_flag() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_presetDirty = true;

  // Switching preset triggers applyPreset
  presetService.handleEvent(makeUIPresetValueChangeEvent(1));

  TEST_ASSERT_FALSE(logicalState.m_presetDirty);
}

void test_save_preset_clears_dirty_flag() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetService presetService(logicalState, eeprom);

  logicalState.m_programMode = ProgramMode::kPreset;
  logicalState.m_presetDirty = true;
  logicalState.m_saveTargetPreset = 0;

  presetService.handleEvent(makeUiSavePresetEvent());

  TEST_ASSERT_FALSE(logicalState.m_presetDirty);
}

int main() {
  UNITY_BEGIN();

  // Init Tests
  RUN_TEST(test_init_preset_mode_changes_logical_state);
  RUN_TEST(test_init_program_mode_does_not_apply_preset);
  RUN_TEST(test_init_syncs_logical_state);

  // Preset change tests
  RUN_TEST(test_ui_preset_loading_changes_logical_state);
  RUN_TEST(test_ui_preset_loading_wraps_around);
  RUN_TEST(test_midi_preset_loading_changes_logical_state);
  RUN_TEST(test_preset_bank_value_changed_sets_logical_state);
  RUN_TEST(test_physical_tap_press_changes_logical_state);
  RUN_TEST(test_physical_tap_long_press_changes_logical_state);
  RUN_TEST(test_physical_tap_wraps_around);

  // Preset setting change tests
  RUN_TEST(test_ui_preset_setting_changed_sets_logical_state);

  // Apply preset tests
  RUN_TEST(test_ui_preset_change_applies_preset_data_to_logical_state);
  RUN_TEST(test_midi_preset_change_applies_preset_data_to_logical_state);
  RUN_TEST(test_physical_tap_applies_preset_data_to_logical_state);

  // Save preset tests
  RUN_TEST(test_ui_save_preset_sets_logical_state);
  RUN_TEST(test_ui_save_preset_invalid_value_not_sets_logical_state);
  RUN_TEST(test_ui_save_preset_saves_logical_state_to_preset_bank);

  // Preset Dirty Flag
  RUN_TEST(test_apply_preset_clears_dirty_flag);
  RUN_TEST(test_save_preset_clears_dirty_flag);

  // interestedIn Tests
  RUN_TEST(test_interested_in_midi_preset_value_change);
  RUN_TEST(test_interested_in_ui_preset_value_change);
  RUN_TEST(test_interested_in_physical_tap_switch_press);
  RUN_TEST(test_interested_in_physical_tap_switch_long_press);
  RUN_TEST(test_interested_in_ui_preset_save);
  RUN_TEST(test_interested_in_ui_preset_setting_changed);
  RUN_TEST(test_interested_in_logic_program_mode_toggle);
  RUN_TEST(test_interested_in_logic_preset_bank_value_changed);
  RUN_TEST(test_not_interested_in_physical_tap_program_mode);
  RUN_TEST(test_not_interested_in_other_events);

  UNITY_END();
}