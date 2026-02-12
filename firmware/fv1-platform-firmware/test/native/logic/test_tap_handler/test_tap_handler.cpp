#include <unity.h>
#include "core/event_bus.h"

#include "../src/logic/tap_handler.cpp"


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
  TapHandler tapHandler;

  TEST_ASSERT_EQUAL(TapState::kDisabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, tapHandler.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, tapHandler.m_divValue);
  TEST_ASSERT_EQUAL(0, tapHandler.m_interval);
  TEST_ASSERT_EQUAL(0, tapHandler.m_divInterval);
  TEST_ASSERT_EQUAL(1000, tapHandler.m_tapTimeout);
  TEST_ASSERT_FALSE(tapHandler.m_isNewIntervalSet);
}

// =============================================================================
// Enum Validation Tests
// =============================================================================

void test_tap_enums_validation() {
  TEST_ASSERT_TRUE(TapStateValidator::isValid(0));
  TEST_ASSERT_TRUE(TapStateValidator::isValid(1));

  TEST_ASSERT_TRUE(DivStateValidator::isValid(0));
  TEST_ASSERT_TRUE(DivStateValidator::isValid(1));

  TEST_ASSERT_TRUE(DivValueValidator::isValid(0));
  TEST_ASSERT_TRUE(DivValueValidator::isValid(1));
  TEST_ASSERT_TRUE(DivValueValidator::isValid(2));
  TEST_ASSERT_TRUE(DivValueValidator::isValid(3));
  TEST_ASSERT_TRUE(DivValueValidator::isValid(4));

  TEST_ASSERT_FALSE(TapStateValidator::isValid(2));
  TEST_ASSERT_FALSE(DivStateValidator::isValid(2));
  TEST_ASSERT_FALSE(DivValueValidator::isValid(6));

  TEST_ASSERT_EQUAL(TapState::kDisabled, TapStateValidator::sanitize(0, TapState::kEnabled));
  TEST_ASSERT_EQUAL(TapState::kEnabled, TapStateValidator::sanitize(1, TapState::kDisabled));
  TEST_ASSERT_EQUAL(TapState::kEnabled, TapStateValidator::sanitize(4, TapState::kEnabled));

  TEST_ASSERT_EQUAL(DivState::kDisabled, DivStateValidator::sanitize(0, DivState::kEnabled));
  TEST_ASSERT_EQUAL(DivState::kEnabled, DivStateValidator::sanitize(1, DivState::kDisabled));
  TEST_ASSERT_EQUAL(DivState::kEnabled, DivStateValidator::sanitize(4, DivState::kEnabled));

  TEST_ASSERT_EQUAL(DivValue::kQuarter, DivValueValidator::sanitize(0, DivValue::kDottedEight));
  TEST_ASSERT_EQUAL(DivValue::kEight, DivValueValidator::sanitize(1, DivValue::kEightTriplet));
  TEST_ASSERT_EQUAL(DivValue::kSixteenth, DivValueValidator::sanitize(6, DivValue::kSixteenth));
}

// =============================================================================
// registerTap Tests
// =============================================================================

