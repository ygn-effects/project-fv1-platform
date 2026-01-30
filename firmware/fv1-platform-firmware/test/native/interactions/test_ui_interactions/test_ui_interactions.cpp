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

Event makeDriverPotMove(PotId t_id, uint16_t t_value) {
  Event e{};
  e.m_domain = EventDomain::kDriver;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
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

  return UNITY_END();
}
