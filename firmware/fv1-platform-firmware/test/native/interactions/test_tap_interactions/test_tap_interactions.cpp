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

Event makeDriverSwitchPressedEvent(SwitchId t_id, uint32_t t_timestamp) {
  Event e{};
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;
  e.m_timestamp = t_timestamp;
  e.m_id = static_cast<uint8_t>(t_id);
  return e;
}

Event makeDriverSwitchLongPressedEvent(SwitchId t_id, uint32_t t_timestamp) {
  Event e{};
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kLongPressed;
  e.m_timestamp = t_timestamp;
  e.m_id = static_cast<uint8_t>(t_id);
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

void makeMidiCCTapShortPressMessage(MockedSerial& t_serial) {
  t_serial.feedByte(0xB0);
  t_serial.feedByte(0x05);
  t_serial.feedByte(0x00);
}

void makeMidiCCTapLongPressMessage(MockedSerial& t_serial) {
  t_serial.feedByte(0xB0);
  t_serial.feedByte(0x05);
  t_serial.feedByte(0x7F);
}

void setUp() {

}

void tearDown() {

}

// =============================================================================
// Physical Tap Press
// =============================================================================

void test_driver_tap_switch_two_taps_set_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send first tap
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kTap, 200));
  // Send second tap
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kTap, 400));

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(200, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, fix.logicalState.m_divValue);
}

void test_driver_tap_switch_two_taps_persist() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send first tap
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kTap, 200));
  // Send second tap
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kTap, 400));

  // Reset EEPROM
  fix.init();

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(200, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, fix.logicalState.m_divValue);
}

// =============================================================================
// Physical Long Press
// =============================================================================

void test_driver_tap_switch_long_press_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_divState = DivState::kEnabled;
  fix.logicalState.m_divValue = DivValue::kQuarter;
  fix.logicalState.m_interval = 400;
  fix.logicalState.m_divInterval = 0;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Long press
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kTap, 200));

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(200, fix.logicalState.m_divInterval);
}

void test_driver_tap_switch_long_press_cycles_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_divState = DivState::kEnabled;
  fix.logicalState.m_divValue = DivValue::kQuarter;
  fix.logicalState.m_interval = 400;
  fix.logicalState.m_divInterval = 0;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Long press
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kTap, 200));

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(200, fix.logicalState.m_divInterval);

  // Long press
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kTap, 400));

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kSixteenth, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(100, fix.logicalState.m_divInterval);

  // Long press
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kTap, 400));

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kDottedEight, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(300, fix.logicalState.m_divInterval);

  // Long press
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kTap, 600));

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEightTriplet, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(133, fix.logicalState.m_divInterval);

  // Long press
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kTap, 600));

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_divInterval);
}

void test_driver_tap_switch_long_press_persists() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_divState = DivState::kEnabled;
  fix.logicalState.m_divValue = DivValue::kQuarter;
  fix.logicalState.m_interval = 400;
  fix.logicalState.m_divInterval = 0;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Long press
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kTap, 200));

  // Reset EEPROM
  fix.init();

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(200, fix.logicalState.m_divInterval);
}

// =============================================================================
// MIDI Tap
// =============================================================================

void test_midi_cc_tap_short_press_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Set clock
  fix.mockClock.advanceBy(200);
  // Send CC message
  makeMidiCCTapShortPressMessage(fix.mockSerial);

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Set clock
  fix.mockClock.advanceBy(200);
  // Send CC message
  makeMidiCCTapShortPressMessage(fix.mockSerial);

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(200, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, fix.logicalState.m_divValue);
}

void test_midi_cc_tap_long_press_sets_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_divState = DivState::kDisabled;
  fix.logicalState.m_divValue = DivValue::kQuarter;
  fix.logicalState.m_interval = 400;
  fix.logicalState.m_divInterval = 0;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send CC message
  makeMidiCCTapLongPressMessage(fix.mockSerial);

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(200, fix.logicalState.m_divInterval);
}