void test_register_tap_first_tap_does_not_enable() {
  TapHandler tapHandler;

  tapHandler.registerTap(1000);

  TEST_ASSERT_EQUAL(TapState::kDisabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(0, tapHandler.m_interval);
  TEST_ASSERT_FALSE(tapHandler.m_isNewIntervalSet);
}

void test_register_tap() {
  TapHandler tapHandler;

  TEST_ASSERT_EQUAL(TapState::kDisabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, tapHandler.m_divState);

  tapHandler.registerTap(200);
  tapHandler.registerTap(400);

  TEST_ASSERT_TRUE(tapHandler.m_isNewIntervalSet);
  TEST_ASSERT_EQUAL(TapState::kEnabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(200, tapHandler.m_interval);

  TEST_ASSERT_EQUAL(DivState::kDisabled, tapHandler.m_divState);
}

void test_register_tap_timeout_resets_sequence() {
  TapHandler tapHandler;
  tapHandler.m_tapTimeout = 500;

  tapHandler.registerTap(1000);
  tapHandler.registerTap(1200);
  TEST_ASSERT_EQUAL(TapState::kEnabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(200, tapHandler.m_interval);
  TEST_ASSERT_TRUE(tapHandler.m_isNewIntervalSet);

  // Tap after timeout (1200 + 600 = 1800, delta = 600 > 500 timeout)
  tapHandler.registerTap(1800);

  TEST_ASSERT_EQUAL(TapState::kDisabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(0, tapHandler.m_interval);
  TEST_ASSERT_FALSE(tapHandler.m_isNewIntervalSet);
}

void test_register_tap_multiple_taps_averages_interval() {
  TapHandler tapHandler;

  tapHandler.registerTap(1000);  // First
  tapHandler.registerTap(1500);  // +500ms
  TEST_ASSERT_EQUAL(500, tapHandler.m_interval);

  tapHandler.registerTap(2000);  // +500ms
  TEST_ASSERT_EQUAL(500, tapHandler.m_interval);

  tapHandler.registerTap(2600);  // +600ms
  // Interval = (2600 - 1000) / 3 = 533ms
  TEST_ASSERT_EQUAL(533, tapHandler.m_interval);
}

// =============================================================================
// setNextDivValue Tests
// =============================================================================

void test_div() {
  TapHandler tapHandler;

  TEST_ASSERT_EQUAL(TapState::kDisabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, tapHandler.m_divState);

  tapHandler.registerTap(200);
  tapHandler.registerTap(400);

  TEST_ASSERT_EQUAL(TapState::kEnabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, tapHandler.m_divState);

  tapHandler.setNextDivValue();

  TEST_ASSERT_EQUAL(TapState::kEnabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, tapHandler.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, tapHandler.m_divValue);
  TEST_ASSERT_EQUAL(200, tapHandler.m_interval);
  TEST_ASSERT_EQUAL(100, tapHandler.m_divInterval);
}

void test_div_values() {
  TapHandler tapHandler;

  TEST_ASSERT_EQUAL(TapState::kDisabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, tapHandler.m_divState);

  tapHandler.registerTap(200);
  tapHandler.registerTap(400);

  TEST_ASSERT_EQUAL(TapState::kEnabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, tapHandler.m_divState);

  tapHandler.setNextDivValue();

  TEST_ASSERT_EQUAL(TapState::kEnabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, tapHandler.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, tapHandler.m_divValue);
  TEST_ASSERT_EQUAL(200, tapHandler.m_interval);
  TEST_ASSERT_EQUAL(100, tapHandler.m_divInterval);

  tapHandler.setNextDivValue();

  TEST_ASSERT_EQUAL(TapState::kEnabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, tapHandler.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kSixteenth, tapHandler.m_divValue);
  TEST_ASSERT_EQUAL(200, tapHandler.m_interval);
  TEST_ASSERT_EQUAL(50, tapHandler.m_divInterval);

  tapHandler.setNextDivValue();

  TEST_ASSERT_EQUAL(TapState::kEnabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, tapHandler.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kDottedEight, tapHandler.m_divValue);
  TEST_ASSERT_EQUAL(200, tapHandler.m_interval);
  TEST_ASSERT_EQUAL(150, tapHandler.m_divInterval);

  tapHandler.setNextDivValue();

  TEST_ASSERT_EQUAL(TapState::kEnabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, tapHandler.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEightTriplet, tapHandler.m_divValue);
  TEST_ASSERT_EQUAL(200, tapHandler.m_interval);
  TEST_ASSERT_EQUAL(66, tapHandler.m_divInterval);

  tapHandler.setNextDivValue();

  TEST_ASSERT_EQUAL(TapState::kEnabled, tapHandler.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, tapHandler.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, tapHandler.m_divValue);
  TEST_ASSERT_EQUAL(200, tapHandler.m_interval);
  TEST_ASSERT_EQUAL(0, tapHandler.m_divInterval);
}

// =============================================================================
// Div interval recalculation Tests
// =============================================================================

void test_div_interval_recalculation_on_tap() {
  TapHandler tapHandler;

  // Set up initial interval with div enabled
  tapHandler.registerTap(1000);
  tapHandler.registerTap(1200);
  TEST_ASSERT_EQUAL(200, tapHandler.m_interval);

  tapHandler.setNextDivValue();  // Enable kEight
  TEST_ASSERT_EQUAL(DivState::kEnabled, tapHandler.m_divState);
  TEST_ASSERT_EQUAL(100, tapHandler.m_divInterval);

  // New tap should recalculate divInterval
  tapHandler.registerTap(1500);  // (1500 - 1000) / 2 = 250ms interval
  TEST_ASSERT_EQUAL(250, tapHandler.m_interval);
  TEST_ASSERT_EQUAL(125, tapHandler.m_divInterval);  // 250 / 2 = 125
}

int main() {
  UNITY_BEGIN();

  // Initialization
  RUN_TEST(test_default_initialization);

  // Enum validation
  RUN_TEST(test_tap_enums_validation);

  // registerTap
  RUN_TEST(test_register_tap_first_tap_does_not_enable);
  RUN_TEST(test_register_tap);
  RUN_TEST(test_register_tap_timeout_resets_sequence);
  RUN_TEST(test_register_tap_multiple_taps_averages_interval);

  // setNextDivValue
  RUN_TEST(test_div);
  RUN_TEST(test_div_values);

  // Div interval recalculation
  RUN_TEST(test_div_interval_recalculation_on_tap);

  UNITY_END();
}
