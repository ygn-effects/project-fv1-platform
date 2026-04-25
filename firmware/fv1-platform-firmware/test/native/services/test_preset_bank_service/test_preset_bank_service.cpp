#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/preset.h"
#include "services/preset_bank_service.h"
#include "ui/settings.h"
#include "mock/mock_eeprom.h"

#include "../src/logic/memory_handler.cpp"
#include "../src/services/preset_bank_service.cpp"

// =============================================================================
// Helper functions
// =============================================================================

Event makeMidiPresetValueChangeEvent(uint8_t t_preset) {
  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_preset;
  return e;
}

Event makeUIPresetBankValueChangeEvent(int8_t t_delta) {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kValueChanged;
  e.m_data.delta = t_delta;
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

void assertPresetBankValueChangedEventPublished() {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPresetBank, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
}

void assertPresetBankSaveEventPublished() {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kPresetBank, e.m_subject);
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

void test_init_load_preset_bank_from_eeprom() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  // Modify random values
  logicalState.m_loadedPresetBank.m_presets[0].m_divState = DivState::kEnabled;
  logicalState.m_loadedPresetBank.m_presets[1].m_divValue = DivValue::kEight;
  logicalState.m_loadedPresetBank.m_presets[2].m_interval = 512;
  logicalState.m_loadedPresetBank.m_presets[3].m_exprState = ExprState::kActive;

  // Init
  presetBankService.init();

  // Loading from blank EEPROM should reset everything to default
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_loadedPresetBank.m_presets[0].m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, logicalState.m_loadedPresetBank.m_presets[1].m_divValue);
  TEST_ASSERT_EQUAL(0, logicalState.m_loadedPresetBank.m_presets[2].m_interval);
  TEST_ASSERT_EQUAL(ExprState::kInactive, logicalState.m_loadedPresetBank.m_presets[3].m_exprState);
}

void test_init_syncs_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  // Set logical state
  logicalState.m_currentPresetBank = 1;

  // Init
  presetBankService.init();

  // Check logicalState
  TEST_ASSERT_EQUAL(1, logicalState.m_saveTargetBank);

  // Event bus should be empty
  assertEventBusEmpty();
}

// =============================================================================
// Preset setting change tests
// =============================================================================

void test_ui_preset_setting_changed_sets_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  // Send the event
  presetBankService.handleEvent(makeUIPresetSettingChangeEvent(SavePresetParam::kTargetBank, 1));

  // Check logical state
  TEST_ASSERT_EQUAL(1, logicalState.m_saveTargetBank);

  // Event bus should be empty
  assertEventBusEmpty();
}

void test_ui_preset_setting_changed_clamps_at_max() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  logicalState.m_saveTargetBank = PresetConstants::c_presetBankCount - 1;

  presetBankService.handleEvent(makeUIPresetSettingChangeEvent(SavePresetParam::kTargetBank, 1));

  TEST_ASSERT_EQUAL(PresetConstants::c_presetBankCount - 1, logicalState.m_saveTargetBank);
  assertEventBusEmpty();
}

void test_ui_preset_setting_changed_clamps_at_min() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  logicalState.m_saveTargetBank = 0;

  presetBankService.handleEvent(makeUIPresetSettingChangeEvent(SavePresetParam::kTargetBank, -1));

  TEST_ASSERT_EQUAL(0, logicalState.m_saveTargetBank);
  assertEventBusEmpty();
}

void test_ui_preset_setting_changed_unknown_id_does_nothing() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  logicalState.m_saveTargetBank = 2;

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kSettingChanged;
  e.m_id = 99;
  e.m_data.delta = 1;
  presetBankService.handleEvent(e);

  TEST_ASSERT_EQUAL(2, logicalState.m_saveTargetBank);
  assertEventBusEmpty();
}

// =============================================================================
// Preset bank loading tests
// =============================================================================

void test_midi_bank_loading_changes_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  presetBankService.init();

  // MIDI event
  presetBankService.handleEvent(makeMidiPresetValueChangeEvent(15));

  // Check logical state and event bus
  TEST_ASSERT_EQUAL(3, logicalState.m_currentPresetBank);
  TEST_ASSERT_EQUAL(3, logicalState.m_saveTargetBank);
  assertPresetBankSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_bank_loading_changes_logical_state() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  presetBankService.init();

  // UI event
  presetBankService.handleEvent(makeUIPresetBankValueChangeEvent(1));

  // Check logical state and event bus
  TEST_ASSERT_EQUAL(1, logicalState.m_currentPresetBank);
  TEST_ASSERT_EQUAL(1, logicalState.m_saveTargetBank);
  assertPresetBankValueChangedEventPublished();
  assertPresetBankSaveEventPublished();

  // UI event
  presetBankService.handleEvent(makeUIPresetBankValueChangeEvent(-1));

  // Check logical state and event bus
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);
  TEST_ASSERT_EQUAL(0, logicalState.m_saveTargetBank);
  assertPresetBankValueChangedEventPublished();
  assertPresetBankSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_bank_loading_wraps_around() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  presetBankService.init();

  // UI event
  presetBankService.handleEvent(makeUIPresetBankValueChangeEvent(-1));

  // Check logical state and event bus
  TEST_ASSERT_EQUAL(PresetConstants::c_presetBankCount - 1, logicalState.m_currentPresetBank);
  assertPresetBankValueChangedEventPublished();
  assertPresetBankSaveEventPublished();

  // UI event
  presetBankService.handleEvent(makeUIPresetBankValueChangeEvent(1));

  // Check logical state and event bus
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);
  assertPresetBankValueChangedEventPublished();
  assertPresetBankSaveEventPublished();
}

