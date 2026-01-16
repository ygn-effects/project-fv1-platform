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

// =============================================================================
// Initialization Tests
// =============================================================================

void test_default_initialization() {
  TempoHandler tempoHandler;

  TEST_ASSERT_EQUAL(0, tempoHandler.m_interval);
  TEST_ASSERT_EQUAL(0, tempoHandler.m_minInterval);
  TEST_ASSERT_EQUAL(0, tempoHandler.m_maxInterval);
  TEST_ASSERT_EQUAL(TempoSource::kPot, tempoHandler.m_source);
  TEST_ASSERT_TRUE(tempoHandler.m_ledIncreasing);
  TEST_ASSERT_EQUAL(0, tempoHandler.m_ledLastUpdate);
  TEST_ASSERT_EQUAL(0, tempoHandler.m_ledValue);
}

// =============================================================================
// mapInterval Tests
// =============================================================================

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

// =============================================================================
// calculateTempoLedValue Tests
// =============================================================================

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

void test_led_value_no_update_when_elapsed_under_20ms() {
  TempoHandler tempoHandler;

  tempoHandler.m_interval = 500;
  tempoHandler.m_ledValue = 100;
  tempoHandler.m_ledLastUpdate = 0;

  // Elapsed < 20ms, should not update ledValue
  uint8_t result1 = tempoHandler.calculateTempoLedValue(10);
  TEST_ASSERT_EQUAL(0, tempoHandler.m_ledLastUpdate);  // Not updated
  TEST_ASSERT_EQUAL(100, tempoHandler.m_ledValue);     // Unchanged

  uint8_t result2 = tempoHandler.calculateTempoLedValue(19);
  TEST_ASSERT_EQUAL(0, tempoHandler.m_ledLastUpdate);  // Still not updated
  TEST_ASSERT_EQUAL(100, tempoHandler.m_ledValue);     // Still unchanged
}

void test_led_value_reverses_at_peak() {
  TempoHandler tempoHandler;

  tempoHandler.m_interval = 500;
  tempoHandler.m_ledValue = 250;
  tempoHandler.m_ledIncreasing = true;
  tempoHandler.m_ledLastUpdate = 0;

  // Push ledValue over 255, should clamp and reverse direction
  tempoHandler.calculateTempoLedValue(100);

  TEST_ASSERT_EQUAL(255, tempoHandler.m_ledValue);
  TEST_ASSERT_FALSE(tempoHandler.m_ledIncreasing);
}

void test_led_value_reverses_at_zero() {
  TempoHandler tempoHandler;

  tempoHandler.m_interval = 500;
  tempoHandler.m_ledValue = 10;
  tempoHandler.m_ledIncreasing = false;
  tempoHandler.m_ledLastUpdate = 0;

  // delta = (255 * 100) / (500 / 2) = 25500 / 250 = 102
  // Since delta (102) >= ledValue (10), should clamp to 0 and reverse
  tempoHandler.calculateTempoLedValue(100);

  TEST_ASSERT_EQUAL(0, tempoHandler.m_ledValue);
  TEST_ASSERT_TRUE(tempoHandler.m_ledIncreasing);
}

void test_led_value_reverses_at_zero_exact_boundary() {
  TempoHandler tempoHandler;

  tempoHandler.m_interval = 500;
  tempoHandler.m_ledValue = 0;
  tempoHandler.m_ledIncreasing = false;
  tempoHandler.m_ledLastUpdate = 0;

  // delta = (255 * 20) / (500 / 2) = 5100 / 250 = 20
  // Since delta (20) >= ledValue (0), should stay at 0 and reverse
  tempoHandler.calculateTempoLedValue(20);

  TEST_ASSERT_EQUAL(0, tempoHandler.m_ledValue);
  TEST_ASSERT_TRUE(tempoHandler.m_ledIncreasing);
}

void test_led_value_decreasing() {
  TempoHandler tempoHandler;

  tempoHandler.m_interval = 500;
  tempoHandler.m_ledValue = 200;
  tempoHandler.m_ledIncreasing = false;
  tempoHandler.m_ledLastUpdate = 0;

  uint8_t result = tempoHandler.calculateTempoLedValue(20);

  // LED should have decreased
  TEST_ASSERT_TRUE(tempoHandler.m_ledValue < 200);
  TEST_ASSERT_FALSE(tempoHandler.m_ledIncreasing);
}

int main() {
  UNITY_BEGIN();

  // Initialization
  RUN_TEST(test_default_initialization);

  // mapInterval
  RUN_TEST(test_tap_mapping);
  RUN_TEST(test_pot_mapping);
  RUN_TEST(test_menu_mapping);

  // calculateTempoLedValue
  RUN_TEST(test_led_value);
  RUN_TEST(test_led_value_no_update_when_elapsed_under_20ms);
  RUN_TEST(test_led_value_reverses_at_peak);
  RUN_TEST(test_led_value_reverses_at_zero);
  RUN_TEST(test_led_value_reverses_at_zero_exact_boundary);
  RUN_TEST(test_led_value_decreasing);

  UNITY_END();
}
