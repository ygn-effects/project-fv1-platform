#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/expr_service.h"
#include "ui/settings.h"

#include "../src/services/expr_service.cpp"
#include "../src/logic/expr_handler.cpp"

// =============================================================================
// Helper functions
// =============================================================================

Event makePhysicalExprEvent(uint16_t t_value) {
  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = t_value;
  return e;
}

Event makePhysicalExprConnected() {
  Event e{};
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kConnected;
  return e;
}

Event makePhysicalExprDisconnected() {
  Event e{};
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kDisconnected;
  return e;
}

Event makeLogicProgramChangedEvent(uint8_t t_programId) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_id = t_programId;
  return e;
}

Event makeUIExprEvent(ExprParam t_param, int16_t t_delta = 0) {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kDeltaChanged;
  e.m_id = static_cast<uint8_t>(t_param);
  e.m_data.delta = t_delta;
  return e;
}

void assertExprSaveEventPublished() {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kMemory, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kExpr, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kSave, e.m_action);
}

void assertLogicExprEventPublished(PotId t_expectedPotId, uint16_t t_expectedValue) {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kExpr, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_EQUAL(static_cast<uint8_t>(t_expectedPotId), e.m_id);
  TEST_ASSERT_EQUAL(t_expectedValue, e.m_data.value);
}

void assertExprConnectedPublished() {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kExpr, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kConnected, e.m_action);
}

void assertExprDisconnectedPublished() {
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kLogic, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kExpr, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kDisconnected, e.m_action);
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

void test_init_when_inactive() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  // Configure program 0 with inactive expr state
  logicalState.m_exprParams[0].m_state = ExprState::kInactive;
  logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot0;

  exprService.init();

  // Verify expr events are ignored when inactive
  exprService.handleEvent(makePhysicalExprEvent(512));
  assertEventBusEmpty();
}

void test_init_syncs_handler_from_current_program() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  // Configure program 0 with specific expr settings
  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot2;
  logicalState.m_exprParams[0].m_direction = Direction::kInverted;
  logicalState.m_exprParams[0].m_heelValue = 100;
  logicalState.m_exprParams[0].m_toeValue = 900;

  exprService.init();

  // Verify handler synced by sending physical event and checking output
  exprService.handleEvent(makePhysicalExprEvent(0));

  // With direction inverted, ADC 0 should map to toeValue (900)
  assertLogicExprEventPublished(PotId::kPot2, 900);
  assertEventBusEmpty();
}

// =============================================================================
// Physical Expression Pedal Events
// =============================================================================

void test_physical_expr_ignored_when_disconnected_and_inactive() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprParams[logicalState.m_currentProgram].m_state = ExprState::kInactive;
  logicalState.m_exprConnected = false;

  exprService.handleEvent(makePhysicalExprEvent(512));

  assertEventBusEmpty();
}

void test_physical_expr_ignored_when_connected_and_inactive() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprParams[logicalState.m_currentProgram].m_state = ExprState::kInactive;
  logicalState.m_exprConnected = true;

  exprService.handleEvent(makePhysicalExprEvent(512));

  assertEventBusEmpty();
}

void test_physical_expr_ignored_when_disconnected_and_active() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprConnected = false;
  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot0;
  exprService.init();

  exprService.handleEvent(makePhysicalExprEvent(512));

  assertEventBusEmpty();
}

void test_physical_expr_publishes_logic_event_when_connected_and_active() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprConnected = true;
  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot0;
  exprService.init();

  exprService.handleEvent(makePhysicalExprEvent(512));

  assertLogicExprEventPublished(PotId::kPot0, 512);
  assertEventBusEmpty();
}

void test_physical_expr_maps_to_correct_pot_id() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  // Test each mapped pot
  MappedPot mappedPots[] = {MappedPot::kPot0, MappedPot::kPot1, MappedPot::kPot2, MappedPot::kMixPot};
  PotId expectedPotIds[] = {PotId::kPot0, PotId::kPot1, PotId::kPot2, PotId::kMixPot};

  for (int i = 0; i < 4; i++) {
    clearEventBus();

    logicalState.m_exprParams[0].m_state = ExprState::kActive;
    logicalState.m_exprParams[0].m_mappedPot = mappedPots[i];
    exprService.init();

    exprService.handleEvent(makePhysicalExprEvent(512));

    assertLogicExprEventPublished(expectedPotIds[i], 512);
    assertEventBusEmpty();
  }
}

