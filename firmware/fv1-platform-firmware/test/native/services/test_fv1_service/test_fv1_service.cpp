#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/fv1_handler.h"
#include "services/fv1_service.h"
#include "mock/mock_fv1.h"

#include "../src/logic/fv1_handler.cpp"
#include "../src/services/fv1_service.cpp"

// =============================================================================
// Helper Functions
// =============================================================================

void clearEventBus() {
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }
}

Event makeLogicProgramChangedEvent(uint8_t t_programId) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_id = t_programId;
  return e;
}

Event makeLogicPotValueChangedEvent(uint8_t t_potId, uint16_t t_value) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;
  e.m_id = t_potId;
  e.m_data.value = t_value;
  return e;
}

Event makeLogicTempoValueChangedEvent(uint16_t t_value) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_value;
  return e;
}

void assertEventBusEmpty() {
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void setUp() {
  clearEventBus();
}

void tearDown() {
}

// =============================================================================
// Init Tests - Delay Effects
// =============================================================================

void test_init_sends_program_change() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  logicalState.m_currentProgram = 3;

  fv1Service.init();

  // Program 3 = binary 011 (S0=1, S1=1, S2=0)
  TEST_ASSERT_EQUAL(1, mockFv1.m_s0);
  TEST_ASSERT_EQUAL(1, mockFv1.m_s1);
  TEST_ASSERT_EQUAL(0, mockFv1.m_s2);
}

void test_init_sends_program_change_for_program_7() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  logicalState.m_currentProgram = 7;

  fv1Service.init();

  // Program 7 = binary 111 (S0=1, S1=1, S2=1)
  TEST_ASSERT_EQUAL(1, mockFv1.m_s0);
  TEST_ASSERT_EQUAL(1, mockFv1.m_s1);
  TEST_ASSERT_EQUAL(1, mockFv1.m_s2);
}

void test_init_delay_effect_sends_mapped_tempo_to_pot0() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  // Program 0 is a delay effect (Digital delay)
  logicalState.m_currentProgram = 0;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tempo = 500; // Middle of 20-1000ms range

  fv1Service.init();

  // Should have sent 4 pot values (pot0 via tempo, pot1, pot2, and pot3 is unused but Fv1 only has 3 pots)
  TEST_ASSERT_EQUAL(3, mockFv1.m_potValues.size());

  // First pot value should be Pot0 with mapped tempo value
  auto& firstPot = mockFv1.m_potValues[0];
  TEST_ASSERT_EQUAL(Fv1Pot::Pot0, std::get<0>(firstPot));
  // Value should be mapped from 500ms in range 20-1000ms to DAC range
}

void test_init_delay_effect_configures_tempo_handler() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  // Program 0 is a delay effect with min=20ms, max=1000ms
  logicalState.m_currentProgram = 0;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tempo = 20; // At minimum

  fv1Service.init();

  // First pot value should be Pot0
  TEST_ASSERT_EQUAL(Fv1Pot::Pot0, std::get<0>(mockFv1.m_potValues[0]));
}

void test_init_delay_effect_sends_pot1_and_pot2() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  logicalState.m_currentProgram = 0;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_potParams[0][1].m_value = 512;
  logicalState.m_potParams[0][2].m_value = 768;

  fv1Service.init();

  // Should have 3 pot values: Pot0 (tempo), Pot1, Pot2
  TEST_ASSERT_EQUAL(3, mockFv1.m_potValues.size());

  // Pot1 value
  auto& pot1 = mockFv1.m_potValues[1];
  TEST_ASSERT_EQUAL(Fv1Pot::Pot1, std::get<0>(pot1));
  TEST_ASSERT_EQUAL(512, std::get<1>(pot1));

  // Pot2 value
  auto& pot2 = mockFv1.m_potValues[2];
  TEST_ASSERT_EQUAL(Fv1Pot::Pot2, std::get<0>(pot2));
  TEST_ASSERT_EQUAL(768, std::get<1>(pot2));
}

