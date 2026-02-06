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

Event makeLogicBypassToggledEvent() {
  Event e{};
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kBypass;
  e.m_action = EventAction::kToggled;
  return e;
}

Event makeDriverSwitchPress(SwitchId t_id, uint32_t t_timestamp = 0) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kPressed;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_timestamp = t_timestamp;
  return e;
}

Event makeDriverSwitchLongPress(SwitchId t_id, uint32_t t_timestamp = 0) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kSwitch;
  e.m_action = EventAction::kLongPressed;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_timestamp = t_timestamp;
  return e;
}

Event makeDriverPotMove(PotId t_id, uint16_t t_value, uint32_t t_timestamp = 0) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_timestamp = t_timestamp;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.value = t_value;
  return e;
}

Event makeDriverExprMove(uint16_t t_value) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_value;
  return e;
}

Event makeDriverEncoderDelta(EncoderId t_id, int16_t t_delta) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kEncoder;
  e.m_action = EventAction::kDeltaChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.delta = t_delta;
  return e;
}

Event makeUIProgramValueChange(int8_t t_delta) {
  Event e{};
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_data.delta = t_delta;
  return e;
}

Event makeUIPresetValueChange(int8_t t_delta) {
  Event e{};
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;
  e.m_data.delta = t_delta;
  return e;
}

Event makeUIPresetBankValueChange(int8_t t_delta) {
  Event e{};
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPresetBank;
  e.m_action = EventAction::kValueChanged;
  e.m_data.delta = t_delta;
  return e;
}

Event makeMidiProgramValueChange(int8_t t_value) {
  Event e{};
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_value;
  return e;
}

Event makeMidiPresetValueChange(int8_t t_value) {
  Event e{};
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPreset;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_value;
  return e;
}

Event makeMidiPotValueChange(PotId t_id, uint8_t t_value) {
  Event e{};
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  e.m_data.value = t_value;
  return e;
}

void setUp() {

}

void tearDown() {

}

// =============================================================================
// Boot
// =============================================================================

