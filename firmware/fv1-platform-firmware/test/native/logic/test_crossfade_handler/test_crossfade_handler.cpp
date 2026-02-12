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

  // 0 is 100% dry
  auto result = handler.calculate(0);

  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(0, result.m_wet);
}

void test_curve_transition_full_wet() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;

  // 1023 is 100% wet
  auto result = handler.calculate(1023);

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

  // 256 is 25%
  auto result = handler.calculate(256);

  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(511, result.m_wet);
}

void test_curve_transition_75_percent_mix() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;

  // 768 is 75%
  auto result = handler.calculate(768);

  TEST_ASSERT_EQUAL(1023, result.m_wet);
  TEST_ASSERT_EQUAL(511, result.m_dry);
}

// =============================================================================
// Linear curve tests
// =============================================================================

void test_curve_linear_full_dry() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;

  // 1023 is 100% dry
  auto result = handler.calculate(0);

  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(0, result.m_wet);
}

void test_curve_linear_full_wet() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;

  // 0 is 100% wet
  auto result = handler.calculate(1023);

  TEST_ASSERT_EQUAL(0, result.m_dry);
  TEST_ASSERT_EQUAL(1023, result.m_wet);
}

void test_curve_linear_50_50_mix() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;

  // 512 is 50/50
  auto result = handler.calculate(512);

  TEST_ASSERT_EQUAL(511, result.m_dry);
  TEST_ASSERT_EQUAL(512, result.m_wet);
}

void test_curve_linear_25_percent_mix() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;

  // 256 is 25%
  auto result = handler.calculate(256);

  TEST_ASSERT_EQUAL(767, result.m_dry);
  TEST_ASSERT_EQUAL(256, result.m_wet);
}

void test_curve_linear_75_percent_mix() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;

  // 768 is 75%
  auto result = handler.calculate(768);

  TEST_ASSERT_EQUAL(255, result.m_dry);
  TEST_ASSERT_EQUAL(768, result.m_wet);
}

// =============================================================================
// Constant Power curve tests
// =============================================================================

void test_curve_constant_power_full_dry() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kConstantPower;

  // 0 is 100% dry
  auto result = handler.calculate(0);

  // sineLut[0] = 0, sineLut[255] = 1023
  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(0, result.m_wet);
}

void test_curve_constant_power_full_wet() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kConstantPower;

  // 1023 is 100% wet
  auto result = handler.calculate(1023);

  // index = 1023 >> 2 = 255
  // sineLut[255] = 1023, sineLut[0] = 0
  TEST_ASSERT_EQUAL(0, result.m_dry);
  TEST_ASSERT_EQUAL(1023, result.m_wet);
}

void test_curve_constant_power_50_50_mix() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kConstantPower;

  // 512 is 50/50
  auto result = handler.calculate(512);

  // index = 512 >> 2 = 128
  // sineLut[128] = 724, sineLut[127] = 719
  TEST_ASSERT_EQUAL(719, result.m_dry);
  TEST_ASSERT_EQUAL(724, result.m_wet);
}

// =============================================================================
// Logarithmic curve tests
// =============================================================================

void test_curve_logarithmic_full_dry() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLogarithmic;

  // 0 is 100% dry
  auto result = handler.calculate(0);

  // square = 0, wet = 0, dry = 1023
  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(0, result.m_wet);
}

void test_curve_logarithmic_full_wet() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLogarithmic;

  // 1023 is 100% wet
  auto result = handler.calculate(1023);

  // square = 1023 * 1023 = 1046529
  // wet = 1046529 >> 10 = 1022
  // dry = 1023 - 1022 = 1
  TEST_ASSERT_EQUAL(1, result.m_dry);
  TEST_ASSERT_EQUAL(1022, result.m_wet);
}

void test_curve_logarithmic_50_50_mix() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLogarithmic;

  // 512 is 50%
  auto result = handler.calculate(512);

  // square = 512 * 512 = 262144
  // wet = 262144 >> 10 = 256
  // dry = 1023 - 256 = 767
  TEST_ASSERT_EQUAL(767, result.m_dry);
  TEST_ASSERT_EQUAL(256, result.m_wet);
}

// =============================================================================
// Edge case tests
// =============================================================================

void test_input_clamping_above_max() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;

  // Input above 1023 should be clamped to 1023
  auto result = handler.calculate(2000);

  // Should behave same as calculate(1023)
  TEST_ASSERT_EQUAL(0, result.m_dry);
  TEST_ASSERT_EQUAL(1023, result.m_wet);
}

void test_transition_boundary_at_512() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;

  // 512 is the boundary - should be in left side (<=512)
  auto result = handler.calculate(512);

  // Left side: dry = 1023, wet = mapped value
  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(1023, result.m_wet);
}

void test_transition_boundary_at_513() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;

  // 513 is just past the boundary - should be in right side (>512)
  auto result = handler.calculate(513);

  // Right side: wet = 1023, dry = mapped from 512..1023 to 1023..0
  // (513 - 512) / (1023 - 512) * (0 - 1023) + 1023 = 1021
  TEST_ASSERT_EQUAL(1023, result.m_wet);
  TEST_ASSERT_EQUAL(1021, result.m_dry);
}

void test_default_curve_is_transition() {
  CrossfadeHandler handler;
  // Don't set m_currentCurve - should default to kTransition

  auto result = handler.calculate(0);

  // kTransition at 0: dry = 1023, wet = 0
  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(0, result.m_wet);
}

// =============================================================================
// Input range mapping tests
// =============================================================================

void test_default_input_range() {
  CrossfadeHandler handler;

  // Default range should be 0-1023
  TEST_ASSERT_EQUAL(0, handler.m_minInputValue);
  TEST_ASSERT_EQUAL(1023, handler.m_maxInputValue);
}