void test_physical_expr_connected_sets_logical_state() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  exprService.init();

  exprService.handleEvent(makePhysicalExprConnected());

  TEST_ASSERT_EQUAL(true, logicalState.m_exprConnected);
  assertExprConnectedPublished();
  assertEventBusEmpty();
}

void test_physical_expr_disconnected_sets_logical_state() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  // Default is false
  logicalState.m_exprConnected = true;
  exprService.init();

  exprService.handleEvent(makePhysicalExprDisconnected());

  TEST_ASSERT_EQUAL(false, logicalState.m_exprConnected);
  assertExprDisconnectedPublished();
  assertEventBusEmpty();
}

// =============================================================================
// Program Change Events
// =============================================================================

void test_program_change_syncs_handler() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  // Configure different settings for program 0 and 1
  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot0;

  logicalState.m_exprParams[1].m_state = ExprState::kActive;
  logicalState.m_exprParams[1].m_mappedPot = MappedPot::kPot1;

  exprService.init();

  // Verify program 0 settings are active
  exprService.handleEvent(makePhysicalExprEvent(512));
  assertLogicExprEventPublished(PotId::kPot0, 512);
  assertEventBusEmpty();

  // Change to program 1
  logicalState.m_currentProgram = 1;
  exprService.handleEvent(makeLogicProgramChangedEvent(1));

  // Verify program 1 settings are now active
  exprService.handleEvent(makePhysicalExprEvent(512));
  assertLogicExprEventPublished(PotId::kPot1, 512);
  assertEventBusEmpty();
}

void test_program_change_to_inactive_expr() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  // Program 0: active, Program 1: inactive
  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[1].m_state = ExprState::kInactive;

  exprService.init();

  // Verify expr works on program 0
  exprService.handleEvent(makePhysicalExprEvent(512));
  assertLogicExprEventPublished(PotId::kPot0, 512);
  assertEventBusEmpty();

  // Change to program 1
  logicalState.m_currentProgram = 1;
  exprService.handleEvent(makeLogicProgramChangedEvent(1));

  // Expr events should now be ignored
  exprService.handleEvent(makePhysicalExprEvent(512));
  assertEventBusEmpty();
}

// =============================================================================
// UI Expression Parameter Events
// =============================================================================

void test_ui_toggle_state() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  // Initial state is inactive
  TEST_ASSERT_EQUAL(ExprState::kInactive, logicalState.m_exprParams[0].m_state);

  exprService.init();
  exprService.handleEvent(makeUIExprEvent(ExprParam::kState));

  // State should be toggled to active
  TEST_ASSERT_EQUAL(ExprState::kActive, logicalState.m_exprParams[0].m_state);
  assertExprSaveEventPublished();
  assertEventBusEmpty();

  // Toggle again
  exprService.handleEvent(makeUIExprEvent(ExprParam::kState));
  TEST_ASSERT_EQUAL(ExprState::kInactive, logicalState.m_exprParams[0].m_state);
  assertExprSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_change_mapped_pot() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  // Initial mapped pot is Pot0
  TEST_ASSERT_EQUAL(MappedPot::kPot0, logicalState.m_exprParams[0].m_mappedPot);

  exprService.init();
  exprService.handleEvent(makeUIExprEvent(ExprParam::kMappedPot, 1));

  TEST_ASSERT_EQUAL(MappedPot::kPot1, logicalState.m_exprParams[0].m_mappedPot);
  assertExprSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_change_mapped_pot_wraps() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprParams[0].m_mappedPot = MappedPot::kMixPot;
  exprService.init();

  // Increment past last pot should wrap to first
  exprService.handleEvent(makeUIExprEvent(ExprParam::kMappedPot, 1));
  TEST_ASSERT_EQUAL(MappedPot::kPot0, logicalState.m_exprParams[0].m_mappedPot);

  // Decrement from first should wrap to last
  exprService.handleEvent(makeUIExprEvent(ExprParam::kMappedPot, -1));
  TEST_ASSERT_EQUAL(MappedPot::kMixPot, logicalState.m_exprParams[0].m_mappedPot);
}

