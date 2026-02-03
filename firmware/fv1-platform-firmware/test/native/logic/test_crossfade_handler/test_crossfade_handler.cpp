#include <unity.h>
#include "core/event_bus.h"

#include "../src/logic/crossfade_handler.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

// =============================================================================
// Transition curve tests
// =============================================================================

void test_curve_transition_full_dry() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;

  // 1023 is 100% dry
  auto result = handler.calculate(1023);

  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(0, result.m_wet);
}

void test_curve_transition_full_wet() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;

  // 0 is 100% wet
  auto result = handler.calculate(0);

  TEST_ASSERT_EQUAL(0, result.m_dry);
  TEST_ASSERT_EQUAL(1023, result.m_wet);
}

void test_curve_transition_50_50_mix() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;

  // 512 is 50/50
  auto result = handler.calculate(512);

  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(1023, result.m_wet);
}

void test_curve_transition_25_percent_mix() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;

  // 256 is 25% so 1023 wet ~511 dry
  auto result = handler.calculate(256);

  TEST_ASSERT_EQUAL(1023, result.m_wet);
  TEST_ASSERT_INT_WITHIN(2, 511, result.m_dry);
}

void test_curve_transition_75_percent_mix() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;

  // 768 is 75% so 1023 dry ~511 wet
  auto result = handler.calculate(768);

  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_INT_WITHIN(2, 511, result.m_wet);
}

// =============================================================================
// Linear curve tests
// =============================================================================

void test_curve_linear_full_dry() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;

  // 1023 is 100% dry
  auto result = handler.calculate(1023);

  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(0, result.m_wet);
}

void test_curve_linear_full_wet() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;

  // 0 is 100% wet
  auto result = handler.calculate(0);

  TEST_ASSERT_EQUAL(0, result.m_dry);
  TEST_ASSERT_EQUAL(1023, result.m_wet);
}

void test_curve_linear_50_50_mix() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;

  // 512 is 50/50
  auto result = handler.calculate(512);

  TEST_ASSERT_EQUAL(512, result.m_dry);
  TEST_ASSERT_EQUAL(511, result.m_wet);
}

void test_curve_linear_25_percent_mix() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;

  // 256 is 25%
  auto result = handler.calculate(256);

  TEST_ASSERT_EQUAL(256, result.m_dry);
  TEST_ASSERT_EQUAL(767, result.m_wet);
}

void test_curve_linear_75_percent_mix() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;

  // 768 is 75%
  auto result = handler.calculate(768);

  TEST_ASSERT_EQUAL(768, result.m_dry);
  TEST_ASSERT_EQUAL(255, result.m_wet);
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // Transition curve tests
  RUN_TEST(test_curve_transition_full_dry);
  RUN_TEST(test_curve_transition_full_wet);
  RUN_TEST(test_curve_transition_50_50_mix);
  RUN_TEST(test_curve_transition_25_percent_mix);
  RUN_TEST(test_curve_transition_75_percent_mix);

  // Linear curve tests
  RUN_TEST(test_curve_linear_full_dry);
  RUN_TEST(test_curve_linear_full_wet);
  RUN_TEST(test_curve_linear_50_50_mix);
  RUN_TEST(test_curve_linear_25_percent_mix);
  RUN_TEST(test_curve_linear_75_percent_mix);

  UNITY_END();
}
