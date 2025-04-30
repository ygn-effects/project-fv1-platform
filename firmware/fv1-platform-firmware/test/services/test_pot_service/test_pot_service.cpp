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

void test_pot() {
  LogicalState logicalState;
  PotService potService(logicalState);

  potService.handleEvent({EventType::kPot0Moved, 100, {.value=100}});
  potService.handleEvent({EventType::kPot1Moved, 200, {.value=200}});
  potService.handleEvent({EventType::kPot2Moved, 300, {.value=300}});
  potService.handleEvent({EventType::kMixPotMoved, 400, {.value=400}});

  TEST_ASSERT_EQUAL(100, logicalState.m_potParams[0].m_value);
  TEST_ASSERT_EQUAL(200, logicalState.m_potParams[1].m_value);
  TEST_ASSERT_EQUAL(300, logicalState.m_potParams[2].m_value);
  TEST_ASSERT_EQUAL(400, logicalState.m_potParams[3].m_value);
}

void test_interested_in() {
  LogicalState logicalState;
  PotService potService(logicalState);

  Event e{EventType::kPot0Moved, 500, {.value=100}};
  TEST_ASSERT_TRUE(potService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kProgramChanged, 500, {.delta=1}};
  TEST_ASSERT_FALSE(potService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kMenuExprMappedPotMoved, 500, {.delta=1}};
  TEST_ASSERT_FALSE(potService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));
}


int main() {
  UNITY_BEGIN();
  RUN_TEST(test_pot);
  RUN_TEST(test_interested_in);
  UNITY_END();
}
