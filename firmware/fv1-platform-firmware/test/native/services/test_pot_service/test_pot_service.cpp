#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/pot_service.h"

#include "../src/services/pot_service.cpp"
#include "../src/logic/pot_handler.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_pot_adc() {
  LogicalState logicalState;
  PotService potService(logicalState);

  potService.init();

  potService.handleEvent({EventType::kPot0Moved, 100, {.value=100}});
  potService.handleEvent({EventType::kPot1Moved, 200, {.value=200}});
  potService.handleEvent({EventType::kPot2Moved, 300, {.value=300}});
  potService.handleEvent({EventType::kMixPotMoved, 400, {.value=400}});

  TEST_ASSERT_EQUAL(100, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
  TEST_ASSERT_EQUAL(200, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);
  TEST_ASSERT_EQUAL(300, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);
  TEST_ASSERT_EQUAL(400, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);
}

void test_pot_adc_mapping() {
  LogicalState logicalState;
  PotService potService(logicalState);

  logicalState.m_potParams[0][2].m_minValue = 200;
  logicalState.m_potParams[0][2].m_maxValue = 800;

  potService.init();

  potService.handleEvent({EventType::kPot0Moved, 100, {.value=100}});
  potService.handleEvent({EventType::kPot1Moved, 200, {.value=200}});
  potService.handleEvent({EventType::kPot2Moved, 300, {.value=300}});
  potService.handleEvent({EventType::kMixPotMoved, 400, {.value=400}});

  TEST_ASSERT_EQUAL(100, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
  TEST_ASSERT_EQUAL(200, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);
  TEST_ASSERT_EQUAL(375, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);
  TEST_ASSERT_EQUAL(400, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);
}

void test_pot_menu() {
  LogicalState logicalState;
  PotService potService(logicalState);

  logicalState.m_potParams[0][2].m_value = 300;
  logicalState.m_potParams[0][2].m_minValue = 200;
  logicalState.m_potParams[0][2].m_maxValue = 800;

  potService.init();

  potService.handleEvent({EventType::kMenuPot0Moved, 0, {.delta=1}});
  potService.handleEvent({EventType::kMenuPot1Moved, 0, {.delta=-1}});
  potService.handleEvent({EventType::kMenuPot2Moved, 0, {.delta=1}});
  potService.handleEvent({EventType::kMenuMixPotMoved, 0, {.delta=-1}});

  TEST_ASSERT_EQUAL(1, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
  TEST_ASSERT_EQUAL(0, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);
  TEST_ASSERT_EQUAL(301, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);
  TEST_ASSERT_EQUAL(0, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);
}

void test_pot_midi() {
  LogicalState logicalState;
  PotService potService(logicalState);

  logicalState.m_potParams[0][2].m_value = 300;
  logicalState.m_potParams[0][2].m_minValue = 200;
  logicalState.m_potParams[0][2].m_maxValue = 800;

  potService.init();

  potService.handleEvent({EventType::kMidiPot0Moved, 0, {.value=127}});
  potService.handleEvent({EventType::kMidiPot1Moved, 0, {.value=64}});
  potService.handleEvent({EventType::kMidiPot2Moved, 0, {.value=96}});
  potService.handleEvent({EventType::kMixPotMoved, 0, {.value=0}});

  TEST_ASSERT_EQUAL(1023, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
  TEST_ASSERT_EQUAL(515, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);
  TEST_ASSERT_EQUAL(653, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);
  TEST_ASSERT_EQUAL(0, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);
}

void test_pot_expr() {
  LogicalState logicalState;
  PotService potService(logicalState);

  logicalState.m_potParams[0][2].m_value = 300;
  logicalState.m_potParams[0][2].m_minValue = 200;
  logicalState.m_potParams[0][2].m_maxValue = 800;

  potService.init();

  potService.handleEvent({EventType::kExprPot0Moved, 0, {.value=127}});
  potService.handleEvent({EventType::kExprPot1Moved, 0, {.value=256}});
  potService.handleEvent({EventType::kExprPot2Moved, 0, {.value=768}});
  potService.handleEvent({EventType::kExprMixPotMoved, 0, {.value=1023}});

  TEST_ASSERT_EQUAL(127, logicalState.m_potParams[logicalState.m_currentProgram][0].m_value);
  TEST_ASSERT_EQUAL(256, logicalState.m_potParams[logicalState.m_currentProgram][1].m_value);
  TEST_ASSERT_EQUAL(768, logicalState.m_potParams[logicalState.m_currentProgram][2].m_value);
  TEST_ASSERT_EQUAL(1023, logicalState.m_potParams[logicalState.m_currentProgram][3].m_value);
}

void test_interested_in() {
  LogicalState logicalState;
  PotService potService(logicalState);

  Event e{EventType::kPot0Moved, 500, {.value=100}};
  TEST_ASSERT_TRUE(potService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kMenuProgramChanged, 500, {.delta=1}};
  TEST_ASSERT_FALSE(potService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kMenuExprMappedPotMoved, 500, {.delta=1}};
  TEST_ASSERT_FALSE(potService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_pot_adc);
  RUN_TEST(test_pot_adc_mapping);
  RUN_TEST(test_pot_menu);
  RUN_TEST(test_pot_midi);
  RUN_TEST(test_pot_expr);
  RUN_TEST(test_interested_in);
  UNITY_END();
}
