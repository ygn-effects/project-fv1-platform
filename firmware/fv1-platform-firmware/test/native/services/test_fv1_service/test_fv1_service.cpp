#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/fv1_handler.h"
#include "services/program_service.h"
#include "mock/mock_fv1.h"

#include "../src/logic/fv1_handler.cpp"
#include "../src/services/fv1_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_program_change() {
  LogicalState logicalState;

  MockFv1 mockFv1;
  Fv1Service fv1service(logicalState, mockFv1);

  logicalState.m_currentProgram = 1;
  fv1service.handleEvent({EventType::kProgramChanged, 0, {}});

  TEST_ASSERT_EQUAL(1, mockFv1.m_s0);
  TEST_ASSERT_EQUAL(0, mockFv1.m_s1);
  TEST_ASSERT_EQUAL(0, mockFv1.m_s2);

  logicalState.m_currentProgram = 4;
  fv1service.handleEvent({EventType::kProgramChanged, 0, {}});

  TEST_ASSERT_EQUAL(0, mockFv1.m_s0);
  TEST_ASSERT_EQUAL(0, mockFv1.m_s1);
  TEST_ASSERT_EQUAL(1, mockFv1.m_s2);

  logicalState.m_currentProgram = 7;
  fv1service.handleEvent({EventType::kProgramChanged, 0, {}});

  TEST_ASSERT_EQUAL(1, mockFv1.m_s0);
  TEST_ASSERT_EQUAL(1, mockFv1.m_s1);
  TEST_ASSERT_EQUAL(1, mockFv1.m_s2);
}

void test_change_tempo() {
  LogicalState logicalState;

  MockFv1 mockFv1;
  Fv1Service fv1service(logicalState, mockFv1);

  fv1service.init();

  logicalState.m_tempo = 512;

  fv1service.handleEvent({EventType::kTempoChanged, 0, {}});

  auto [pot, val] = mockFv1.m_potValues.back();
  TEST_ASSERT_EQUAL(513, val);
  TEST_ASSERT_EQUAL(Fv1Pot::Pot0, pot);
}

void test_change_pot0_delay() {
  LogicalState logicalState;

  MockFv1 mockFv1;
  Fv1Service fv1service(logicalState, mockFv1);

  TEST_ASSERT_TRUE(logicalState.m_activeProgram->m_isDelayEffect);

  logicalState.m_potParams[0]->m_value = 512;

  fv1service.handleEvent({EventType::kPot0Moved, 0, {}});

  TEST_ASSERT_TRUE(mockFv1.m_potValues.empty());
}

void test_change_pot1() {
  LogicalState logicalState;

  MockFv1 mockFv1;
  Fv1Service fv1service(logicalState, mockFv1);

  logicalState.m_potParams[1]->m_value = 512;

  fv1service.handleEvent({EventType::kPot1Moved, 0, {}});

  auto [pot, val] = mockFv1.m_potValues.back();
  TEST_ASSERT_EQUAL(512, val);
  TEST_ASSERT_EQUAL(Fv1Pot::Pot1, pot);
}

void test_interested_in() {

}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_program_change);
  RUN_TEST(test_change_tempo);
  RUN_TEST(test_change_pot0_delay);
  RUN_TEST(test_change_pot1);
  UNITY_END();
}
