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

void setUp() {

}

void tearDown() {

}

// =============================================================================
// Init Sequence Tests
// =============================================================================

void test_init_fsm_starts_in_restore_state() {
  InteractionFixture fix;
  fix.init();

  TEST_ASSERT_EQUAL(AppState::kRestoreState, fix.fsmService.getAppState());
}

void test_init_initializes_eeprom() {
  InteractionFixture fix;
  fix.init();

  TEST_ASSERT_TRUE(fix.mockEeprom.initialized);
}

void test_init_syncs_bypass_relay_when_active() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  // Bypass relay should be on when active
  TEST_ASSERT_EQUAL(1, fix.mockBypass.m_kState);
}

void test_init_syncs_bypass_relay_when_bypassed() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();

  // Bypass relay should be off when bypassed
  TEST_ASSERT_EQUAL(0, fix.mockBypass.m_kState);
}

void test_init_sends_program_to_fv1() {
  InteractionFixture fix;
  fix.logicalState.m_currentProgram = 3;
  fix.syncEepromWithState();
  fix.init();

  // FV1 should receive program change
  // Program 3 = binary 011 -> S0=1, S1=1, S2=0
  TEST_ASSERT_EQUAL(1, fix.mockFv1.m_s0);
  TEST_ASSERT_EQUAL(1, fix.mockFv1.m_s1);
  TEST_ASSERT_EQUAL(0, fix.mockFv1.m_s2);
}

void test_init_sends_pot_values_to_fv1() {
  InteractionFixture fix;
  fix.logicalState.m_potParams[0][0].m_value = 512;
  fix.logicalState.m_potParams[0][1].m_value = 256;
  fix.logicalState.m_potParams[0][2].m_value = 768;
  fix.init();

  // FV1 should receive pot values (at least 3 for Pot0, Pot1, Pot2)
  TEST_ASSERT_TRUE(fix.mockFv1.m_potValues.size() >= 3);
}

void test_init_publish_menu_updated_event() {
  InteractionFixture fix;
  fix.init();

  TEST_ASSERT_TRUE(fix.findEvent(EventDomain::kUI, EventSubject::kMenu, EventAction::kUpdated));
  TEST_ASSERT_FALSE(fix.hasEvents());
}

void test_init_sets_active_program_pointer() {
  InteractionFixture fix;
  fix.logicalState.m_currentProgram = 1;
  fix.syncEepromWithState();
  fix.init();

  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[1], fix.logicalState.m_activeProgram);
}

void test_init_loads_preset_bank() {
  InteractionFixture fix;
  fix.logicalState.m_currentPresetBank = 2;
  fix.syncEepromWithState();
  fix.init();

  TEST_ASSERT_EQUAL(2, fix.logicalState.m_loadedPresetBank.m_id);
}

void test_init_preset_mode_applies_preset() {
  InteractionFixture fix;

  // Set preset mode and preset/bank
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.logicalState.m_currentPresetBank = 2;
  fix.logicalState.m_currentPreset = 1;

  // Set some preset values
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_programIndex = 1;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_tapState = TapState::kEnabled;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_interval = 300;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_tempo = 300;

  // Persist
  fix.syncEepromWithState();
  fix.SyncEepromWithLoadedPresetBank();

  // Reset and check
  fix.init();

  TEST_ASSERT_EQUAL(ProgramMode::kPreset, fix.logicalState.m_programMode);
  TEST_ASSERT_EQUAL(2, fix.logicalState.m_currentPresetBank);
  TEST_ASSERT_EQUAL(1, fix.logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(1, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(TapState::kEnabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(300, fix.logicalState.m_interval);
  TEST_ASSERT_EQUAL(300, fix.logicalState.m_tempo);
}

void test_init_program_mode_does_not_apply_preset() {
  InteractionFixture fix;

  // Set program mode (not preset mode)
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.logicalState.m_currentProgram = 0;
  fix.logicalState.m_currentPreset = 1;
  fix.logicalState.m_tapState = TapState::kDisabled;
  fix.logicalState.m_tempo = 500;

  // Set preset with different values that should NOT be applied
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_programIndex = 3;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_tapState = TapState::kEnabled;
  fix.logicalState.m_loadedPresetBank.m_presets[1].m_tempo = 200;

  fix.syncEepromWithState();
  fix.SyncEepromWithLoadedPresetBank();
  fix.init();

  // Original state should be preserved, preset values should NOT be applied
  TEST_ASSERT_EQUAL(0, fix.logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(TapState::kDisabled, fix.logicalState.m_tapState);
  TEST_ASSERT_EQUAL(500, fix.logicalState.m_tempo);
}

void test_init_delay_effect_sends_tempo_to_pot0() {
  InteractionFixture fix;

  // Program 0 is Digital delay (isDelayEffect = true)
  fix.logicalState.m_currentProgram = 0;
  fix.logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  fix.logicalState.m_tempo = 500;
  fix.logicalState.m_potParams[0][0].m_value = 100; // Different from tempo

  fix.syncEepromWithState();
  fix.init();

  // Find the Pot0 value sent to FV1
  uint16_t pot0Value = 0;
  for (const auto& [pot, value] : fix.mockFv1.m_potValues) {
    if (pot == Fv1Pot::Pot0) {
      pot0Value = value;
      break;
    }
  }

  // Pot0 should NOT be the raw pot value (100), it should be tempo-mapped
  TEST_ASSERT_NOT_EQUAL(100, pot0Value);
}

void test_init_non_delay_effect_sends_pot_value_to_pot0() {
  InteractionFixture fix;

  // Program 5 is Plate reverb (isDelayEffect = false)
  fix.logicalState.m_currentProgram = 5;
  fix.logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[5];
  fix.logicalState.m_tempo = 500;
  fix.logicalState.m_potParams[5][0].m_value = 777;

  fix.syncEepromWithState();
  fix.init();

  // Find the Pot0 value sent to FV1
  uint16_t pot0Value = 0;
  for (const auto& [pot, value] : fix.mockFv1.m_potValues) {
    if (pot == Fv1Pot::Pot0) {
      pot0Value = value;
      break;
    }
  }

  // Pot0 should be the raw pot value, not tempo-mapped
  TEST_ASSERT_EQUAL(777, pot0Value);
}

void test_init_menu_starts_locked() {
  InteractionFixture fix;
  fix.init();

  // Menu should not publish an unlock event on init - it starts locked
  TEST_ASSERT_FALSE(fix.findEvent(EventDomain::kUI, EventSubject::kMenu, EventAction::kUnlocked));
}

// =============================================================================
// Boot Complete - FSM State Transition Tests
// =============================================================================

void test_boot_complete_enters_program_idle_when_active_program_mode() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();

  fix.publishAndDispatchAllEvents(makeBootEvent());

  TEST_ASSERT_EQUAL(AppState::kProgramIdle, fix.fsmService.getAppState());
}

void test_boot_complete_enters_preset_idle_when_active_preset_mode() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();

  fix.publishAndDispatchAllEvents(makeBootEvent());

  TEST_ASSERT_EQUAL(AppState::kPresetIdle, fix.fsmService.getAppState());
}

void test_boot_complete_enters_bypassed_when_bypass_state_is_bypassed() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();

  fix.publishAndDispatchAllEvents(makeBootEvent());

  TEST_ASSERT_EQUAL(AppState::kBypassed, fix.fsmService.getAppState());
}

void test_boot_complete_bypassed_takes_precedence_over_preset_mode() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.logicalState.m_programMode = ProgramMode::kPreset;
  fix.syncEepromWithState();
  fix.init();

  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Bypassed takes precedence over program mode
  TEST_ASSERT_EQUAL(AppState::kBypassed, fix.fsmService.getAppState());
}