void test_midi_cc_tap_long_press_cycles_logical_state() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_divState = DivState::kDisabled;
  fix.logicalState.m_divValue = DivValue::kQuarter;
  fix.logicalState.m_interval = 400;
  fix.logicalState.m_divInterval = 0;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send CC message
  makeMidiCCTapLongPressMessage(fix.mockSerial);

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(200, fix.logicalState.m_divInterval);

  // Send CC message
  makeMidiCCTapLongPressMessage(fix.mockSerial);

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kSixteenth, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(100, fix.logicalState.m_divInterval);

  // Send CC message
  makeMidiCCTapLongPressMessage(fix.mockSerial);

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kDottedEight, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(300, fix.logicalState.m_divInterval);

  // Send CC message
  makeMidiCCTapLongPressMessage(fix.mockSerial);

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEightTriplet, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(133, fix.logicalState.m_divInterval);

  // Send CC message
  makeMidiCCTapLongPressMessage(fix.mockSerial);

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(400, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_divInterval);
}

// =============================================================================
// Disabling Tap
// =============================================================================

void test_driver_pot0_value_changed_disables_tap() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_divState = DivState::kEnabled;
  fix.logicalState.m_divValue = DivValue::kEight;
  fix.logicalState.m_interval = 400;
  fix.logicalState.m_divInterval = 200;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // POT0 move
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kPot0, 500));

  // Check logical state
  TEST_ASSERT_EQUAL(TapState::kDisabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, fix.logicalState.m_divValue);
}

void test_ui_tempo_value_changed_disables_tap() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_divState = DivState::kEnabled;
  fix.logicalState.m_divValue = DivValue::kEight;
  fix.logicalState.m_interval = 400;
  fix.logicalState.m_divInterval = 200;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // UI tempo change
  fix.publishAndDispatchAllEvents(makeUITempoValueChangedEvent(500));

  // Check logical state
  TEST_ASSERT_EQUAL(TapState::kDisabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, fix.logicalState.m_divValue);
}

void test_logical_program_value_changed_non_delay_disables_tap() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_divState = DivState::kEnabled;
  fix.logicalState.m_divValue = DivValue::kEight;
  fix.logicalState.m_interval = 400;
  fix.logicalState.m_divInterval = 200;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Set program and send event
  fix.publishAndDispatchAllEvents(makeUIProgramValueChanged(6));

  // Check logical state
  TEST_ASSERT_EQUAL(TapState::kDisabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, fix.logicalState.m_divValue);
}

// =============================================================================
// Program Compatibility
// =============================================================================

void test_driver_tap_switch_ignored_when_non_delay_effect() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_currentProgram = 7;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send first tap
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kTap, 200));
  // Send second tap
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kTap, 400));

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kDisabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kDisabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, fix.logicalState.m_divValue);
}

// =============================================================================
// Persistence
// =============================================================================

void test_tap_persist() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send first tap
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kTap, 200));
  // Send second tap
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kTap, 400));
  // Long press
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPressedEvent(SwitchId::kTap, 600));

  // Reset
  fix.init();

  // Test logical state
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(200, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(DivState::kEnabled, fix.logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, fix.logicalState.m_divValue);
  TEST_ASSERT_EQUAL(100, fix.logicalState.m_divInterval);
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Physical Tap Press
  RUN_TEST(test_driver_tap_switch_two_taps_set_logical_state);
  RUN_TEST(test_driver_tap_switch_two_taps_persist);

  // Physical Long Press
  RUN_TEST(test_driver_tap_switch_long_press_sets_logical_state);
  RUN_TEST(test_driver_tap_switch_long_press_cycles_logical_state);
  RUN_TEST(test_driver_tap_switch_long_press_persists);

  // MIDI Tap
  RUN_TEST(test_midi_cc_tap_short_press_sets_logical_state);
  RUN_TEST(test_midi_cc_tap_long_press_sets_logical_state);
  RUN_TEST(test_midi_cc_tap_long_press_cycles_logical_state);

  // Disabling Tap
  RUN_TEST(test_driver_pot0_value_changed_disables_tap);
  RUN_TEST(test_ui_tempo_value_changed_disables_tap);
  RUN_TEST(test_logical_program_value_changed_non_delay_disables_tap);

  // Program Compatibility
  RUN_TEST(test_driver_tap_switch_ignored_when_non_delay_effect);

  // Persistence
  RUN_TEST(test_tap_persist);

  return UNITY_END();
}
