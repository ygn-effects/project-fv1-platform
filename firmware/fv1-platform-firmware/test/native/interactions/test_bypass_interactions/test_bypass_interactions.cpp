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

void makeMidiCCBypassOnMessage(MidiHandler* t_handler) {
  // CC message bypass on
  t_handler->pushByte(0xB0);
  t_handler->pushByte(0x04);
  t_handler->pushByte(0x7F);
}

void makeMidiCCBypassOffMessage(MidiHandler* t_handler) {
  // CC message bypass off
  t_handler->pushByte(0xB0);
  t_handler->pushByte(0x04);
  t_handler->pushByte(0x00);
}

void setUp() {

}

void tearDown() {

}

// =============================================================================
// Physical bypass toggle tests
// =============================================================================

void test_driver_footswitch_pressed_event_toggles_bypass_on() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send footswitch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kBypass));

  // Test bypass and LogicalState
  TEST_ASSERT_EQUAL(1, fix.mockBypass.m_kState);
}

void test_driver_footswitch_pressed_event_toggles_bypass_off() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send footswitch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kBypass));

  // Test bypass and LogicalState
  TEST_ASSERT_EQUAL(0, fix.mockBypass.m_kState);
}

void test_driver_footswitch_pressed_event_sets_logical_state_active() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send footswitch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kBypass));

  // Test LogicalState
  TEST_ASSERT_EQUAL(BypassState::kActive, fix.logicalState.m_bypassState);
}

void test_driver_footswitch_pressed_event_sets_logical_state_bypassed() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send footswitch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kBypass));

  // Test LogicalState
  TEST_ASSERT_EQUAL(BypassState::kBypassed, fix.logicalState.m_bypassState);
}

void test_bypass_state_toggled_sets_fsm_program_mode_idle() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send footswitch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kBypass));

  // Test FSM state
  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fix.fsmService.getAppState());
}

void test_bypass_state_toggled_sets_fsm_preset_mode_idle() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send footswitch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kBypass));

  // Test FSM state
  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fix.fsmService.getAppState());
}

void test_bypass_state_toggled_sets_fsm_program_mode_bypassed() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send footswitch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kBypass));

  // Test FSM state
  TEST_ASSERT_EQUAL(AppState::kBypassed, fix.fsmService.getAppState());
}

void test_only_bypass_switch_republished_when_bypassed() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Send encoder switch
  fix.publishAndDispatchEvent(makeDriverSwitchPressedEvent(SwitchId::kMenuEncoder));
  // Event bus should be empty
  TEST_ASSERT_FALSE(fix.hasEvents());

  // Send program mode switch
  fix.publishAndDispatchEvent(makeDriverSwitchPressedEvent(SwitchId::kProgramMode));
  // Event bus should be empty
  TEST_ASSERT_FALSE(fix.hasEvents());

  // Send tap switch
  fix.publishAndDispatchEvent(makeDriverSwitchPressedEvent(SwitchId::kTap));
  // Event bus should be empty
  TEST_ASSERT_FALSE(fix.hasEvents());

  // Send menu lock switch
  fix.publishAndDispatchEvent(makeDriverSwitchPressedEvent(SwitchId::kMenuLock));
  // Event bus should be empty
  TEST_ASSERT_FALSE(fix.hasEvents());
}

// =============================================================================
// MIDI bypass control tests
// =============================================================================

void test_toggle_bypass_off_on_midi_cc_message() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send CC message
  makeMidiCCBypassOffMessage(fix.midiService.getMidiHandler());

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test bypass and LogicalState
  TEST_ASSERT_EQUAL(0, fix.mockBypass.m_kState);
  TEST_ASSERT_EQUAL(BypassState::kBypassed, fix.logicalState.m_bypassState);
}

void test_toggle_bypass_on_on_midi_cc_message() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send CC message
  makeMidiCCBypassOnMessage(fix.midiService.getMidiHandler());

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test bypass and LogicalState
  TEST_ASSERT_EQUAL(1, fix.mockBypass.m_kState);
  TEST_ASSERT_EQUAL(BypassState::kActive, fix.logicalState.m_bypassState);
}

void test_bypass_stays_off_on_midi_cc_off_message() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send CC message
  makeMidiCCBypassOffMessage(fix.midiService.getMidiHandler());

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test bypass and LogicalState
  TEST_ASSERT_EQUAL(0, fix.mockBypass.m_kState);
  TEST_ASSERT_EQUAL(BypassState::kBypassed, fix.logicalState.m_bypassState);
}

