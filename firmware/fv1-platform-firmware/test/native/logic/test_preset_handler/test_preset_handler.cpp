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

  presetHandler.snapshotFromState(logicalState);

  TEST_ASSERT_EQUAL(logicalState.m_currentProgram, presetHandler.m_snapshot.m_programIndex);
  TEST_ASSERT_EQUAL(logicalState.m_tapState, presetHandler.m_snapshot.m_tapState);
  TEST_ASSERT_EQUAL(logicalState.m_divState, presetHandler.m_snapshot.m_divState);
  TEST_ASSERT_EQUAL(logicalState.m_divValue, presetHandler.m_snapshot.m_divValue);
  TEST_ASSERT_EQUAL(logicalState.m_interval, presetHandler.m_snapshot.m_interval);
  TEST_ASSERT_EQUAL(logicalState.m_divInterval, presetHandler.m_snapshot.m_divInterval);
  TEST_ASSERT_EQUAL(logicalState.m_tempo, presetHandler.m_snapshot.m_tempo);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_state, presetHandler.m_snapshot.m_exprState);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_mappedPot, presetHandler.m_snapshot.m_mappedPot);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_direction, presetHandler.m_snapshot.m_direction);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_heelValue, presetHandler.m_snapshot.m_heelValue);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_toeValue, presetHandler.m_snapshot.m_toeValue);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_state, presetHandler.m_snapshot.m_potParams[3].m_state);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_value, presetHandler.m_snapshot.m_potParams[3].m_value);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_minValue, presetHandler.m_snapshot.m_potParams[3].m_minValue);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_maxValue, presetHandler.m_snapshot.m_potParams[3].m_maxValue);
}

void test_apply() {
  LogicalState logicalState;
  PresetHandler presetHandler;

  PresetBank bank;
  presetHandler.m_currentPresetBank = &bank;

  presetHandler.m_currentPresetBank->m_presets[1].m_programIndex = 2;
  presetHandler.m_currentPresetBank->m_presets[1].m_tapState = TapState::kEnabled;
  presetHandler.m_currentPresetBank->m_presets[1].m_divState = DivState::kEnabled;
  presetHandler.m_currentPresetBank->m_presets[1].m_divValue = DivValue::kEight;
  presetHandler.m_currentPresetBank->m_presets[1].m_interval = 512;
  presetHandler.m_currentPresetBank->m_presets[1].m_divInterval = 256;
  presetHandler.m_currentPresetBank->m_presets[1].m_tempo = 256;
  presetHandler.m_currentPresetBank->m_presets[1].m_exprState = ExprState::kActive;
  presetHandler.m_currentPresetBank->m_presets[1].m_mappedPot = MappedPot::kPot0;
  presetHandler.m_currentPresetBank->m_presets[1].m_direction = Direction::kInverted;
  presetHandler.m_currentPresetBank->m_presets[1].m_heelValue = 256;
  presetHandler.m_currentPresetBank->m_presets[1].m_toeValue = 512;
  presetHandler.m_currentPresetBank->m_presets[1].m_potParams[3].m_state = PotState::kActive;
  presetHandler.m_currentPresetBank->m_presets[1].m_potParams[3].m_value = 512;
  presetHandler.m_currentPresetBank->m_presets[1].m_potParams[3].m_minValue = 256;
  presetHandler.m_currentPresetBank->m_presets[1].m_potParams[3].m_maxValue = 768;

  presetHandler.applyToState(logicalState, 1);

  TEST_ASSERT_EQUAL(logicalState.m_currentProgram, presetHandler.m_currentPresetBank->m_presets[1].m_programIndex);
  TEST_ASSERT_EQUAL(logicalState.m_tapState, presetHandler.m_currentPresetBank->m_presets[1].m_tapState);
  TEST_ASSERT_EQUAL(logicalState.m_divState, presetHandler.m_currentPresetBank->m_presets[1].m_divState);
  TEST_ASSERT_EQUAL(logicalState.m_divValue, presetHandler.m_currentPresetBank->m_presets[1].m_divValue);
  TEST_ASSERT_EQUAL(logicalState.m_interval, presetHandler.m_currentPresetBank->m_presets[1].m_interval);
  TEST_ASSERT_EQUAL(logicalState.m_divInterval, presetHandler.m_currentPresetBank->m_presets[1].m_divInterval);
  TEST_ASSERT_EQUAL(logicalState.m_tempo, presetHandler.m_currentPresetBank->m_presets[1].m_tempo);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_state, presetHandler.m_currentPresetBank->m_presets[1].m_exprState);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_mappedPot, presetHandler.m_currentPresetBank->m_presets[1].m_mappedPot);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_direction, presetHandler.m_currentPresetBank->m_presets[1].m_direction);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_heelValue, presetHandler.m_currentPresetBank->m_presets[1].m_heelValue);
  TEST_ASSERT_EQUAL(logicalState.m_exprParams[2].m_toeValue, presetHandler.m_currentPresetBank->m_presets[1].m_toeValue);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_state, presetHandler.m_currentPresetBank->m_presets[1].m_potParams[3].m_state);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_value, presetHandler.m_currentPresetBank->m_presets[1].m_potParams[3].m_value);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_minValue, presetHandler.m_currentPresetBank->m_presets[1].m_potParams[3].m_minValue);
  TEST_ASSERT_EQUAL(logicalState.m_potParams[2][3].m_maxValue, presetHandler.m_currentPresetBank->m_presets[1].m_potParams[3].m_maxValue);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_snapshot);
  RUN_TEST(test_apply);
  UNITY_END();
}