void test_boot_lock_screen_program_mode_delay_effect_div_disabled_expr_inactive() {
  InteractionFixture fix;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Labels
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Lock screen"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("B"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("P"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Prog"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("D"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("T"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("P0"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("P1"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Mix"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("Expr"));
}

void test_boot_lock_screen_program_mode_delay_effect_div_enabled_expr_inactive() {
  InteractionFixture fix;
  fix.logicalState.m_interval = 400;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_divState = DivState::kEnabled;
  fix.logicalState.m_divValue = DivValue::kEight;
  fix.logicalState.m_divInterval = 200;
  fix.logicalState.m_tempo = 200;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Labels
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Lock screen"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("B"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("P"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Prog"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("D"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("T"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("P0"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("P1"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Mix"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("Expr"));

  // Values
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("T", "200 ms")); // Tempo
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("D", "/2")); // Div value
}

void test_boot_lock_screen_program_mode_delay_effect_div_disabled_expr_active() {
  InteractionFixture fix;
  fix.logicalState.m_exprParams[fix.logicalState.m_currentProgram].m_state = ExprState::kActive;
  fix.logicalState.m_exprParams[fix.logicalState.m_currentProgram].m_mappedPot = MappedPot::kPot1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Labels
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Lock screen"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("B"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("P"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Prog"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("D"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("T"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("P0"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("P1"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Mix"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Expr"));

  // values
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("Expr", "Pot1")); // Mapped pot
}

void test_boot_lock_screen_program_mode_not_delay_effect_div_disabled_expr_inactive() {
  InteractionFixture fix;
  fix.logicalState.m_currentProgram = 7;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Labels
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Lock screen"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("B"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("P"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Prog"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("D"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("T"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("P0"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("P1"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Mix"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("Expr"));
}

void test_boot_lock_screen_preset_mode_delay_effect_div_disabled_expr_inactive() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_programIndex = 1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Labels
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Lock screen"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("B"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("P"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Prog"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("D"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("T"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("P0"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("P1"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Mix"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("Expr"));

  // Values
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("B", "2")); // Bank
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("P", "1")); // Preset
}

// =============================================================================
// Menu State Transitions
// =============================================================================

void test_menu_unlock_menu_lock_switch_long_press_transition_to_program_menu_view() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Program mode"));
}

void test_menu_unlock_menu_lock_switch_long_press_transition_to_preset_menu_view() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Preset mode"));
}

void test_menu_lock_menu_lock_switch_long_press_transition_to_lock_screen() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Menu lock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Lock screen"));
}

void test_menu_lock_timeout_transition_to_lock_screen() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock, 100));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Advance the clock and update MenuService
  fix.mockClock.advanceBy(ui::MenuConstants::c_menuTimeout + 1);
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Lock screen"));
}

void test_menu_lock_bypass_toggle_transition_to_lock_screen() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Bypass toggle
  fix.publishAndDispatchAllEvents(makeLogicBypassToggledEvent());
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Lock screen"));
}

void test_menu_encoder_delta_changed_moves_cursor() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Cursor position
  TEST_ASSERT_TRUE(fix.mockDisplay.isCursorPointingTo("Prog"));

  // Reset the mock
  fix.mockDisplay.reset();

  // Menu encoder move
  fix.publishAndDispatchAllEvents(makeDriverEncoderDelta(EncoderId::kMenuEncoder, 1));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Cursor position
  TEST_ASSERT_TRUE(fix.mockDisplay.isCursorPointingTo("Tempo"));

  // Reset the mock
  fix.mockDisplay.reset();

  // Menu encoder move
  fix.publishAndDispatchAllEvents(makeDriverEncoderDelta(EncoderId::kMenuEncoder, -1));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Cursor position
  TEST_ASSERT_TRUE(fix.mockDisplay.isCursorPointingTo("Prog"));
}

void test_menu_encoder_press_on_submenu_transition_to_submenu() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Navigate to the submenu
  while (! fix.mockDisplay.isCursorPointingTo("Expression settings")) {
    fix.mockDisplay.reset();
    fix.publishAndDispatchAllEvents(makeDriverEncoderDelta(EncoderId::kMenuEncoder, 1));
    fix.updateAllServices();
  }

  // Reset the mock
  fix.mockDisplay.reset();

  // Enter submenu
  fix.publishAndDispatchAllEvents(makeDriverSwitchPress(SwitchId::kMenuEncoder));
  fix.updateAllServices();

  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Expression settings"));
  TEST_ASSERT_TRUE(fix.mockDisplay.isHighlighted("Expression settings"));
}

void test_menu_encoder_press_on_back_transition_to_previous_menu() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Navigate to the submenu
  while (! fix.mockDisplay.isCursorPointingTo("Expression settings")) {
    fix.mockDisplay.reset();
    fix.publishAndDispatchAllEvents(makeDriverEncoderDelta(EncoderId::kMenuEncoder, 1));
    fix.updateAllServices();
  }

  // Reset the mock
  fix.mockDisplay.reset();

  // Enter submenu
  fix.publishAndDispatchAllEvents(makeDriverSwitchPress(SwitchId::kMenuEncoder));
  fix.updateAllServices();

  // Navigate to the back button
  while (! fix.mockDisplay.isCursorPointingTo("Back")) {
    fix.mockDisplay.reset();
    fix.publishAndDispatchAllEvents(makeDriverEncoderDelta(EncoderId::kMenuEncoder, 1));
    fix.updateAllServices();
  }

  // Reset the mock
  fix.mockDisplay.reset();

  // Enter submenu
  fix.publishAndDispatchAllEvents(makeDriverSwitchPress(SwitchId::kMenuEncoder));
  fix.updateAllServices();

  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Program mode"));
}

void test_menu_encoder_press_on_editable_items_highlight_value() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Begin editing
  fix.publishAndDispatchAllEvents(makeDriverSwitchPress(SwitchId::kMenuEncoder));
  fix.updateAllServices();

  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Digital delay"));
  TEST_ASSERT_TRUE(fix.mockDisplay.isHighlighted("Digital delay"));
}

void test_menu_encoder_press_on_hilighted_item_exit_edit() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Begin editing
  fix.publishAndDispatchAllEvents(makeDriverSwitchPress(SwitchId::kMenuEncoder));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Exit editing
  fix.publishAndDispatchAllEvents(makeDriverSwitchPress(SwitchId::kMenuEncoder));
  fix.updateAllServices();

  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Digital delay"));
  TEST_ASSERT_FALSE(fix.mockDisplay.isHighlighted("Digital delay"));
}