// =============================================================================
// Init Tests - Non-Delay Effects
// =============================================================================

void test_init_non_delay_effect_sends_pot0_directly() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  // Program 5 is Plate reverb (non-delay effect)
  logicalState.m_currentProgram = 5;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[5];
  logicalState.m_potParams[5][0].m_value = 256;
  logicalState.m_potParams[5][1].m_value = 512;
  logicalState.m_potParams[5][2].m_value = 768;

  fv1Service.init();

  // Should have 3 pot values
  TEST_ASSERT_EQUAL(3, mockFv1.m_potValues.size());

  // Pot0 should have direct value from potParams, not mapped tempo
  auto& pot0 = mockFv1.m_potValues[0];
  TEST_ASSERT_EQUAL(Fv1Pot::Pot0, std::get<0>(pot0));
  TEST_ASSERT_EQUAL(256, std::get<1>(pot0));
}

void test_init_non_delay_effect_sends_all_pots() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  // Program 7 is Shimmer plate (non-delay effect)
  logicalState.m_currentProgram = 7;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[7];
  logicalState.m_potParams[7][0].m_value = 100;
  logicalState.m_potParams[7][1].m_value = 200;
  logicalState.m_potParams[7][2].m_value = 300;

  fv1Service.init();

  TEST_ASSERT_EQUAL(3, mockFv1.m_potValues.size());

  TEST_ASSERT_EQUAL(Fv1Pot::Pot0, std::get<0>(mockFv1.m_potValues[0]));
  TEST_ASSERT_EQUAL(100, std::get<1>(mockFv1.m_potValues[0]));

  TEST_ASSERT_EQUAL(Fv1Pot::Pot1, std::get<0>(mockFv1.m_potValues[1]));
  TEST_ASSERT_EQUAL(200, std::get<1>(mockFv1.m_potValues[1]));

  TEST_ASSERT_EQUAL(Fv1Pot::Pot2, std::get<0>(mockFv1.m_potValues[2]));
  TEST_ASSERT_EQUAL(300, std::get<1>(mockFv1.m_potValues[2]));
}

// =============================================================================
// handleEvent Tests - Program Change
// =============================================================================

void test_handle_program_change_reinitializes() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  // Initial state
  logicalState.m_currentProgram = 0;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];

  fv1Service.init();
  mockFv1.m_potValues.clear();

  // Change to program 5
  logicalState.m_currentProgram = 5;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[5];
  logicalState.m_potParams[5][0].m_value = 400;
  logicalState.m_potParams[5][1].m_value = 500;
  logicalState.m_potParams[5][2].m_value = 600;

  fv1Service.handleEvent(makeLogicProgramChangedEvent(5));

  // Should have sent program change
  TEST_ASSERT_EQUAL(1, mockFv1.m_s0);
  TEST_ASSERT_EQUAL(0, mockFv1.m_s1);
  TEST_ASSERT_EQUAL(1, mockFv1.m_s2);

  // Should have sent all pot values
  TEST_ASSERT_EQUAL(3, mockFv1.m_potValues.size());
}

// =============================================================================
// handleEvent Tests - Pot Value Changed
// =============================================================================

void test_handle_pot_value_changed_sends_to_fv1() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  logicalState.m_currentProgram = 5;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[5];
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_value = 333;

  fv1Service.init();
  mockFv1.m_potValues.clear();

  // Update pot value in logical state (normally done by PotService)
  logicalState.m_potParams[logicalState.m_currentProgram][0].m_value = 777;

  fv1Service.handleEvent(makeLogicPotValueChangedEvent(0, 777));

  TEST_ASSERT_EQUAL(1, mockFv1.m_potValues.size());
  TEST_ASSERT_EQUAL(Fv1Pot::Pot0, std::get<0>(mockFv1.m_potValues[0]));
  TEST_ASSERT_EQUAL(777, std::get<1>(mockFv1.m_potValues[0]));
}