void test_bypass_stays_on_on_midi_cc_on_message() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send CC message
  makeMidiCCBypassOnMessage(fix.midiService.getMidiHandler());

  // Update services and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Test bypass and LogicalState
  TEST_ASSERT_EQUAL(1, fix.mockBypass.m_kState);
  TEST_ASSERT_EQUAL(BypassState::kActive, fix.logicalState.m_bypassState);
}

// =============================================================================
// Memory tests
// =============================================================================

void test_toggle_bypass_on_persists() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send footswitch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kBypass));

  // Reset EEPROM
  fix.init();

  // Test logicalstate
  TEST_ASSERT_EQUAL(BypassState::kActive, fix.logicalState.m_bypassState);
}

void test_toggle_bypass_off_persists() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());
  // Send footswitch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kBypass));

  // Reset EEPROM
  fix.init();

  // Test logicalstate
  TEST_ASSERT_EQUAL(BypassState::kBypassed, fix.logicalState.m_bypassState);
}

// =============================================================================
// Menu tests
// =============================================================================

void test_toggle_bypass_when_menu_unlocked_locks_menu() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Unlock menu
  fix.menuService.getMenuHandler()->m_mode = UiMode::kUnlocked;

  // Send footswitch event
  fix.publishAndDispatchAllEvents(makeDriverSwitchPressedEvent(SwitchId::kBypass));

  // Check handler
  TEST_ASSERT_EQUAL(UiMode::kLocked, fix.menuService.getMenuHandler()->m_mode);
}

// =============================================================================
// Event chain tests
// =============================================================================

void test_full_bypass_event_chain() {
  InteractionFixture fix;
  Event e1;
  Event e2;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Clear
  fix.clearEventBus();

  // Send footswitch event
  fix.publishAndDispatchEvent(makeDriverSwitchPressedEvent(SwitchId::kBypass));

  // First event should be FSM republishing the driver event
  TEST_ASSERT_TRUE(fix.hasEvents());
  fix.recallEvent(e1);
  TEST_ASSERT_EQUAL(EventDomain::kPhysical, e1.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kSwitch, e1.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kPressed, e1.m_action);
  TEST_ASSERT_EQUAL(static_cast<uint8_t>(SwitchId::kBypass), e1.m_id);

  // Event bus should be empty
  TEST_ASSERT_FALSE(fix.hasEvents());

  // Publish
  fix.publishAndDispatchEvent(e1);

  // Event should be bypass toggled
  TEST_ASSERT_TRUE(fix.hasEvents());
  fix.recallEvent(e1);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e1.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kBypass, e1.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kToggled, e1.m_action);

  // Event should be bypass save
  TEST_ASSERT_TRUE(fix.hasEvents());
  fix.recallEvent(e2);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e2.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kBypass, e2.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e2.m_action);

  // Publish
  fix.publishAndDispatchEvent(e1);
  fix.publishAndDispatchEvent(e2);

  // Event bus should be empty
  TEST_ASSERT_FALSE(fix.hasEvents());
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Physical bypass toggle tests
  RUN_TEST(test_driver_footswitch_pressed_event_toggles_bypass_on);
  RUN_TEST(test_driver_footswitch_pressed_event_toggles_bypass_off);
  RUN_TEST(test_driver_footswitch_pressed_event_sets_logical_state_active);
  RUN_TEST(test_driver_footswitch_pressed_event_sets_logical_state_bypassed);
  RUN_TEST(test_bypass_state_toggled_sets_fsm_program_mode_idle);
  RUN_TEST(test_bypass_state_toggled_sets_fsm_preset_mode_idle);
  RUN_TEST(test_bypass_state_toggled_sets_fsm_program_mode_bypassed);
  RUN_TEST(test_only_bypass_switch_republished_when_bypassed);

  // MIDI bypass control tests
  RUN_TEST(test_toggle_bypass_off_on_midi_cc_message);
  RUN_TEST(test_toggle_bypass_on_on_midi_cc_message);
  RUN_TEST(test_bypass_stays_on_on_midi_cc_on_message);
  RUN_TEST(test_bypass_stays_off_on_midi_cc_off_message);

  // Memory tests
  RUN_TEST(test_toggle_bypass_on_persists);
  RUN_TEST(test_toggle_bypass_off_persists);

  // Menu tests
  RUN_TEST(test_toggle_bypass_when_menu_unlocked_locks_menu);

  // Event chain tests
  RUN_TEST(test_full_bypass_event_chain);

  return UNITY_END();
}