// =============================================================================
// Overlay Display
// =============================================================================

void test_pot_move_shows_pot_overlay_menu_unlocked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Pot move
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot1, 512));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Header
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[1].m_label));
}

void test_pot_move_not_shows_pot_overlay_menu_locked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Pot move
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot1, 512));
  fix.updateAllServices();

  // Display should be empty
  TEST_ASSERT_TRUE(fix.mockDisplay.isCleared());
}

void test_pot_overlay_timeout_pop_overlay() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Pot move
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot1, 512));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Set the clock
  fix.mockClock.advanceBy(ui::MenuConstants::c_potMenuTimeout + 1);

  // update and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Program mode"));
}

void test_pot0_move_shows_tempo_overlay_delay_effect_menu_unlocked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Pot move
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot0, 512));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Header
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Tempo"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("Program mode"));
}

void test_pot0_move_shows_pot_overlay_non_delay_effect_menu_unlocked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_currentProgram = 7;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Pot move
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot0, 512));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Header
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[7].m_params[0].m_label));
}

void test_tempo_overlay_timeout_pop_overlay() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Pot move
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot0, 512));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Set the clock
  fix.mockClock.advanceBy(ui::MenuConstants::c_potMenuTimeout + 1);

  // update and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Program mode"));
}

void test_successive_pot_move_pushes_pops_overlays() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Pot move
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot0, 512, 0));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Header
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Tempo"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("Program mode"));

  // Advance clock and update
  fix.mockClock.advanceBy(ui::MenuConstants::c_potMenuTimeout - 100);
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Shouldn't have changed
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Tempo"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("Program mode"));

  // Reset the mock
  fix.mockDisplay.reset();

  // Pot move
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot1, 512, 399));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Header
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[1].m_label));

  // Advance clock and update
  fix.mockClock.advanceBy(ui::MenuConstants::c_potMenuTimeout - 100);
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Shouldn't have changed
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[1].m_label));

  // Reset the mock
  fix.mockDisplay.reset();

  // Pot move
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot2, 512, 799));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Header
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[2].m_label));

  // Advance clock and update
  fix.mockClock.advanceBy(ui::MenuConstants::c_potMenuTimeout - 100);
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Shouldn't have changed
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[2].m_label));

  // Reset the mock
  fix.mockDisplay.reset();

  // Pot move
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kMixPot, 512,  1199));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Header
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[3].m_label));

  // Advance clock and update
  fix.mockClock.advanceBy(ui::MenuConstants::c_potMenuTimeout - 100);
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Shouldn't have changed
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[3].m_label));

  // Reset the mock
  fix.mockDisplay.reset();

  // Set the clock
  fix.mockClock.advanceBy(ui::MenuConstants::c_potMenuTimeout + 1);

  // update and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Program mode"));
}

// =============================================================================
// Overlay Cross-Transitions
// =============================================================================

void test_tempo_overlay_active_pot_move_replaces_with_pot_overlay() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Pot0 move on delay effect → tempo overlay
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot0, 512));
  fix.updateAllServices();

  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Tempo"));

  // Reset the mock
  fix.mockDisplay.reset();

  // Pot1 move → should pop tempo overlay, push pot overlay
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot1, 256));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Should show pot1 overlay, not tempo
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[1].m_label));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("Tempo"));
}

