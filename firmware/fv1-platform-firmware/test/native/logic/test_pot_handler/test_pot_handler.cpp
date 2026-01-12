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

// =============================================================================
// Initialization Tests
// =============================================================================

void test_default_initialization() {
  PotHandler potHandler;

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    TEST_ASSERT_EQUAL(PotState::kActive, potHandler.m_state[i]);
  }

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    TEST_ASSERT_EQUAL(0, potHandler.m_minValue[i]);
  }

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    TEST_ASSERT_EQUAL(1023, potHandler.m_maxValue[i]);
  }
}

// =============================================================================
// Enum Validation Tests
// =============================================================================

void test_pot_enums_validation() {
  TEST_ASSERT_TRUE(PotStateValidator::isValid(0));
  TEST_ASSERT_TRUE(PotStateValidator::isValid(1));

  TEST_ASSERT_FALSE(PotStateValidator::isValid(2));

  TEST_ASSERT_EQUAL(PotState::kDisabled, PotStateValidator::sanitize(0, PotState::kActive));
  TEST_ASSERT_EQUAL(PotState::kActive, PotStateValidator::sanitize(1, PotState::kDisabled));
  TEST_ASSERT_EQUAL(PotState::kActive, PotStateValidator::sanitize(4, PotState::kActive));
}

// =============================================================================
// mapMidiValue Tests
// =============================================================================

void test_midi_mapping() {
  PotHandler potHandler;

  TEST_ASSERT_EQUAL(0, potHandler.mapMidiValue(0, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.mapMidiValue(127, 0));
  TEST_ASSERT_EQUAL(515, potHandler.mapMidiValue(64, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.mapMidiValue(254, 0));
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

// =============================================================================
// mapAdcValue Tests
// =============================================================================

void test_adc_mapping() {
  PotHandler potHandler;

  TEST_ASSERT_EQUAL(0, potHandler.mapAdcValue(0, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.mapAdcValue(1023, 0));
  TEST_ASSERT_EQUAL(512, potHandler.mapAdcValue(512, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.mapAdcValue(2048, 0));
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

// =============================================================================
// mapMenuValue Tests
// =============================================================================

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

// =============================================================================
// togglePotState Tests
// =============================================================================

void test_toggle_pot_state() {
  PotHandler potHandler;

  TEST_ASSERT_EQUAL(PotState::kDisabled, potHandler.togglePotState(0));
  TEST_ASSERT_EQUAL(PotState::kActive, potHandler.togglePotState(0));
}

void test_toggle_pot_state_all_pots() {
  PotHandler potHandler;

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    TEST_ASSERT_EQUAL(PotState::kDisabled, potHandler.togglePotState(i));
    TEST_ASSERT_EQUAL(PotState::kActive, potHandler.togglePotState(i));
  }
}

// =============================================================================
// changePotMinValue / changePotMaxValue Tests
// =============================================================================

void test_change_min_value() {
  PotHandler potHandler;

  TEST_ASSERT_EQUAL(1, potHandler.changePotMinValue(1, 0));
  TEST_ASSERT_EQUAL(0, potHandler.changePotMinValue(-1, 0));
  TEST_ASSERT_EQUAL(0, potHandler.changePotMinValue(-1, 0));
}

void test_change_max_value() {
  PotHandler potHandler;

  TEST_ASSERT_EQUAL(1023, potHandler.m_maxValue[0]);
  TEST_ASSERT_EQUAL(1022, potHandler.changePotMaxValue(-1, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.changePotMaxValue(1, 0));
  TEST_ASSERT_EQUAL(1023, potHandler.changePotMaxValue(1, 0));
}

int main() {
  UNITY_BEGIN();

  // Initialization
  RUN_TEST(test_default_initialization);

  // Enum validation
  RUN_TEST(test_pot_enums_validation);

  // mapMidiValue
  RUN_TEST(test_midi_mapping);
  RUN_TEST(test_midi_range);

  // mapAdcValue
  RUN_TEST(test_adc_mapping);
  RUN_TEST(test_adc_range);
  RUN_TEST(test_all_pots);

  // mapMenuValue
  RUN_TEST(test_menu_mapping);
  RUN_TEST(test_menu_range);

  // togglePotState
  RUN_TEST(test_toggle_pot_state);
  RUN_TEST(test_toggle_pot_state_all_pots);

  // changePotMinValue / changePotMaxValue
  RUN_TEST(test_change_min_value);
  RUN_TEST(test_change_max_value);

  UNITY_END();
}
