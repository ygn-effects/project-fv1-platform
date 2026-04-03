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

// =============================================================================
// Pickup Mode - Initialization
// =============================================================================

void test_pickup_default_all_pots_picked_up() {
  PotHandler potHandler;

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    TEST_ASSERT_TRUE(potHandler.m_pickedUp[i]);
  }
}

// =============================================================================
// Pickup Mode - resetPickup
// =============================================================================

void test_resetPickup_marks_all_pots_not_picked_up() {
  PotHandler potHandler;

  potHandler.resetPickup();

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    TEST_ASSERT_FALSE(potHandler.m_pickedUp[i]);
  }
}

// =============================================================================
// Pickup Mode - checkPickup
// =============================================================================

void test_checkPickup_returns_true_when_already_picked_up() {
  PotHandler potHandler;
  // Default state: all picked up

  TEST_ASSERT_TRUE(potHandler.checkPickup(500, 300, 0));
}

void test_checkPickup_first_reading_stores_reference_returns_false() {
  PotHandler potHandler;
  potHandler.resetPickup();

  // First reading after reset: stores reference, returns false
  TEST_ASSERT_FALSE(potHandler.checkPickup(200, 500, 0));
  TEST_ASSERT_FALSE(potHandler.m_pickedUp[0]);
}

void test_checkPickup_no_crossover_returns_false() {
  PotHandler potHandler;
  potHandler.resetPickup();

  // Stored value is 500, physical pot is below at 200
  potHandler.checkPickup(200, 500, 0);  // First reading (reference)

  // Moving further below stored value - no crossover
  TEST_ASSERT_FALSE(potHandler.checkPickup(100, 500, 0));
  TEST_ASSERT_FALSE(potHandler.checkPickup(150, 500, 0));
  TEST_ASSERT_FALSE(potHandler.checkPickup(300, 500, 0));
  TEST_ASSERT_FALSE(potHandler.m_pickedUp[0]);
}

void test_checkPickup_crossover_from_below_picks_up() {
  PotHandler potHandler;
  potHandler.resetPickup();

  // Stored value is 500, physical pot starts below
  potHandler.checkPickup(200, 500, 0);  // Reference: below stored
  potHandler.checkPickup(400, 500, 0);  // Still below

  // Crosses over stored value
  TEST_ASSERT_TRUE(potHandler.checkPickup(550, 500, 0));
  TEST_ASSERT_TRUE(potHandler.m_pickedUp[0]);
}

void test_checkPickup_crossover_from_above_picks_up() {
  PotHandler potHandler;
  potHandler.resetPickup();

  // Stored value is 300, physical pot starts above
  potHandler.checkPickup(700, 300, 0);  // Reference: above stored
  potHandler.checkPickup(500, 300, 0);  // Still above

  // Crosses over stored value
  TEST_ASSERT_TRUE(potHandler.checkPickup(250, 300, 0));
  TEST_ASSERT_TRUE(potHandler.m_pickedUp[0]);
}

void test_checkPickup_exact_match_picks_up() {
  PotHandler potHandler;
  potHandler.resetPickup();

  // Stored value is 500, physical pot starts below
  potHandler.checkPickup(200, 500, 0);  // Reference: below stored

  // Lands exactly on stored value
  TEST_ASSERT_TRUE(potHandler.checkPickup(500, 500, 0));
  TEST_ASSERT_TRUE(potHandler.m_pickedUp[0]);
}

void test_checkPickup_independent_per_pot() {
  PotHandler potHandler;
  potHandler.resetPickup();

  // Pot 0: set reference below stored
  potHandler.checkPickup(200, 500, 0);
  // Pot 1: set reference above stored
  potHandler.checkPickup(800, 400, 1);

  // Pot 0 crosses over - picks up
  TEST_ASSERT_TRUE(potHandler.checkPickup(600, 500, 0));
  TEST_ASSERT_TRUE(potHandler.m_pickedUp[0]);

  // Pot 1 still hasn't crossed
  TEST_ASSERT_FALSE(potHandler.checkPickup(600, 400, 1));
  TEST_ASSERT_FALSE(potHandler.m_pickedUp[1]);

  // Pot 1 crosses over
  TEST_ASSERT_TRUE(potHandler.checkPickup(350, 400, 1));
  TEST_ASSERT_TRUE(potHandler.m_pickedUp[1]);
}

void test_checkPickup_stays_picked_up_after_crossover() {
  PotHandler potHandler;
  potHandler.resetPickup();

  potHandler.checkPickup(200, 500, 0);  // Reference
  potHandler.checkPickup(600, 500, 0);  // Crossover - picked up

  // Subsequent calls always return true
  TEST_ASSERT_TRUE(potHandler.checkPickup(700, 500, 0));
  TEST_ASSERT_TRUE(potHandler.checkPickup(100, 500, 0));
  TEST_ASSERT_TRUE(potHandler.checkPickup(500, 500, 0));
}

void test_resetPickup_after_pickup_resets_again() {
  PotHandler potHandler;
  potHandler.resetPickup();

  // Pick up pot 0
  potHandler.checkPickup(200, 500, 0);
  potHandler.checkPickup(600, 500, 0);
  TEST_ASSERT_TRUE(potHandler.m_pickedUp[0]);

  // Reset again (simulates another program change)
  potHandler.resetPickup();
  TEST_ASSERT_FALSE(potHandler.m_pickedUp[0]);

  // Needs crossover again
  potHandler.checkPickup(800, 500, 0);  // Reference: above
  TEST_ASSERT_FALSE(potHandler.checkPickup(600, 500, 0));  // Still above
  TEST_ASSERT_TRUE(potHandler.checkPickup(400, 500, 0));   // Crossed over
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

  // Pickup mode - initialization
  RUN_TEST(test_pickup_default_all_pots_picked_up);

  // Pickup mode - resetPickup
  RUN_TEST(test_resetPickup_marks_all_pots_not_picked_up);

  // Pickup mode - checkPickup
  RUN_TEST(test_checkPickup_returns_true_when_already_picked_up);
  RUN_TEST(test_checkPickup_first_reading_stores_reference_returns_false);
  RUN_TEST(test_checkPickup_no_crossover_returns_false);
  RUN_TEST(test_checkPickup_crossover_from_below_picks_up);
  RUN_TEST(test_checkPickup_crossover_from_above_picks_up);
  RUN_TEST(test_checkPickup_exact_match_picks_up);
  RUN_TEST(test_checkPickup_independent_per_pot);
  RUN_TEST(test_checkPickup_stays_picked_up_after_crossover);
  RUN_TEST(test_resetPickup_after_pickup_resets_again);

  UNITY_END();
}