void test_pot_overlay_active_tempo_change_replaces_with_tempo_overlay() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_interval = 400;
  fix.logicalState.m_tempo = 400;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Pot1 move → pot overlay
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot1, 512));
  fix.updateAllServices();

  TEST_ASSERT_TRUE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[1].m_label));

  // Reset the mock
  fix.mockDisplay.reset();

  // Tap long press → div change → tempo overlay should replace pot overlay
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kTap));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Should show tempo overlay, not pot
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Tempo"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[1].m_label));
}

void test_pot_overlay_active_preset_save_replaces_with_save_overlay() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Pot1 move → pot overlay
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot1, 512));
  fix.updateAllServices();

  TEST_ASSERT_TRUE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[1].m_label));

  // Reset the mock
  fix.mockDisplay.reset();

  // Encoder long press → preset save should replace pot overlay
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuEncoder));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Should show preset save overlay, not pot
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Preset save"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[1].m_label));
}

void test_tempo_overlay_active_preset_save_replaces_with_save_overlay() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Pot0 move on delay effect → tempo overlay
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot0, 512));
  fix.updateAllServices();

  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Tempo"));

  // Reset the mock
  fix.mockDisplay.reset();

  // Encoder long press → preset save should replace tempo overlay
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuEncoder));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Should show preset save overlay, not tempo
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Preset save"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsText("Tempo"));
}

void test_tempo_overlay_active_pot_move_timeout_returns_to_menu() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Pot0 move on delay effect → tempo overlay
  fix.mockClock.setClock(1000);
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot0, 512, 1000));
  fix.updateAllServices();

  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Tempo"));

  // Pot1 move → replaces tempo with pot overlay
  fix.mockClock.setClock(1200);
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot1, 256, 1200));
  fix.updateAllServices();

  TEST_ASSERT_TRUE(fix.mockDisplay.showsText(ProgramsDefinitions::kPrograms[0].m_params[1].m_label));

  // Reset the mock
  fix.mockDisplay.reset();

  // Timeout pot overlay → should return to menu, not tempo overlay
  fix.mockClock.setClock(1200 + ui::MenuConstants::c_potMenuTimeout + 1);
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Should show menu, not an overlay
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Program mode"));
}

// =============================================================================
// Preset Saving
// =============================================================================

void test_menu_encoder_long_press_show_preset_saving_menu_unlocked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Pot move
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuEncoder));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Header
  TEST_ASSERT_TRUE(fix.mockDisplay.showsText("Preset save"));
}

// =============================================================================
// Value Change Display Updates
// =============================================================================

void test_program_value_change_update_display_unlocked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Send the event
  fix.publishAndDispatchAllEvents(makeUIProgramValueChange(1));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("Prog", ProgramsDefinitions::kPrograms[1].m_name));
}

void test_program_value_change_update_display_locked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Send the event
  fix.publishAndDispatchAllEvents(makeMidiProgramValueChange(1));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("Prog", ProgramsDefinitions::kPrograms[1].m_name));
}

void test_preset_value_change_update_display_unlocked() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_programIndex = 1;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.SyncEepromWithLoadedPresetBank();
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Send the event
  fix.publishAndDispatchAllEvents(makeUIPresetValueChange(1));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("Preset", "1"));
}

void test_preset_value_change_update_display_locked() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_programIndex = 1;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.SyncEepromWithLoadedPresetBank();
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Send the event
  fix.publishAndDispatchAllEvents(makeMidiPresetValueChange(1));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("P", "1"));
}

void test_preset_bank_value_change_update_display_unlocked() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Send the event
  fix.publishAndDispatchAllEvents(makeUIPresetBankValueChange(1));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("Bank", "1"));
}

void test_preset_bank_value_change_update_display_locked() {
  InteractionFixture fix;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Send the event
  fix.publishAndDispatchAllEvents(makeMidiPresetValueChange(4));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("B", "1"));
}

void test_tempo_value_change_updates_display_unlocked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Pot move
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kPot0, 512));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Set the clock
  fix.mockClock.advanceBy(ui::MenuConstants::c_tempoMenuTimeout + 1);

  // update and handle events
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("Tempo", "510 ms"));
}

