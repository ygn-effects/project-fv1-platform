#include <unity.h>
#include "core/event_bus.h"

#include "../src/logic/pot_handler.cpp"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_pot_enums_validation() {
  TEST_ASSERT_TRUE(PotStateValidator::isValid(0));
  TEST_ASSERT_TRUE(PotStateValidator::isValid(1));

  TEST_ASSERT_FALSE(PotStateValidator::isValid(2));

  TEST_ASSERT_EQUAL(PotState::kDisabled, PotStateValidator::sanitize(0, PotState::kActive));
  TEST_ASSERT_EQUAL(PotState::kActive, PotStateValidator::sanitize(1, PotState::kDisabled));
  TEST_ASSERT_EQUAL(PotState::kActive, PotStateValidator::sanitize(4, PotState::kActive));
}

void test_midi_mapping() {
  PotHandler potHandler;

  TEST_ASSERT_EQUAL(0, potHandler.mapMidiValue(0, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.mapMidiValue(127, 0));
  TEST_ASSERT_EQUAL(515, potHandler.mapMidiValue(64, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.mapMidiValue(254, 0));
}

void test_adc_mapping() {
  PotHandler potHandler;

  TEST_ASSERT_EQUAL(0, potHandler.mapAdcValue(0, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.mapAdcValue(1023, 0));
  TEST_ASSERT_EQUAL(512, potHandler.mapAdcValue(512, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.mapAdcValue(2048, 0));
}

void test_midi_range() {
  PotHandler potHandler;
  potHandler.m_minValue[0] = 200;
  potHandler.m_maxValue[0] = 600;

  TEST_ASSERT_EQUAL(200, potHandler.mapMidiValue(0, 0));
  TEST_ASSERT_EQUAL(600, potHandler.mapMidiValue(127, 0));
  TEST_ASSERT_EQUAL(401, potHandler.mapMidiValue(64, 0));
  TEST_ASSERT_EQUAL(600, potHandler.mapMidiValue(254, 0));
}

void test_adc_range() {
  PotHandler potHandler;
  potHandler.m_minValue[0] = 200;
  potHandler.m_maxValue[0] = 600;

  TEST_ASSERT_EQUAL(200, potHandler.mapAdcValue(0, 0));
  TEST_ASSERT_EQUAL(600, potHandler.mapAdcValue(1023, 0));
  TEST_ASSERT_EQUAL(400, potHandler.mapAdcValue(512, 0));
  TEST_ASSERT_EQUAL(600, potHandler.mapAdcValue(2048, 0));
}

void test_all_pots() {
  PotHandler potHandler;
  potHandler.m_minValue[0] = 0;
  potHandler.m_maxValue[0] = 1023;
  potHandler.m_minValue[1] = 0;
  potHandler.m_maxValue[1] = 1023;
  potHandler.m_minValue[2] = 0;
  potHandler.m_maxValue[2] = 1023;
  potHandler.m_minValue[3] = 0;
  potHandler.m_maxValue[3] = 1023;

  TEST_ASSERT_EQUAL(0, potHandler.mapAdcValue(0, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.mapAdcValue(1023, 0));
  TEST_ASSERT_EQUAL(512, potHandler.mapAdcValue(512, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.mapAdcValue(2048, 0));

  TEST_ASSERT_EQUAL(0, potHandler.mapAdcValue(0, 1));
  TEST_ASSERT_EQUAL(1023, potHandler.mapAdcValue(1023, 1));
  TEST_ASSERT_EQUAL(512, potHandler.mapAdcValue(512, 1));
  TEST_ASSERT_EQUAL(1023, potHandler.mapAdcValue(2048, 1));

  TEST_ASSERT_EQUAL(0, potHandler.mapAdcValue(0, 2));
  TEST_ASSERT_EQUAL(1023, potHandler.mapAdcValue(1023, 2));
  TEST_ASSERT_EQUAL(512, potHandler.mapAdcValue(512, 2));
  TEST_ASSERT_EQUAL(1023, potHandler.mapAdcValue(2048, 2));

  TEST_ASSERT_EQUAL(0, potHandler.mapAdcValue(0, 3));
  TEST_ASSERT_EQUAL(1023, potHandler.mapAdcValue(1023, 3));
  TEST_ASSERT_EQUAL(512, potHandler.mapAdcValue(512, 3));
  TEST_ASSERT_EQUAL(1023, potHandler.mapAdcValue(2048, 3));
}

void test_menu_mapping() {
  PotHandler potHandler;

  TEST_ASSERT_EQUAL(501, potHandler.mapMenuValue(500, 1, 0));
  TEST_ASSERT_EQUAL(499, potHandler.mapMenuValue(500, -1, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.mapMenuValue(1023, 1, 0));
  TEST_ASSERT_EQUAL(0, potHandler.mapMenuValue(0, -1, 0));
}

void test_menu_range() {
  PotHandler potHandler;
  potHandler.m_minValue[0] = 200;
  potHandler.m_maxValue[0] = 600;

  TEST_ASSERT_EQUAL(501, potHandler.mapMenuValue(500, 1, 0));
  TEST_ASSERT_EQUAL(499, potHandler.mapMenuValue(500, -1, 0));
  TEST_ASSERT_EQUAL(600, potHandler.mapMenuValue(1023, 1, 0));
  TEST_ASSERT_EQUAL(200, potHandler.mapMenuValue(100, -1, 0));
}

void test_toggle_pot_state() {
  PotHandler potHandler;

  TEST_ASSERT_EQUAL(PotState::kDisabled, potHandler.togglePotState(0));
  TEST_ASSERT_EQUAL(PotState::kActive, potHandler.togglePotState(0));
}

void test_change_min_value() {
  PotHandler potHandler;

  TEST_ASSERT_EQUAL(1, potHandler.changePotMinValue(1, 0));
  TEST_ASSERT_EQUAL(0, potHandler.changePotMinValue(-1, 0));
  TEST_ASSERT_EQUAL(0, potHandler.changePotMinValue(-1, 0));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_pot_enums_validation);
  RUN_TEST(test_midi_mapping);
  RUN_TEST(test_adc_mapping);
  RUN_TEST(test_midi_range);
  RUN_TEST(test_adc_range);
  RUN_TEST(test_all_pots);
  RUN_TEST(test_menu_mapping);
  RUN_TEST(test_menu_range);
  RUN_TEST(test_toggle_pot_state);
  RUN_TEST(test_change_min_value);
  UNITY_END();
}
