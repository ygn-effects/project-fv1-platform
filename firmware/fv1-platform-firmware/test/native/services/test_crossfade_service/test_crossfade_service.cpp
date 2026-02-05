#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "mock/mock_dac.h"
#include "ui/settings.h"

#include "../src/services/crossfade_service.cpp"
#include "../src/logic/crossfade_handler.cpp"

// =============================================================================
// Helper functions
// =============================================================================

Event makeLogicProgramChangedEvent(uint8_t t_programId) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_id = t_programId;
  return e;
}

Event makeLogicPotValueChangedEvent(PotId t_id) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(t_id);
  return e;
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

void test_init_syncs_handler_current_curve() {
  LogicalState state;
  MockDac dacWet;
  MockDac dacDry;
  CrossfadeService fadeService(state, dacDry, dacWet);

  fadeService.init();

  TEST_ASSERT_EQUAL(ProgramsDefinitions::kPrograms[state.m_currentProgram].m_crossfadeParams.m_curve, fadeService.getHandler().m_currentCurve);

  assertEventBusEmpty();
}

void test_init_syncs_handler_min_max_values() {
  LogicalState state;
  MockDac dacWet;
  MockDac dacDry;
  CrossfadeService fadeService(state, dacDry, dacWet);

  fadeService.init();

  TEST_ASSERT_EQUAL(ProgramsDefinitions::kPrograms[state.m_currentProgram].m_crossfadeParams.m_minValue, fadeService.getHandler().m_minInputValue);
  TEST_ASSERT_EQUAL(ProgramsDefinitions::kPrograms[state.m_currentProgram].m_crossfadeParams.m_maxValue, fadeService.getHandler().m_maxInputValue);

  assertEventBusEmpty();
}

void test_init_initialized_dacs() {
  LogicalState state;
  MockDac dacWet;
  MockDac dacDry;
  CrossfadeService fadeService(state,dacDry, dacWet);

  fadeService.init();

  TEST_ASSERT_TRUE(dacDry.m_initialized);
  TEST_ASSERT_TRUE(dacWet.m_initialized);
}

// =============================================================================
// Pot value mapping tests
// =============================================================================

void test_logical_pot_value_changed_event_writes_dacs_full_dry() {
  LogicalState state;
  MockDac dacWet;
  MockDac dacDry;
  CrossfadeService fadeService(state, dacDry, dacWet);

  state.m_currentProgram = 0;
  fadeService.init();

  // Full dry
  state.m_potParams[state.m_currentProgram][3].m_value = 0;
  fadeService.handleEvent(makeLogicPotValueChangedEvent(PotId::kMixPot));

  TEST_ASSERT_EQUAL(1023, dacDry.m_lastValue);
  TEST_ASSERT_EQUAL(0, dacWet.m_lastValue);

  assertEventBusEmpty();
}

void test_logical_pot_value_changed_event_writes_dacs_full_wet() {
  LogicalState state;
  MockDac dacWet;
  MockDac dacDry;
  CrossfadeService fadeService(state, dacDry, dacWet);

  state.m_currentProgram = 0;
  fadeService.init();

  // Full wet
  state.m_potParams[state.m_currentProgram][3].m_value = 1023;
  fadeService.handleEvent(makeLogicPotValueChangedEvent(PotId::kMixPot));

  // index = 512 >> 2 = 128
  // sineLut[128] = 724, sineLut[127] = 719
  TEST_ASSERT_EQUAL(719, dacDry.m_lastValue);
  TEST_ASSERT_EQUAL(724, dacWet.m_lastValue);

  assertEventBusEmpty();
}

void test_logical_pot_value_changed_event_writes_dacs_mid_point() {
  LogicalState state;
  MockDac dacWet;
  MockDac dacDry;
  CrossfadeService fadeService(state, dacDry, dacWet);

  state.m_currentProgram = 0;
  fadeService.init();

  // Mid point (512)
  state.m_potParams[state.m_currentProgram][3].m_value = 512;
  fadeService.handleEvent(makeLogicPotValueChangedEvent(PotId::kMixPot));

  // Program 0 uses kConstantPower with range 0-512
  // effectiveValue = mapValue(512, 0, 1023, 0, 512) = 256
  // kConstantPower at 256: index = 256 >> 2 = 64
  // sineLut[64] = 392, sineLut[191] = 943
  TEST_ASSERT_EQUAL(943, dacDry.m_lastValue);
  TEST_ASSERT_EQUAL(392, dacWet.m_lastValue);

  assertEventBusEmpty();
}

// =============================================================================
// Program Change Events
// =============================================================================

