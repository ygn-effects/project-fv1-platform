#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/program_service.h"

#include "../src/services/program_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_transition_relative() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.handleEvent({EventType::kMenuProgramChanged, 1000, {.delta = 1}});
  TEST_ASSERT_EQUAL(1, logicalState.m_currentProgram);

  programService.handleEvent({EventType::kMenuProgramChanged, 1000, {.delta = 1}});
  TEST_ASSERT_EQUAL(2, logicalState.m_currentProgram);

  programService.handleEvent({EventType::kMenuProgramChanged, 1000, {.delta = -1}});
  TEST_ASSERT_EQUAL(1, logicalState.m_currentProgram);

  programService.handleEvent({EventType::kMenuProgramChanged, 1000, {.delta = -1}});
  TEST_ASSERT_EQUAL(0, logicalState.m_currentProgram);
}

void test_wrap_around() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.handleEvent({EventType::kMenuProgramChanged, 1000, {.delta = -1}});
  TEST_ASSERT_EQUAL(7, logicalState.m_currentProgram);

  programService.handleEvent({EventType::kMenuProgramChanged, 1000, {.delta = 1}});
  TEST_ASSERT_EQUAL(0, logicalState.m_currentProgram);
}

void test_out_of_bounds() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.handleEvent({EventType::kMenuProgramChanged, 1000, {.delta = 5}});
  TEST_ASSERT_EQUAL(5, logicalState.m_currentProgram);

  programService.handleEvent({EventType::kMenuProgramChanged, 1000, {.delta = 100}});
  TEST_ASSERT_EQUAL(5, logicalState.m_currentProgram);

  programService.handleEvent({EventType::kMenuProgramChanged, 1000, {.delta = -100}});
  TEST_ASSERT_EQUAL(5, logicalState.m_currentProgram);
}

void test_init_program() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.init();
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[0], logicalState.m_activeProgram);
}

void test_pointer_transition() {
  LogicalState logicalState;
  ProgramService programService(logicalState);

  programService.init();
  programService.handleEvent({EventType::kMenuProgramChanged, 1000, {.delta = 1}});
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[1], logicalState.m_activeProgram);

  programService.init();
  programService.handleEvent({EventType::kMenuProgramChanged, 1000, {.delta = 2}});
  TEST_ASSERT_EQUAL_PTR(&ProgramsDefinitions::kPrograms[3], logicalState.m_activeProgram);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_transition_relative);
  RUN_TEST(test_wrap_around);
  RUN_TEST(test_out_of_bounds);
  RUN_TEST(test_init_program);
  RUN_TEST(test_pointer_transition);
  UNITY_END();
}
