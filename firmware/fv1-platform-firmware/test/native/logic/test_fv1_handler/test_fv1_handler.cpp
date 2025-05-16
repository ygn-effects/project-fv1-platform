#include <unity.h>
#include "core/event_bus.h"
#include "logic/fv1_handler.h"

#include "../src/logic/fv1_handler.cpp"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_tempo_mapping() {
  Fv1Handler fv1Handler;

  fv1Handler.m_tempoConfig.m_minLogical = 200;
  fv1Handler.m_tempoConfig.m_maxLogical = 800;

  uint16_t value = fv1Handler.mapTempoValue(200);
  TEST_ASSERT_EQUAL(0, value);

  value = fv1Handler.mapTempoValue(800);
  TEST_ASSERT_EQUAL(1023, value);

  value = fv1Handler.mapTempoValue(500);
  TEST_ASSERT_EQUAL(511, value);
}

void test_pot_mapping() {
  Fv1Handler fv1Handler;

  fv1Handler.m_potConfigs[0].m_minLogical = 200;
  fv1Handler.m_potConfigs[0].m_maxLogical = 800;

  uint16_t value = fv1Handler.mapPotValue(0, 0);
  TEST_ASSERT_EQUAL(200, value);

  value = fv1Handler.mapPotValue(0, 1023);
  TEST_ASSERT_EQUAL(800, value);

  value = fv1Handler.mapPotValue(0, 512);
  TEST_ASSERT_EQUAL(500, value);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_tempo_mapping);
  RUN_TEST(test_pot_mapping);
  UNITY_END();
}