// =============================================================================
// Boot Complete - Hardware State Verification
// =============================================================================

void test_boot_complete_bypass_relay_on_when_active() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.syncEepromWithState();
  fix.init();

  fix.publishAndDispatchAllEvents(makeBootEvent());

  TEST_ASSERT_EQUAL(1, fix.mockBypass.m_kState);
}

void test_boot_complete_bypass_relay_off_when_bypassed() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kBypassed;
  fix.syncEepromWithState();
  fix.init();

  fix.publishAndDispatchAllEvents(makeBootEvent());

  TEST_ASSERT_EQUAL(0, fix.mockBypass.m_kState);
}

void test_boot_complete_fv1_has_correct_program() {
  InteractionFixture fix;
  fix.logicalState.m_currentProgram = 5;
  fix.syncEepromWithState();
  fix.init();

  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Program 5 = binary 101 -> S0=1, S1=0, S2=1
  TEST_ASSERT_EQUAL(1, fix.mockFv1.m_s0);
  TEST_ASSERT_EQUAL(0, fix.mockFv1.m_s1);
  TEST_ASSERT_EQUAL(1, fix.mockFv1.m_s2);
}

// =============================================================================
// Boot Complete - Event Chain Verification
// =============================================================================

void test_boot_complete_event_bus_empty_after_full_dispatch() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_programMode = ProgramMode::kProgram;
  fix.syncEepromWithState();
  fix.init();
  fix.clearEventBus();

  fix.publishAndDispatchAllEvents(makeBootEvent());

  // All events should be fully processed
  TEST_ASSERT_FALSE(fix.hasEvents());
}

// =============================================================================
// Boot Complete - Display State
// =============================================================================

void test_boot_complete_display_initialized() {
  InteractionFixture fix;
  fix.init();

  TEST_ASSERT_TRUE(fix.mockDisplay.m_initialized);
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Init sequence tests
  RUN_TEST(test_init_fsm_starts_in_restore_state);
  RUN_TEST(test_init_initializes_eeprom);
  RUN_TEST(test_init_syncs_bypass_relay_when_active);
  RUN_TEST(test_init_syncs_bypass_relay_when_bypassed);
  RUN_TEST(test_init_sends_program_to_fv1);
  RUN_TEST(test_init_sends_pot_values_to_fv1);
  RUN_TEST(test_init_publish_menu_updated_event);
  RUN_TEST(test_init_sets_active_program_pointer);
  RUN_TEST(test_init_loads_preset_bank);
  RUN_TEST(test_init_preset_mode_applies_preset);
  RUN_TEST(test_init_program_mode_does_not_apply_preset);
  RUN_TEST(test_init_delay_effect_sends_tempo_to_pot0);
  RUN_TEST(test_init_non_delay_effect_sends_pot_value_to_pot0);
  RUN_TEST(test_init_menu_starts_locked);

  // Boot complete - FSM state transitions
  RUN_TEST(test_boot_complete_enters_program_idle_when_active_program_mode);
  RUN_TEST(test_boot_complete_enters_preset_idle_when_active_preset_mode);
  RUN_TEST(test_boot_complete_enters_bypassed_when_bypass_state_is_bypassed);
  RUN_TEST(test_boot_complete_bypassed_takes_precedence_over_preset_mode);

  // Boot complete - hardware state
  RUN_TEST(test_boot_complete_bypass_relay_on_when_active);
  RUN_TEST(test_boot_complete_bypass_relay_off_when_bypassed);
  RUN_TEST(test_boot_complete_fv1_has_correct_program);

  // Boot complete - event chain
  RUN_TEST(test_boot_complete_event_bus_empty_after_full_dispatch);

  // Boot complete - display
  RUN_TEST(test_boot_complete_display_initialized);

  return UNITY_END();
}
