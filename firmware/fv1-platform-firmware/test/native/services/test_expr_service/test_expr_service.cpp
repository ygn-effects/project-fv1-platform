#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/expr_service.h"
#include "services/program_service.h"

#include "../src/services/expr_service.cpp"
#include "../src/services/program_service.cpp"
#include "../src/logic/expr_handler.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_expr_inactive() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  exprService.handleEvent({EventType::kExprMoved, 1000, {.value = 512}});
  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

void test_expr_active() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  exprService.init();

  exprService.handleEvent({EventType::kExprMoved, 1000, {.value = 512}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event potMovedEvent;
  EventBus::recall(potMovedEvent);

  TEST_ASSERT_EQUAL(512, potMovedEvent.m_data.value);
  TEST_ASSERT_EQUAL(EventType::kExprPot0Moved, potMovedEvent.m_type);
}

void test_expr_mapped_pots() {
  LogicalState logicalState;
  ExprService exprService(logicalState);
  ProgramService programService(logicalState);

  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  logicalState.m_exprParams[1].m_state = ExprState::kActive;
  logicalState.m_exprParams[2].m_state = ExprState::kActive;
  logicalState.m_exprParams[3].m_state = ExprState::kActive;

  logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[1].m_mappedPot = MappedPot::kPot1;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot2;
  logicalState.m_exprParams[3].m_mappedPot = MappedPot::kMixPot;
  exprService.init();

  exprService.handleEvent({EventType::kExprMoved, 1000, {.value = 512}});
  Event potMovedEvent;
  EventBus::recall(potMovedEvent);
  TEST_ASSERT_EQUAL(EventType::kExprPot0Moved, potMovedEvent.m_type);

  programService.handleEvent({EventType::kMenuProgramChanged, 1100, {.delta = 1}});
  EventBus::recall(potMovedEvent); // Program change
  EventBus::recall(potMovedEvent); // Program change
  exprService.handleEvent({EventType::kProgramChanged, 1100, {.delta = 1}});

  exprService.handleEvent({EventType::kExprMoved, 1000, {.value = 512}});
  EventBus::recall(potMovedEvent);
  TEST_ASSERT_EQUAL(EventType::kExprPot1Moved, potMovedEvent.m_type);

  programService.handleEvent({EventType::kMenuProgramChanged, 1100, {.delta = 1}});
  EventBus::recall(potMovedEvent); // Program change
  EventBus::recall(potMovedEvent); // Program change
  exprService.handleEvent({EventType::kProgramChanged, 1100, {.delta = 1}});

  exprService.handleEvent({EventType::kExprMoved, 1000, {.value = 512}});
  EventBus::recall(potMovedEvent);
  TEST_ASSERT_EQUAL(EventType::kExprPot2Moved, potMovedEvent.m_type);

  programService.handleEvent({EventType::kMenuProgramChanged, 1100, {.delta = 1}});
  EventBus::recall(potMovedEvent); // Program change
  EventBus::recall(potMovedEvent); // Program change
  exprService.handleEvent({EventType::kProgramChanged, 1100, {.delta = 1}});

  exprService.handleEvent({EventType::kExprMoved, 1000, {.value = 512}});
  EventBus::recall(potMovedEvent);
  TEST_ASSERT_EQUAL(EventType::kExprMixPotMoved, potMovedEvent.m_type);
}

void test_expr_direction() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  logicalState.m_exprParams[0].m_direction = Direction::kInverted;
  exprService.init();

  exprService.handleEvent({EventType::kExprMoved, 1000, {.value = 200}});
  Event potMovedEvent;
  EventBus::recall(potMovedEvent);
  TEST_ASSERT_EQUAL(823, potMovedEvent.m_data.value);
}

void test_expr_heel_toe_value() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  logicalState.m_exprParams[0].m_heelValue = 200;
  logicalState.m_exprParams[0].m_toeValue = 300;
  exprService.init();

  exprService.handleEvent({EventType::kExprMoved, 1000, {.value = 100}});
  Event potMovedEvent;
  EventBus::recall(potMovedEvent);
  TEST_ASSERT_EQUAL(210, potMovedEvent.m_data.value);
}

void test_expr_clamping() {
  LogicalState logicalState;
  ExprService exprService(logicalState);

  logicalState.m_exprParams[0].m_state = ExprState::kActive;
  exprService.init();

  exprService.handleEvent({EventType::kExprMoved, 1000, {.value = 2000}});
  Event potMovedEvent;
  EventBus::recall(potMovedEvent);
  TEST_ASSERT_EQUAL(1023, potMovedEvent.m_data.value);
}

void test_interested_in() {
  LogicalState logicalState;
  ExprService bypassService(logicalState);

  Event e{EventType::kExprMoved, 500, {.value=100}};
  TEST_ASSERT_TRUE(bypassService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kProgramChanged, 500, {.delta=1}};
  TEST_ASSERT_TRUE(bypassService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kPot0Moved, 500, {.delta=1}};
  TEST_ASSERT_FALSE(bypassService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kMenuEncoderMoved, 500, {.delta=1}};
  TEST_ASSERT_FALSE(bypassService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_expr_inactive);
  RUN_TEST(test_expr_active);
  RUN_TEST(test_expr_mapped_pots);
  RUN_TEST(test_expr_direction);
  RUN_TEST(test_expr_heel_toe_value);
  RUN_TEST(test_expr_clamping);
  RUN_TEST(test_interested_in);
  UNITY_END();
}