void test_ui_toggle_direction() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  // Initial direction is normal
  TEST_ASSERT_EQUAL(Direction::kNormal, logicalState.m_exprParams[0].m_direction);

  exprService.init();
  exprService.handleEvent(makeUIExprEvent(ExprParam::kDirection));

  TEST_ASSERT_EQUAL(Direction::kInverted, logicalState.m_exprParams[0].m_direction);
  assertExprSaveEventPublished();
  assertEventBusEmpty();

  // Toggle again
  exprService.handleEvent(makeUIExprEvent(ExprParam::kDirection));
  TEST_ASSERT_EQUAL(Direction::kNormal, logicalState.m_exprParams[0].m_direction);
  assertExprSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_change_heel_value() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  // Initial heel value is 0
  TEST_ASSERT_EQUAL(0, logicalState.m_exprParams[0].m_heelValue);

  exprService.init();
  exprService.handleEvent(makeUIExprEvent(ExprParam::kHeel, 10));

  TEST_ASSERT_EQUAL(10, logicalState.m_exprParams[0].m_heelValue);
  assertExprSaveEventPublished();
  assertEventBusEmpty();

  // Negative delta
  exprService.handleEvent(makeUIExprEvent(ExprParam::kHeel, -5));
  TEST_ASSERT_EQUAL(5, logicalState.m_exprParams[0].m_heelValue);
  assertExprSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_change_toe_value() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  // Initial toe value is 1023
  TEST_ASSERT_EQUAL(1023, logicalState.m_exprParams[0].m_toeValue);

  exprService.init();
  exprService.handleEvent(makeUIExprEvent(ExprParam::kToe, -10));

  TEST_ASSERT_EQUAL(1013, logicalState.m_exprParams[0].m_toeValue);
  assertExprSaveEventPublished();
  assertEventBusEmpty();

  // Positive delta
  exprService.handleEvent(makeUIExprEvent(ExprParam::kToe, 5));
  TEST_ASSERT_EQUAL(1018, logicalState.m_exprParams[0].m_toeValue);
  assertExprSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_heel_value_clamped() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprParams[0].m_heelValue = 5;
  exprService.init();

  // Try to go below 0
  exprService.handleEvent(makeUIExprEvent(ExprParam::kHeel, -10));
  TEST_ASSERT_EQUAL(0, logicalState.m_exprParams[0].m_heelValue);
  assertExprSaveEventPublished();
  assertEventBusEmpty();

  // Try to go above 1023
  logicalState.m_exprParams[0].m_heelValue = 1020;
  exprService.init();
  exprService.handleEvent(makeUIExprEvent(ExprParam::kHeel, 10));
  TEST_ASSERT_EQUAL(1023, logicalState.m_exprParams[0].m_heelValue);
  assertExprSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_toe_value_clamped() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprParams[0].m_toeValue = 5;
  exprService.init();

  // Try to go below 0
  exprService.handleEvent(makeUIExprEvent(ExprParam::kToe, -10));
  TEST_ASSERT_EQUAL(0, logicalState.m_exprParams[0].m_toeValue);
  assertExprSaveEventPublished();
  assertEventBusEmpty();

  // Try to go above 1023
  logicalState.m_exprParams[0].m_toeValue = 1020;
  exprService.init();
  exprService.handleEvent(makeUIExprEvent(ExprParam::kToe, 10));
  TEST_ASSERT_EQUAL(1023, logicalState.m_exprParams[0].m_toeValue);
  assertExprSaveEventPublished();
  assertEventBusEmpty();
}

void test_ui_updates_correct_program() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  // Configure program 1
  logicalState.m_currentProgram = 1;
  exprService.init();

  exprService.handleEvent(makeUIExprEvent(ExprParam::kState));

  // Program 1 should be updated, not program 0
  TEST_ASSERT_EQUAL(ExprState::kInactive, logicalState.m_exprParams[0].m_state);
  TEST_ASSERT_EQUAL(ExprState::kActive, logicalState.m_exprParams[1].m_state);
}

// =============================================================================
// Value Mapping Tests
// =============================================================================

void test_value_mapping_normal_direction() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[0].m_direction = Direction::kNormal;
  logicalState.m_exprParams[0].m_heelValue = 0;
  logicalState.m_exprParams[0].m_toeValue = 1023;
  exprService.init();

  // ADC 0 should map to heel value (0)
  exprService.handleEvent(makePhysicalExprEvent(0));
  assertLogicExprEventPublished(PotId::kPot0, 0);
  assertEventBusEmpty();

  // ADC 1023 should map to toe value (1023)
  exprService.handleEvent(makePhysicalExprEvent(1023));
  assertLogicExprEventPublished(PotId::kPot0, 1023);
  assertEventBusEmpty();

  // ADC 512 should map to ~512 (middle)
  exprService.handleEvent(makePhysicalExprEvent(512));
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_INT_WITHIN(2, 512, e.m_data.value);
  assertEventBusEmpty();
}