void test_bank_loading_out_of_range() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  // Set the current preset
  logicalState.m_currentPresetBank = 1;
  presetBankService.init();

  // Bogus value
  presetBankService.handleEvent(makeMidiPresetValueChangeEvent(127));

  // Check logical state and event bus
  TEST_ASSERT_EQUAL(1, logicalState.m_currentPresetBank);
  assertEventBusEmpty();
}

void test_wont_load_same_midi_bank() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  // Set the current preset
  logicalState.m_currentPresetBank = 3;
  presetBankService.init();

  // Bogus value
  presetBankService.handleEvent(makeMidiPresetValueChangeEvent(15));

  // Check logical state and event bus
  TEST_ASSERT_EQUAL(3, logicalState.m_currentPresetBank);
  TEST_ASSERT_EQUAL(3, logicalState.m_saveTargetBank);
  assertEventBusEmpty();
}

void test_preset_save_loads_different_bank() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  // Set the current preset bank
  logicalState.m_currentPresetBank = 3;
  presetBankService.init();

  // Set the target bank
  logicalState.m_saveTargetBank = 4;

  // Send the event
  presetBankService.handleEvent(makeUiSavePresetEvent());

  // Check logical state and event bus
  TEST_ASSERT_EQUAL(4, logicalState.m_currentPresetBank);
  TEST_ASSERT_EQUAL(4, logicalState.m_loadedPresetBank.m_id);

  assertPresetBankSaveEventPublished();
  assertEventBusEmpty();
}

void test_preset_save_bank_out_of_range_does_nothing() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  logicalState.m_currentPresetBank = 1;
  logicalState.m_saveTargetBank = PresetConstants::c_presetBankCount;
  presetBankService.init();

  presetBankService.handleEvent(makeUiSavePresetEvent());

  TEST_ASSERT_EQUAL(1, logicalState.m_currentPresetBank);
  assertEventBusEmpty();
}

void test_preset_save_wont_load_same_bank() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  // Set the current preset bank
  logicalState.m_currentPresetBank = 3;
  logicalState.m_saveTargetBank = 3;
  presetBankService.init();

  // Send the event
  presetBankService.handleEvent(makeUiSavePresetEvent());

  // Check logical state and event bus
  TEST_ASSERT_EQUAL(3, logicalState.m_currentPresetBank);
  TEST_ASSERT_EQUAL(3, logicalState.m_loadedPresetBank.m_id);

  assertEventBusEmpty();
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_ui_preset_bank_value_change() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(presetBankService.interestedIn(e));
}

void test_interested_in_ui_preset_save() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kSave;

  TEST_ASSERT_TRUE(presetBankService.interestedIn(e));
}

void test_interested_in_midi_preset_value_change() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(presetBankService.interestedIn(e));
}

void test_interested_in_ui_preset_setting_changed() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kSettingChanged;

  TEST_ASSERT_TRUE(presetBankService.interestedIn(e));
}

void test_not_interested_in_other_events() {
  LogicalState logicalState;
  MockEEPROM eeprom;
  PresetBankService presetBankService(logicalState, eeprom);

  Event e;

  // Not interested in physical switch events
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  TEST_ASSERT_FALSE(presetBankService.interestedIn(e));

  // Not interested in memory preset banks events (outputs them, doesn't consume)
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPresetBank;
  TEST_ASSERT_FALSE(presetBankService.interestedIn(e));

  // Not interested in other MIDI events
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(presetBankService.interestedIn(e));

  // Not interested in logic Expr events
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(presetBankService.interestedIn(e));

  // Not interested in logic program with different action
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kSave;
  TEST_ASSERT_FALSE(presetBankService.interestedIn(e));

  // Not interested in nonsensical event
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kPressed;
  TEST_ASSERT_FALSE(presetBankService.interestedIn(e));
}


int main() {
  UNITY_BEGIN();

  // Init Tests
  RUN_TEST(test_init_load_preset_bank_from_eeprom);
  RUN_TEST(test_init_syncs_logical_state);

  // Preset setting change tests
  RUN_TEST(test_ui_preset_setting_changed_sets_logical_state);
  RUN_TEST(test_ui_preset_setting_changed_clamps_at_max);
  RUN_TEST(test_ui_preset_setting_changed_clamps_at_min);
  RUN_TEST(test_ui_preset_setting_changed_unknown_id_does_nothing);

  // Preset bank loading tests
  RUN_TEST(test_midi_bank_loading_changes_logical_state);
  RUN_TEST(test_ui_bank_loading_changes_logical_state);
  RUN_TEST(test_ui_bank_loading_wraps_around);
  RUN_TEST(test_bank_loading_out_of_range);
  RUN_TEST(test_wont_load_same_midi_bank);
  RUN_TEST(test_preset_save_wont_load_same_bank);
  RUN_TEST(test_preset_save_loads_different_bank);
  RUN_TEST(test_preset_save_bank_out_of_range_does_nothing);

  // interestedIn Tests
  RUN_TEST(test_interested_in_midi_preset_value_change);
  RUN_TEST(test_interested_in_ui_preset_bank_value_change);
  RUN_TEST(test_interested_in_ui_preset_save);
  RUN_TEST(test_interested_in_ui_preset_setting_changed);
  RUN_TEST(test_not_interested_in_other_events);

  UNITY_END();
}
