#include <unity.h>
#include "core/event_bus.h"

#include "../src/logic/expr_handler.cpp"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_expr_state_validation() {
  TEST_ASSERT_TRUE(ExprStateValidator::isValid(0));
  TEST_ASSERT_TRUE(ExprStateValidator::isValid(1));

  TEST_ASSERT_TRUE(MappedPotValidator::isValid(0));
  TEST_ASSERT_TRUE(MappedPotValidator::isValid(1));
  TEST_ASSERT_TRUE(MappedPotValidator::isValid(2));
  TEST_ASSERT_TRUE(MappedPotValidator::isValid(3));

  TEST_ASSERT_TRUE(DirectionValidator::isValid(0));
  TEST_ASSERT_TRUE(DirectionValidator::isValid(1));

  TEST_ASSERT_FALSE(ExprStateValidator::isValid(2));
  TEST_ASSERT_FALSE(MappedPotValidator::isValid(4));
  TEST_ASSERT_FALSE(DirectionValidator::isValid(2));

  TEST_ASSERT_EQUAL(ExprState::kInactive, ExprStateValidator::sanitize(0, ExprState::kActive));
  TEST_ASSERT_EQUAL(ExprState::kActive, ExprStateValidator::sanitize(1, ExprState::kInactive));
  TEST_ASSERT_EQUAL(ExprState::kActive, ExprStateValidator::sanitize(4, ExprState::kActive));

  TEST_ASSERT_EQUAL(MappedPot::kPot0, MappedPotValidator::sanitize(0, MappedPot::kPot1));
  TEST_ASSERT_EQUAL(MappedPot::kPot1, MappedPotValidator::sanitize(1, MappedPot::kPot2));
  TEST_ASSERT_EQUAL(MappedPot::kMixPot, MappedPotValidator::sanitize(6, MappedPot::kMixPot));

  TEST_ASSERT_EQUAL(Direction::kNormal, DirectionValidator::sanitize(0, Direction::kInverted));
  TEST_ASSERT_EQUAL(Direction::kInverted, DirectionValidator::sanitize(1, Direction::kNormal));
  TEST_ASSERT_EQUAL(Direction::kNormal, DirectionValidator::sanitize(6, Direction::kNormal));
}

void test_clamping() {
  ExprHandler exprHandler;

  TEST_ASSERT_EQUAL(1023, exprHandler.mapAdcValue(2048));
}

void test_map() {
  ExprHandler exprHandler;

  TEST_ASSERT_EQUAL(512, exprHandler.mapAdcValue(512));
  TEST_ASSERT_EQUAL(0, exprHandler.mapAdcValue(0));
  TEST_ASSERT_EQUAL(1023, exprHandler.mapAdcValue(1023));
}

void test_map_inverted() {
  ExprHandler exprHandler;
  exprHandler.m_direction = Direction::kInverted;

  TEST_ASSERT_EQUAL(511, exprHandler.mapAdcValue(512));
  TEST_ASSERT_EQUAL(0, exprHandler.mapAdcValue(1023));
  TEST_ASSERT_EQUAL(1023, exprHandler.mapAdcValue(0));
}

void test_heel_toe() {
  ExprHandler exprHandler;
  exprHandler.m_heelValue = 200;
  exprHandler.m_toeValue = 800;

  TEST_ASSERT_EQUAL(500, exprHandler.mapAdcValue(512));
  TEST_ASSERT_EQUAL(200, exprHandler.mapAdcValue(0));
  TEST_ASSERT_EQUAL(800, exprHandler.mapAdcValue(1023));
}

void test_toggle_expr_state() {
  ExprHandler exprHandler;

  exprHandler.toggleExprState();
  TEST_ASSERT_EQUAL(ExprState::kActive, exprHandler.m_state);

  exprHandler.toggleExprState();
  TEST_ASSERT_EQUAL(ExprState::kInactive, exprHandler.m_state);
}

void test_change_mapped_pot() {
  ExprHandler exprHandler;

  exprHandler.changeMappedPot(-1);
  TEST_ASSERT_EQUAL(MappedPot::kMixPot, exprHandler.m_mappedPot);

  exprHandler.changeMappedPot(1);
  TEST_ASSERT_EQUAL(MappedPot::kPot0,exprHandler.m_mappedPot);

  exprHandler.changeMappedPot(1);
  TEST_ASSERT_EQUAL(MappedPot::kPot1,exprHandler.m_mappedPot);
}

void test_toggle_direction() {
  ExprHandler exprHandler;

  exprHandler.toggleDirection();
  TEST_ASSERT_EQUAL(Direction::kInverted, exprHandler.m_direction);

  exprHandler.toggleDirection();
  TEST_ASSERT_EQUAL(Direction::kNormal, exprHandler.m_direction);
}

void test_change_heel_value() {
  ExprHandler exprHandler;

  exprHandler.changeHeelValue(1);
  TEST_ASSERT_EQUAL(1, exprHandler.m_heelValue);

  exprHandler.changeHeelValue(-1);
  TEST_ASSERT_EQUAL(0, exprHandler.m_heelValue);

  exprHandler.changeHeelValue(-1);
  TEST_ASSERT_EQUAL(0, exprHandler.m_heelValue);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_expr_state_validation);
  RUN_TEST(test_clamping);
  RUN_TEST(test_map);
  RUN_TEST(test_map_inverted);
  RUN_TEST(test_heel_toe);
  RUN_TEST(test_toggle_expr_state);
  RUN_TEST(test_change_mapped_pot);
  RUN_TEST(test_toggle_direction);
  RUN_TEST(test_change_heel_value);
  UNITY_END();
}