void test_tempo_value_change_updates_display_locked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Pot move
  fix.publishAndDispatchAllEvents(makeMidiPotValueChange(PotId::kPot0, 64));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("T", "513 ms"));
}

void test_tap_enabled_div_disabled_tap_long_press_updates_display_unlocked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_interval = 400;
  fix.logicalState.m_tempo = 400;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Send the event
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kTap));

  // Reset the mock
  fix.mockDisplay.reset();

  // Set the clock
  fix.mockClock.advanceBy(ui::MenuConstants::c_tempoMenuTimeout + 1);

  // update and handle events to clear the overlay
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("Tempo", "200 ms"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("Div", "/2"));
}

void test_tap_enabled_div_disabled_tap_long_press_updates_display_locked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_interval = 400;
  fix.logicalState.m_tempo = 400;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Send the event
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kTap));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("T", "200 ms"));
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("D", "/2"));
}

void test_tap_enabled_div_enabled_tap_long_press_updates_display_unlocked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_divState = DivState::kEnabled;
  fix.logicalState.m_divValue = DivValue::kEightTriplet;
  fix.logicalState.m_interval = 300;
  fix.logicalState.m_divInterval = 100;
  fix.logicalState.m_tempo = 100;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Send the event
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kTap));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Set the clock
  fix.mockClock.advanceBy(ui::MenuConstants::c_tempoMenuTimeout + 1);

  // update and handle events to clear the overlay
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("Tempo", "300 ms"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsLabelValue("Div", "/3"));
}

void test_tap_enabled_div_enabled_tap_long_press_updates_display_locked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_tapState = TapState::kEnabled;
  fix.logicalState.m_divState = DivState::kEnabled;
  fix.logicalState.m_divValue = DivValue::kEightTriplet;
  fix.logicalState.m_interval = 300;
  fix.logicalState.m_divInterval = 100;
  fix.logicalState.m_tempo = 100;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Send the event
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kTap));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("T", "300 ms"));
  TEST_ASSERT_FALSE(fix.mockDisplay.showsLabelValue("D", "/3"));
}

void test_mix_pot_moved_updates_display_unlocked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Send the event
  fix.publishAndDispatchAllEvents(makeDriverPotMove(PotId::kMixPot, 512));

  // Reset the mock
  fix.mockDisplay.reset();

  // Set the clock
  fix.mockClock.advanceBy(ui::MenuConstants::c_potMenuTimeout + 1);

  // update and handle events to clear the overlay
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("Mix", "50 %"));
}

void test_expr_moved_updates_mapped_pot_display_unlocked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_exprParams[0].m_state = ExprState::kActive;
  fix.logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Menu unlock
  fix.publishAndDispatchAllEvents(makeDriverSwitchLongPress(SwitchId::kMenuLock));
  fix.updateAllServices();

  // Send the event
  fix.publishAndDispatchAllEvents(makeDriverExprMove(512));
  fix.updateAllServices();

  // Reset the mock
  fix.mockDisplay.reset();

  // Set the clock
  fix.mockClock.advanceBy(ui::MenuConstants::c_potMenuTimeout + 1);

  // update and handle events to clear the overlay
  fix.updateAllServices();
  fix.dispatchAllEvents();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("Feedback", "50 %"));
}

