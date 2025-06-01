#include <unity.h>
#include "core/event_bus.h"

#include "../src/logic/tempo_handler.cpp"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_tap_mapping() {
  TempoHandler tempoHandler;

  tempoHandler.m_minInterval = 0;
  tempoHandler.m_maxInterval = 1000;
  tempoHandler.m_source = TempoSource::kTap;

  TEST_ASSERT_EQUAL(500, tempoHandler.mapInterval(500));

  tempoHandler.m_minInterval = 100;
  tempoHandler.m_maxInterval = 800;

  TEST_ASSERT_EQUAL(100, tempoHandler.mapInterval(50));
  TEST_ASSERT_EQUAL(800, tempoHandler.mapInterval(900));
}

void test_pot_mapping() {
  TempoHandler tempoHandler;

  tempoHandler.m_minInterval = 0;
  tempoHandler.m_maxInterval = 1000;
  tempoHandler.m_source = TempoSource::kPot;

  TEST_ASSERT_EQUAL(0, tempoHandler.mapInterval(0));
  TEST_ASSERT_EQUAL(501, tempoHandler.mapInterval(513));
  TEST_ASSERT_EQUAL(1000, tempoHandler.mapInterval(1023));

  tempoHandler.m_minInterval = 100;
  tempoHandler.m_maxInterval = 800;

  TEST_ASSERT_EQUAL(100, tempoHandler.mapInterval(0));
  TEST_ASSERT_EQUAL(451, tempoHandler.mapInterval(513));
  TEST_ASSERT_EQUAL(800, tempoHandler.mapInterval(1023));
}

void test_menu_mapping() {
  TempoHandler tempoHandler;

  tempoHandler.m_minInterval = 0;
  tempoHandler.m_interval = 500;
  tempoHandler.m_maxInterval = 1000;
  tempoHandler.m_source = TempoSource::kMenu;

  TEST_ASSERT_EQUAL(501, tempoHandler.mapInterval(1));
  TEST_ASSERT_EQUAL(500, tempoHandler.mapInterval(-1));

  tempoHandler.m_minInterval = 100;
  tempoHandler.m_interval = 799;
  tempoHandler.m_maxInterval = 800;

  TEST_ASSERT_EQUAL(800, tempoHandler.mapInterval(1));
  TEST_ASSERT_EQUAL(800, tempoHandler.mapInterval(1));

  tempoHandler.m_interval = 101;

  TEST_ASSERT_EQUAL(100, tempoHandler.mapInterval(-1));
  TEST_ASSERT_EQUAL(100, tempoHandler.mapInterval(-1));
}

void test_led_value() {
  TempoHandler tempoHandler;

  tempoHandler.m_minInterval = 0;
  tempoHandler.m_maxInterval = 1000;
  tempoHandler.m_interval = 500;
  tempoHandler.m_source = TempoSource::kPot;

  TEST_ASSERT_EQUAL(32, tempoHandler.calculateTempoLedValue(0));
  TEST_ASSERT_EQUAL(49, tempoHandler.calculateTempoLedValue(20));
  TEST_ASSERT_EQUAL(66, tempoHandler.calculateTempoLedValue(40));
  TEST_ASSERT_EQUAL(84, tempoHandler.calculateTempoLedValue(60));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_tap_mapping);
  RUN_TEST(test_pot_mapping);
  RUN_TEST(test_menu_mapping);
  RUN_TEST(test_led_value);
  UNITY_END();
}