void test_value_mapping_inverted_direction() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[0].m_direction = Direction::kInverted;
  logicalState.m_exprParams[0].m_heelValue = 0;
  logicalState.m_exprParams[0].m_toeValue = 1023;
  exprService.init();

  // ADC 0 should map to toe value (1023) when inverted
  exprService.handleEvent(makePhysicalExprEvent(0));
  assertLogicExprEventPublished(PotId::kPot0, 1023);
  assertEventBusEmpty();

  // ADC 1023 should map to heel value (0) when inverted
  exprService.handleEvent(makePhysicalExprEvent(1023));
  assertLogicExprEventPublished(PotId::kPot0, 0);
  assertEventBusEmpty();
}

void test_value_mapping_custom_range() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[0].m_direction = Direction::kNormal;
  logicalState.m_exprParams[0].m_heelValue = 200;
  logicalState.m_exprParams[0].m_toeValue = 800;
  exprService.init();

  // ADC 0 should map to heel value (200)
  exprService.handleEvent(makePhysicalExprEvent(0));
  assertLogicExprEventPublished(PotId::kPot0, 200);
  assertEventBusEmpty();

  // ADC 1023 should map to toe value (800)
  exprService.handleEvent(makePhysicalExprEvent(1023));
  assertLogicExprEventPublished(PotId::kPot0, 800);
  assertEventBusEmpty();

  // ADC 512 should map to middle of range (~500)
  exprService.handleEvent(makePhysicalExprEvent(512));
  Event e;
  EventBus::recall(e);
  TEST_ASSERT_INT_WITHIN(5, 500, e.m_data.value);
  assertEventBusEmpty();
}

// =============================================================================
// interestedIn Tests
// =============================================================================

void test_interested_in_physical_expr() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  Event e;
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kExpr;

  TEST_ASSERT_TRUE(exprService.interestedIn(e));
}

void test_interested_in_ui_expr() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kExpr;

  TEST_ASSERT_TRUE(exprService.interestedIn(e));
}

void test_interested_in_logic_program_changed() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;

  TEST_ASSERT_TRUE(exprService.interestedIn(e));
}

void test_not_interested_in_other_events() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  Event e;

  // Not interested in physical pot events
  e.m_domain = EventDomain::kPhysical;
  e.m_subject = EventSubject::kPot;
  TEST_ASSERT_FALSE(exprService.interestedIn(e));

  // Not interested in logic expr events (outputs them, doesn't consume)
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(exprService.interestedIn(e));

  // Not interested in MIDI events
  e.m_domain = EventDomain::kMidi;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(exprService.interestedIn(e));

  // Not interested in memory events
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kExpr;
  TEST_ASSERT_FALSE(exprService.interestedIn(e));

  // Not interested in logic program with different action
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kSave;
  TEST_ASSERT_FALSE(exprService.interestedIn(e));
}

int main() {
  UNITY_BEGIN();

  // Init
  RUN_TEST(test_init_when_inactive);
  RUN_TEST(test_init_syncs_handler_from_current_program);

  // Physical Expression Pedal Events
  RUN_TEST(test_physical_expr_ignored_when_disconnected_and_inactive);
  RUN_TEST(test_physical_expr_ignored_when_connected_and_inactive);
  RUN_TEST(test_physical_expr_ignored_when_disconnected_and_active);
  RUN_TEST(test_physical_expr_publishes_logic_event_when_connected_and_active);
  RUN_TEST(test_physical_expr_maps_to_correct_pot_id);
  RUN_TEST(test_physical_expr_connected_sets_logical_state);
  RUN_TEST(test_physical_expr_disconnected_sets_logical_state);

  // Program Change
  RUN_TEST(test_program_change_syncs_handler);
  RUN_TEST(test_program_change_to_inactive_expr);

  // UI Expression Parameter Events
  RUN_TEST(test_ui_toggle_state);
  RUN_TEST(test_ui_change_mapped_pot);
  RUN_TEST(test_ui_change_mapped_pot_wraps);
  RUN_TEST(test_ui_toggle_direction);
  RUN_TEST(test_ui_change_heel_value);
  RUN_TEST(test_ui_change_toe_value);
  RUN_TEST(test_ui_heel_value_clamped);
  RUN_TEST(test_ui_toe_value_clamped);
  RUN_TEST(test_ui_updates_correct_program);

  // Value Mapping
  RUN_TEST(test_value_mapping_normal_direction);
  RUN_TEST(test_value_mapping_inverted_direction);
  RUN_TEST(test_value_mapping_custom_range);

  // interestedIn
  RUN_TEST(test_interested_in_physical_expr);
  RUN_TEST(test_interested_in_ui_expr);
  RUN_TEST(test_interested_in_logic_program_changed);
  RUN_TEST(test_not_interested_in_other_events);

  UNITY_END();
}