void test_program_change_syncs_handler_current_curve() {
  LogicalState state;
  MockDac dacWet;
  MockDac dacDry;
  CrossfadeService fadeService(state, dacDry, dacWet);

  state.m_currentProgram = 2;

  fadeService.handleEvent(makeLogicProgramChangedEvent(state.m_currentProgram));

  TEST_ASSERT_EQUAL(ProgramsDefinitions::kPrograms[state.m_currentProgram].m_crossfadeParams.m_curve, fadeService.getHandler().m_currentCurve);

  assertEventBusEmpty();
}

void test_program_changes_syncs_handler_min_max_values() {
  LogicalState state;
  MockDac dacWet;
  MockDac dacDry;
  CrossfadeService fadeService(state, dacDry, dacWet);

  state.m_currentProgram = 3;

  fadeService.handleEvent(makeLogicProgramChangedEvent(state.m_currentProgram));

  TEST_ASSERT_EQUAL(ProgramsDefinitions::kPrograms[state.m_currentProgram].m_crossfadeParams.m_minValue, fadeService.getHandler().m_minInputValue);
  TEST_ASSERT_EQUAL(ProgramsDefinitions::kPrograms[state.m_currentProgram].m_crossfadeParams.m_maxValue, fadeService.getHandler().m_maxInputValue);

  assertEventBusEmpty();
}

void test_program_change_recalculates_dacs_with_current_mix_value() {
  LogicalState state;
  MockDac dacWet;
  MockDac dacDry;
  CrossfadeService fadeService(state, dacDry, dacWet);

  fadeService.init();

  dacDry.m_writeCount = 0;
  dacWet.m_writeCount = 0;

  state.m_currentProgram = 2;
  state.m_potParams[state.m_currentProgram][static_cast<uint8_t>(PotId::kMixPot)].m_value = 512;

  fadeService.handleEvent(makeLogicProgramChangedEvent(state.m_currentProgram));

  TEST_ASSERT_EQUAL(1, dacDry.m_writeCount);
  TEST_ASSERT_EQUAL(1, dacWet.m_writeCount);

  assertEventBusEmpty();
}

// =============================================================================
// InterestedIn tests
// =============================================================================

void test_interested_in_logic_mix_pot_value_changed() {
  LogicalState state;
  MockDac dacWet;
  MockDac dacDry;
  CrossfadeService fadeService(state, dacDry, dacWet);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = static_cast<uint8_t>(PotId::kMixPot);

  TEST_ASSERT_TRUE(fadeService.interestedIn(e));
}

void test_interested_in_logic_program_changed() {
  LogicalState state;
  MockDac dacWet;
  MockDac dacDry;
  CrossfadeService fadeService(state, dacDry, dacWet);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(fadeService.interestedIn(e));
}

void test_not_interested_in_other_events() {
  LogicalState state;
  MockDac dacWet;
  MockDac dacDry;
  CrossfadeService fadeService(state, dacDry, dacWet);

  Event e;

  // Not interested in physical pot events
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  TEST_ASSERT_FALSE(fadeService.interestedIn(e));

  // Not interested in other logical pot events
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPot;
  e.m_id = static_cast<uint8_t>(PotId::kPot0);
  TEST_ASSERT_FALSE(fadeService.interestedIn(e));

  // Not interested in logic expr events
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(fadeService.interestedIn(e));

  // Not interested in MIDI events
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(fadeService.interestedIn(e));

  // Not interested in memory events
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(fadeService.interestedIn(e));

  // Not interested in logic program with different action
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kSave;
  TEST_ASSERT_FALSE(fadeService.interestedIn(e));
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Init Tests
  RUN_TEST(test_init_syncs_handler_current_curve);
  RUN_TEST(test_init_syncs_handler_min_max_values);
  RUN_TEST(test_init_initialized_dacs);

  // Program Change Events
  RUN_TEST(test_program_change_syncs_handler_current_curve);
  RUN_TEST(test_program_changes_syncs_handler_min_max_values);
  RUN_TEST(test_program_change_recalculates_dacs_with_current_mix_value);

  // InterestedIn tests
  RUN_TEST(test_interested_in_logic_mix_pot_value_changed);
  RUN_TEST(test_interested_in_logic_program_changed);
  RUN_TEST(test_not_interested_in_other_events);

  // Pot value mapping tests
  RUN_TEST(test_logical_pot_value_changed_event_writes_dacs_full_dry);
  RUN_TEST(test_logical_pot_value_changed_event_writes_dacs_full_wet);
  RUN_TEST(test_logical_pot_value_changed_event_writes_dacs_mid_point);

  UNITY_END();
}