void test_linear_zoomed_range_at_min() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;
  handler.m_minInputValue = 256;
  handler.m_maxInputValue = 768;

  // Input 0 maps to effectiveValue 256
  auto result = handler.calculate(0);

  TEST_ASSERT_EQUAL(767, result.m_dry);
  TEST_ASSERT_EQUAL(256, result.m_wet);
}

void test_linear_zoomed_range_at_mid() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;
  handler.m_minInputValue = 256;
  handler.m_maxInputValue = 768;

  // Input 512 maps to effectiveValue 512
  auto result = handler.calculate(512);

  TEST_ASSERT_EQUAL(511, result.m_dry);
  TEST_ASSERT_EQUAL(512, result.m_wet);
}

void test_linear_zoomed_range_at_max() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;
  handler.m_minInputValue = 256;
  handler.m_maxInputValue = 768;

  // Input 1023 maps to effectiveValue 768
  auto result = handler.calculate(1023);

  TEST_ASSERT_EQUAL(255, result.m_dry);
  TEST_ASSERT_EQUAL(768, result.m_wet);
}

void test_transition_wet_only_range_at_min() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;
  handler.m_minInputValue = 512;
  handler.m_maxInputValue = 1023;

  // Input 0 maps to effectiveValue 512 (boundary, left side)
  auto result = handler.calculate(0);

  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(1023, result.m_wet);
}

void test_transition_wet_only_range_at_max() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;
  handler.m_minInputValue = 512;
  handler.m_maxInputValue = 1023;

  // Input 1023 maps to effectiveValue 1023 (full wet)
  auto result = handler.calculate(1023);

  TEST_ASSERT_EQUAL(0, result.m_dry);
  TEST_ASSERT_EQUAL(1023, result.m_wet);
}

void test_transition_dry_only_range_at_min() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;
  handler.m_minInputValue = 0;
  handler.m_maxInputValue = 512;

  // Input 0 maps to effectiveValue 0 (full dry)
  auto result = handler.calculate(0);

  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(0, result.m_wet);
}

void test_transition_dry_only_range_at_max() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kTransition;
  handler.m_minInputValue = 0;
  handler.m_maxInputValue = 512;

  // Input 1023 maps to effectiveValue 512 (boundary, left side)
  auto result = handler.calculate(1023);

  TEST_ASSERT_EQUAL(1023, result.m_dry);
  TEST_ASSERT_EQUAL(1023, result.m_wet);
}

void test_narrow_range_linear() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLinear;
  handler.m_minInputValue = 500;
  handler.m_maxInputValue = 524;

  // Input 0 maps to effectiveValue 500
  auto result_min = handler.calculate(0);
  TEST_ASSERT_EQUAL(523, result_min.m_dry);
  TEST_ASSERT_EQUAL(500, result_min.m_wet);

  // Input 1023 maps to effectiveValue 524
  auto result_max = handler.calculate(1023);
  TEST_ASSERT_EQUAL(499, result_max.m_dry);
  TEST_ASSERT_EQUAL(524, result_max.m_wet);
}

void test_constant_power_with_custom_range() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kConstantPower;
  handler.m_minInputValue = 0;
  handler.m_maxInputValue = 512;

  // Input 1023 maps to effectiveValue 512
  // index = 512 >> 2 = 128
  // sineLut[128] = 724, sineLut[127] = 719
  auto result = handler.calculate(1023);

  TEST_ASSERT_EQUAL(719, result.m_dry);
  TEST_ASSERT_EQUAL(724, result.m_wet);
}

void test_logarithmic_with_custom_range() {
  CrossfadeHandler handler;
  handler.m_currentCurve = MixCurve::kLogarithmic;
  handler.m_minInputValue = 512;
  handler.m_maxInputValue = 1023;

  // Input 0 maps to effectiveValue 512
  // square = 512 * 512 = 262144
  // wet = 262144 >> 10 = 256
  // dry = 1023 - 256 = 767
  auto result = handler.calculate(0);

  TEST_ASSERT_EQUAL(767, result.m_dry);
  TEST_ASSERT_EQUAL(256, result.m_wet);
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

  // Constant Power curve tests
  RUN_TEST(test_curve_constant_power_full_dry);
  RUN_TEST(test_curve_constant_power_full_wet);
  RUN_TEST(test_curve_constant_power_50_50_mix);

  // Logarithmic curve tests
  RUN_TEST(test_curve_logarithmic_full_dry);
  RUN_TEST(test_curve_logarithmic_full_wet);
  RUN_TEST(test_curve_logarithmic_50_50_mix);

  // Edge case tests
  RUN_TEST(test_input_clamping_above_max);
  RUN_TEST(test_transition_boundary_at_512);
  RUN_TEST(test_transition_boundary_at_513);
  RUN_TEST(test_default_curve_is_transition);

  // Input range mapping tests
  RUN_TEST(test_default_input_range);
  RUN_TEST(test_linear_zoomed_range_at_min);
  RUN_TEST(test_linear_zoomed_range_at_mid);
  RUN_TEST(test_linear_zoomed_range_at_max);
  RUN_TEST(test_transition_wet_only_range_at_min);
  RUN_TEST(test_transition_wet_only_range_at_max);
  RUN_TEST(test_transition_dry_only_range_at_min);
  RUN_TEST(test_transition_dry_only_range_at_max);
  RUN_TEST(test_narrow_range_linear);
  RUN_TEST(test_constant_power_with_custom_range);
  RUN_TEST(test_logarithmic_with_custom_range);

  UNITY_END();
}
