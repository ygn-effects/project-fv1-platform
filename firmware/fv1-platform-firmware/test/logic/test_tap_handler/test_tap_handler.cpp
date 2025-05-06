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
  TEST_ASSERT_EQUAL(200, tapHandler.m_divInterval);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_tap_enums_validation);
  RUN_TEST(test_register_tap);
  RUN_TEST(test_div);
  RUN_TEST(test_div_values);
  UNITY_END();
}