void test_expr_moved_updates_mapped_pot_display_locked() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_exprParams[0].m_state = ExprState::kActive;
  fix.logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot1;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Reset the mock
  fix.mockDisplay.reset();

  // Send the event
  fix.publishAndDispatchAllEvents(makeDriverExprMove(512));
  fix.updateAllServices();

  // Commands
  TEST_ASSERT_TRUE(fix.mockDisplay.hasClearCmd());
  TEST_ASSERT_TRUE(fix.mockDisplay.hasDisplayCmd());

  // Label value
  TEST_ASSERT_TRUE(fix.mockDisplay.showsLabelValue("P1", "50 %"));
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Boot tests
  RUN_TEST(test_boot_lock_screen_program_mode_delay_effect_div_disabled_expr_inactive);
  RUN_TEST(test_boot_lock_screen_program_mode_delay_effect_div_enabled_expr_inactive);
  RUN_TEST(test_boot_lock_screen_program_mode_delay_effect_div_disabled_expr_active);
  RUN_TEST(test_boot_lock_screen_program_mode_not_delay_effect_div_disabled_expr_inactive);
  RUN_TEST(test_boot_lock_screen_preset_mode_delay_effect_div_disabled_expr_inactive);

  // Menu State Transitions
  RUN_TEST(test_menu_unlock_menu_lock_switch_long_press_transition_to_program_menu_view);
  RUN_TEST(test_menu_unlock_menu_lock_switch_long_press_transition_to_preset_menu_view);
  RUN_TEST(test_menu_lock_menu_lock_switch_long_press_transition_to_lock_screen);
  RUN_TEST(test_menu_lock_timeout_transition_to_lock_screen);
  RUN_TEST(test_menu_lock_bypass_toggle_transition_to_lock_screen);
  RUN_TEST(test_menu_encoder_delta_changed_moves_cursor);
  RUN_TEST(test_menu_encoder_press_on_submenu_transition_to_submenu);
  RUN_TEST(test_menu_encoder_press_on_back_transition_to_previous_menu);
  RUN_TEST(test_menu_encoder_press_on_editable_items_highlight_value);
  RUN_TEST(test_menu_encoder_press_on_hilighted_item_exit_edit);

  // Overlay Display
  RUN_TEST(test_pot_move_shows_pot_overlay_menu_unlocked);
  RUN_TEST(test_pot_move_not_shows_pot_overlay_menu_locked);
  RUN_TEST(test_pot_overlay_timeout_pop_overlay);
  RUN_TEST(test_pot0_move_shows_tempo_overlay_delay_effect_menu_unlocked);
  RUN_TEST(test_pot0_move_shows_pot_overlay_non_delay_effect_menu_unlocked);
  RUN_TEST(test_pot_overlay_timeout_pop_overlay);
  RUN_TEST(test_successive_pot_move_pushes_pops_overlays);

  // Overlay Cross-Transitions
  RUN_TEST(test_tempo_overlay_active_pot_move_replaces_with_pot_overlay);
  RUN_TEST(test_pot_overlay_active_tempo_change_replaces_with_tempo_overlay);
  RUN_TEST(test_pot_overlay_active_preset_save_replaces_with_save_overlay);
  RUN_TEST(test_tempo_overlay_active_preset_save_replaces_with_save_overlay);
  RUN_TEST(test_tempo_overlay_active_pot_move_timeout_returns_to_menu);

  // Preset Saving
  RUN_TEST(test_menu_encoder_long_press_show_preset_saving_menu_unlocked);

  // Value Change Display Updates
  RUN_TEST(test_program_value_change_update_display_unlocked);
  RUN_TEST(test_program_value_change_update_display_locked);
  RUN_TEST(test_preset_value_change_update_display_locked);
  RUN_TEST(test_preset_value_change_update_display_unlocked);
  RUN_TEST(test_preset_bank_value_change_update_display_locked);
  RUN_TEST(test_preset_bank_value_change_update_display_unlocked);
  RUN_TEST(test_tempo_value_change_updates_display_unlocked);
  RUN_TEST(test_tempo_value_change_updates_display_locked);
  RUN_TEST(test_tap_enabled_div_disabled_tap_long_press_updates_display_locked);
  RUN_TEST(test_tap_enabled_div_disabled_tap_long_press_updates_display_unlocked);
  RUN_TEST(test_tap_enabled_div_enabled_tap_long_press_updates_display_unlocked);
  RUN_TEST(test_tap_enabled_div_enabled_tap_long_press_updates_display_locked);
  RUN_TEST(test_mix_pot_moved_updates_display_unlocked);
  RUN_TEST(test_expr_moved_updates_mapped_pot_display_unlocked);
  RUN_TEST(test_expr_moved_updates_mapped_pot_display_locked);

  return UNITY_END();
}