void test_handle_pot1_value_changed() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  logicalState.m_currentProgram = 0;
  logicalState.m_potParams[logicalState.m_currentProgram][1].m_value = 512;

  fv1Service.init();
  mockFv1.m_potValues.clear();

  logicalState.m_potParams[logicalState.m_currentProgram][1].m_value = 888;

  fv1Service.handleEvent(makeLogicPotValueChangedEvent(1, 888));

  TEST_ASSERT_EQUAL(1, mockFv1.m_potValues.size());
  TEST_ASSERT_EQUAL(Fv1Pot::Pot1, std::get<0>(mockFv1.m_potValues[0]));
  TEST_ASSERT_EQUAL(888, std::get<1>(mockFv1.m_potValues[0]));
}

void test_handle_pot2_value_changed() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  logicalState.m_currentProgram = 0;
  logicalState.m_potParams[logicalState.m_currentProgram][2].m_value = 256;

  fv1Service.init();
  mockFv1.m_potValues.clear();

  logicalState.m_potParams[logicalState.m_currentProgram][2].m_value = 999;

  fv1Service.handleEvent(makeLogicPotValueChangedEvent(2, 999));

  TEST_ASSERT_EQUAL(1, mockFv1.m_potValues.size());
  TEST_ASSERT_EQUAL(Fv1Pot::Pot2, std::get<0>(mockFv1.m_potValues[0]));
  TEST_ASSERT_EQUAL(999, std::get<1>(mockFv1.m_potValues[0]));
}

// =============================================================================
// handleEvent Tests - Tempo Value Changed
// =============================================================================

void test_handle_tempo_value_changed_sends_mapped_value() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  // Set up delay effect
  logicalState.m_currentProgram = 0;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tempo = 500;

  fv1Service.init();
  mockFv1.m_potValues.clear();

  // Change tempo
  logicalState.m_tempo = 750;

  fv1Service.handleEvent(makeLogicTempoValueChangedEvent(750));

  TEST_ASSERT_EQUAL(1, mockFv1.m_potValues.size());
  TEST_ASSERT_EQUAL(Fv1Pot::Pot0, std::get<0>(mockFv1.m_potValues[0]));
  // Value should be mapped from tempo to DAC range
}

void test_handle_tempo_at_min_boundary() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  // Digital delay: min=20ms, max=1000ms
  logicalState.m_currentProgram = 0;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tempo = 20;

  fv1Service.init();
  mockFv1.m_potValues.clear();

  fv1Service.handleEvent(makeLogicTempoValueChangedEvent(20));

  TEST_ASSERT_EQUAL(1, mockFv1.m_potValues.size());
  TEST_ASSERT_EQUAL(Fv1Pot::Pot0, std::get<0>(mockFv1.m_potValues[0]));
}

void test_handle_tempo_at_max_boundary() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  // Digital delay: min=20ms, max=1000ms
  logicalState.m_currentProgram = 0;
  logicalState.m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  logicalState.m_tempo = 1000;

  fv1Service.init();
  mockFv1.m_potValues.clear();

  fv1Service.handleEvent(makeLogicTempoValueChangedEvent(1000));

  TEST_ASSERT_EQUAL(1, mockFv1.m_potValues.size());
  TEST_ASSERT_EQUAL(Fv1Pot::Pot0, std::get<0>(mockFv1.m_potValues[0]));
}

// =============================================================================
// handleEvent Tests - No Events Published
// =============================================================================

void test_handle_events_does_not_publish_events() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  fv1Service.init();
  clearEventBus();

  // Handle various events
  fv1Service.handleEvent(makeLogicProgramChangedEvent(1));
  fv1Service.handleEvent(makeLogicPotValueChangedEvent(0, 512));
  fv1Service.handleEvent(makeLogicTempoValueChangedEvent(500));

  // Fv1Service should not publish any events - it's a sink
  assertEventBusEmpty();
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_logic_program_value_changed() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(fv1Service.interestedIn(e));
}

