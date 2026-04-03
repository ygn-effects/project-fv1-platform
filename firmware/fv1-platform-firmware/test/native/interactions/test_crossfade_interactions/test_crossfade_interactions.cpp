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

Event makeMidiProgramValueChangedEvent(uint8_t t_program) {
  Event e{};
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_program;
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

void setUp() {

}

void tearDown() {

}

// =============================================================================
// Boot
// =============================================================================

void test_boot_initialize_dacs() {
  InteractionFixture fix;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  TEST_ASSERT_TRUE(fix.mockDacDry.m_initialized);
  TEST_ASSERT_TRUE(fix.mockDacWet.m_initialized);
}

void test_boot_syncs_handler() {
  InteractionFixture fix;
  fix.logicalState.m_currentProgram = 4;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  TEST_ASSERT_EQUAL(fix.logicalState.m_activeProgram->m_crossfadeParams.m_curve, fix.crossfadeService.getHandler().m_currentCurve);
  TEST_ASSERT_EQUAL(fix.logicalState.m_activeProgram->m_crossfadeParams.m_minValue, fix.crossfadeService.getHandler().m_minInputValue);
  TEST_ASSERT_EQUAL(fix.logicalState.m_activeProgram->m_crossfadeParams.m_maxValue, fix.crossfadeService.getHandler().m_maxInputValue);
}

// =============================================================================
// Mix Pot Input
// =============================================================================

void test_mix_pot_full_dry_sets_dacs() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_currentProgram = 5;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Send the vent
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kMixPot, 0));

  TEST_ASSERT_EQUAL(1023, fix.mockDacDry.m_lastValue);
  TEST_ASSERT_EQUAL(0, fix.mockDacWet.m_lastValue);
}

void test_mix_pot_full_wet_sets_dacs() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_currentProgram = 5;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Send the vent
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kMixPot, 1023));

  TEST_ASSERT_EQUAL(0, fix.mockDacDry.m_lastValue);
  TEST_ASSERT_EQUAL(1023, fix.mockDacWet.m_lastValue);
}

void test_mix_pot_mid_point_sets_dacs() {
  InteractionFixture fix;
  fix.logicalState.m_bypassState = BypassState::kActive;
  fix.logicalState.m_currentProgram = 5;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Send the vent
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kMixPot, 512));

  TEST_ASSERT_EQUAL(1023, fix.mockDacDry.m_lastValue);
  TEST_ASSERT_EQUAL(1023, fix.mockDacWet.m_lastValue);
}

// =============================================================================
// Program Change
// =============================================================================

void test_program_change_sync_handler() {
  InteractionFixture fix;
  fix.logicalState.m_currentProgram = 0;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Send event
  fix.publishAndDispatchAllEvents(makeMidiProgramValueChangedEvent(5));

  TEST_ASSERT_EQUAL(fix.logicalState.m_activeProgram->m_crossfadeParams.m_curve, fix.crossfadeService.getHandler().m_currentCurve);
  TEST_ASSERT_EQUAL(fix.logicalState.m_activeProgram->m_crossfadeParams.m_minValue, fix.crossfadeService.getHandler().m_minInputValue);
  TEST_ASSERT_EQUAL(fix.logicalState.m_activeProgram->m_crossfadeParams.m_maxValue, fix.crossfadeService.getHandler().m_maxInputValue);
}

void test_program_change_sets_dacs() {
  InteractionFixture fix;
  fix.logicalState.m_currentProgram = 0;
  fix.syncEepromWithState();
  fix.init();

  // Boot
  fix.publishAndDispatchAllEvents(makeBootEvent());

  // Set the dacs
  fix.publishAndDispatchAllEvents(makeDriverPotValueChangedEvent(PotId::kMixPot, 512));

  // Send event
  fix.publishAndDispatchAllEvents(makeMidiProgramValueChangedEvent(5));

  TEST_ASSERT_EQUAL(1023, fix.mockDacDry.m_lastValue);
  TEST_ASSERT_EQUAL(1023, fix.mockDacWet.m_lastValue);
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Boot
  RUN_TEST(test_boot_initialize_dacs);
  RUN_TEST(test_boot_syncs_handler);

  // Mix Pot Input
  RUN_TEST(test_mix_pot_full_dry_sets_dacs);
  RUN_TEST(test_mix_pot_full_wet_sets_dacs);
  RUN_TEST(test_mix_pot_mid_point_sets_dacs);

  // Program Change
  RUN_TEST(test_program_change_sync_handler);
  RUN_TEST(test_program_change_sets_dacs);

  return UNITY_END();
}
