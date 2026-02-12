#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/preset_handler.h"

#include "../src/logic/preset_handler.cpp"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_snapshot() {
  LogicalState logicalState;
  PresetHandler presetHandler;

  logicalState.m_currentProgram = 2;
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kQuarter;
  logicalState.m_interval = 512;
  logicalState.m_divInterval = 256;
  logicalState.m_tempo = 256;
  logicalState.m_exprParams[2].m_state = ExprState::kActive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot1;
  logicalState.m_exprParams[2].m_direction = Direction::kInverted;
  logicalState.m_exprParams[2].m_heelValue = 256;
  logicalState.m_exprParams[2].m_toeValue = 512;
  logicalState.m_potParams[2][3].m_state = PotState::kDisabled;
  logicalState.m_potParams[2][3].m_value = 512;
  logicalState.m_potParams[2][3].m_minValue = 256;
  logicalState.m_potParams[2][3].m_maxValue = 768;

  presetHandler.snapshotFromState(logicalState, 1);

  TEST_ASSERT_EQUAL(logicalState.m_currentProgram, logicalState.m_loadedPresetBank.m_presets[1].m_programIndex);
  TEST_ASSERT_EQUAL(logicalState.m_tapState, logicalState.m_loadedPresetBank.m_presets[1].m_tapState);
  TEST_ASSERT_EQUAL(logicalState.m_divState, logicalState.m_loadedPresetBank.m_presets[1].m_divState);
  TEST_ASSERT_EQUAL(logicalState.m_divValue, logicalState.m_loadedPresetBank.m_presets[1].m_divValue);
  TEST_ASSERT_EQUAL(logicalState.m_interval, logicalState.m_loadedPresetBank.m_presets[1].m_interval);
  TEST_ASSERT_EQUAL(logicalState.m_divInterval, logicalState.m_loadedPresetBank.m_presets[1].m_divInterval);
  TEST_ASSERT_EQUAL(logicalState.m_tempo, logicalState.m_loadedPresetBank.m_presets[1].m_tempo);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_state, logicalState.m_loadedPresetBank.m_presets[1].m_exprState);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_mappedPot, logicalState.m_loadedPresetBank.m_presets[1].m_mappedPot);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_direction, logicalState.m_loadedPresetBank.m_presets[1].m_direction);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_heelValue, logicalState.m_loadedPresetBank.m_presets[1].m_heelValue);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_toeValue, logicalState.m_loadedPresetBank.m_presets[1].m_toeValue);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_state, logicalState.m_loadedPresetBank.m_presets[1].m_potParams[3].m_state);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_value, logicalState.m_loadedPresetBank.m_presets[1].m_potParams[3].m_value);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_minValue, logicalState.m_loadedPresetBank.m_presets[1].m_potParams[3].m_minValue);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_maxValue, logicalState.m_loadedPresetBank.m_presets[1].m_potParams[3].m_maxValue);
}

void test_apply() {
  LogicalState logicalState;
  PresetHandler presetHandler;

  logicalState.m_loadedPresetBank.m_presets[1].m_programIndex = 2;
  logicalState.m_loadedPresetBank.m_presets[1].m_tapState = TapState::kEnabled;
  logicalState.m_loadedPresetBank.m_presets[1].m_divState = DivState::kEnabled;
  logicalState.m_loadedPresetBank.m_presets[1].m_divValue = DivValue::kEight;
  logicalState.m_loadedPresetBank.m_presets[1].m_interval = 512;
  logicalState.m_loadedPresetBank.m_presets[1].m_divInterval = 256;
  logicalState.m_loadedPresetBank.m_presets[1].m_tempo = 256;
  logicalState.m_loadedPresetBank.m_presets[1].m_exprState = ExprState::kActive;
  logicalState.m_loadedPresetBank.m_presets[1].m_mappedPot = MappedPot::kPot0;
  logicalState.m_loadedPresetBank.m_presets[1].m_direction = Direction::kInverted;
  logicalState.m_loadedPresetBank.m_presets[1].m_heelValue = 256;
  logicalState.m_loadedPresetBank.m_presets[1].m_toeValue = 512;
  logicalState.m_loadedPresetBank.m_presets[1].m_potParams[3].m_state = PotState::kActive;
  logicalState.m_loadedPresetBank.m_presets[1].m_potParams[3].m_value = 512;
  logicalState.m_loadedPresetBank.m_presets[1].m_potParams[3].m_minValue = 256;
  logicalState.m_loadedPresetBank.m_presets[1].m_potParams[3].m_maxValue = 768;

  presetHandler.applyToState(logicalState, 1);

  TEST_ASSERT_EQUAL(logicalState.m_currentProgram, logicalState.m_loadedPresetBank.m_presets[1].m_programIndex);
  TEST_ASSERT_EQUAL(logicalState.m_tapState, logicalState.m_loadedPresetBank.m_presets[1].m_tapState);
  TEST_ASSERT_EQUAL(logicalState.m_divState, logicalState.m_loadedPresetBank.m_presets[1].m_divState);
  TEST_ASSERT_EQUAL(logicalState.m_divValue, logicalState.m_loadedPresetBank.m_presets[1].m_divValue);
  TEST_ASSERT_EQUAL(logicalState.m_interval, logicalState.m_loadedPresetBank.m_presets[1].m_interval);
  TEST_ASSERT_EQUAL(logicalState.m_divInterval, logicalState.m_loadedPresetBank.m_presets[1].m_divInterval);
  TEST_ASSERT_EQUAL(logicalState.m_tempo, logicalState.m_loadedPresetBank.m_presets[1].m_tempo);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_state, logicalState.m_loadedPresetBank.m_presets[1].m_exprState);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_mappedPot, logicalState.m_loadedPresetBank.m_presets[1].m_mappedPot);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_direction, logicalState.m_loadedPresetBank.m_presets[1].m_direction);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_heelValue, logicalState.m_loadedPresetBank.m_presets[1].m_heelValue);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_toeValue, logicalState.m_loadedPresetBank.m_presets[1].m_toeValue);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_state, logicalState.m_loadedPresetBank.m_presets[1].m_potParams[3].m_state);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_value, logicalState.m_loadedPresetBank.m_presets[1].m_potParams[3].m_value);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_minValue, logicalState.m_loadedPresetBank.m_presets[1].m_potParams[3].m_minValue);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_maxValue, logicalState.m_loadedPresetBank.m_presets[1].m_potParams[3].m_maxValue);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_snapshot);
  RUN_TEST(test_apply);
  UNITY_END();
}