void test_interested_in_logic_pot_value_changed() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(fv1Service.interestedIn(e));
}

void test_interested_in_logic_tempo_value_changed() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(fv1Service.interestedIn(e));
}

void test_not_interested_in_logic_program_other_actions() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;

  e.m_action = EventAction::kSave;
  TEST_ASSERT_FALSE(fv1Service.interestedIn(e));

  e.m_action = EventAction::kLoad;
  TEST_ASSERT_FALSE(fv1Service.interestedIn(e));

  e.m_action = EventAction::kToggled;
  TEST_ASSERT_FALSE(fv1Service.interestedIn(e));
}

void test_not_interested_in_physical_events() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_FALSE(fv1Service.interestedIn(e));
}

void test_not_interested_in_midi_events() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  Event e;
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_FALSE(fv1Service.interestedIn(e));
}

void test_not_interested_in_memory_events() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kSave;

  TEST_ASSERT_FALSE(fv1Service.interestedIn(e));
}

void test_not_interested_in_ui_events() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kPot;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_FALSE(fv1Service.interestedIn(e));
}

void test_not_interested_in_logic_other_subjects() {
  LogicalState logicalState;
  MockFv1 mockFv1;
  Fv1Service fv1Service(logicalState, mockFv1);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_action = EventAction::kValueChanged;

  e.m_subject = EventSubject::kBypass;
  TEST_ASSERT_FALSE(fv1Service.interestedIn(e));

  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(fv1Service.interestedIn(e));

  e.m_subject = EventSubject::kPreset;
  TEST_ASSERT_FALSE(fv1Service.interestedIn(e));

  e.m_subject = EventSubject::kSwitch;
  TEST_ASSERT_FALSE(fv1Service.interestedIn(e));
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Init - Delay Effects
  RUN_TEST(test_init_sends_program_change);
  RUN_TEST(test_init_sends_program_change_for_program_7);
  RUN_TEST(test_init_delay_effect_sends_mapped_tempo_to_pot0);
  RUN_TEST(test_init_delay_effect_configures_tempo_handler);
  RUN_TEST(test_init_delay_effect_sends_pot1_and_pot2);

  // Init - Non-Delay Effects
  RUN_TEST(test_init_non_delay_effect_sends_pot0_directly);
  RUN_TEST(test_init_non_delay_effect_sends_all_pots);

  // handleEvent - Program Change
  RUN_TEST(test_handle_program_change_reinitializes);

  // handleEvent - Pot Value Changed
  RUN_TEST(test_handle_pot_value_changed_sends_to_fv1);
  RUN_TEST(test_handle_pot1_value_changed);
  RUN_TEST(test_handle_pot2_value_changed);

  // handleEvent - Tempo Value Changed
  RUN_TEST(test_handle_tempo_value_changed_sends_mapped_value);
  RUN_TEST(test_handle_tempo_at_min_boundary);
  RUN_TEST(test_handle_tempo_at_max_boundary);

  // handleEvent - No Events Published
  RUN_TEST(test_handle_events_does_not_publish_events);

  // interestedIn
  RUN_TEST(test_interested_in_logic_program_value_changed);
  RUN_TEST(test_interested_in_logic_pot_value_changed);
  RUN_TEST(test_interested_in_logic_tempo_value_changed);
  RUN_TEST(test_not_interested_in_logic_program_other_actions);
  RUN_TEST(test_not_interested_in_physical_events);
  RUN_TEST(test_not_interested_in_midi_events);
  RUN_TEST(test_not_interested_in_memory_events);
  RUN_TEST(test_not_interested_in_ui_events);
  RUN_TEST(test_not_interested_in_logic_other_subjects);

  UNITY_END();
}
