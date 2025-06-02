#include <unity.h>
#include "core/event_bus.h"
#include "drivers/eeprom_m95.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"
#include "logic/preset_handler.h"
#include "services/memory_service.h"

#include "../src/drivers/eeprom_m95.cpp"
#include "../src/logic/memory_handler.cpp"
#include "../src/logic/preset_handler.cpp"
#include "../src/services/memory_service.cpp"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_logical_state() {
  LogicalState logicalState;
  hal::DigitalGpioDriver eepromCsPin(0, hal::GpioConfig::kOutput);
  hal::M95Driver eeprom(eepromCsPin);

  MemoryService memoryService(logicalState, eeprom);
  memoryService.init();

  logicalState.m_bypassState = BypassState::kActive;
  logicalState.m_currentProgram = 2;
  logicalState.m_currentPreset = 0;
  logicalState.m_currentPresetBank = 0;
  logicalState.m_midiChannel = 0;
  logicalState.m_tapState = TapState::kDisabled;
  logicalState.m_divState = DivState::kDisabled;
  logicalState.m_divValue = DivValue::kQuarter;
  logicalState.m_interval = 0;
  logicalState.m_divInterval = 0;
  logicalState.m_tempo = 256;
  logicalState.m_exprParams[0].m_state = ExprState::kInactive;
  logicalState.m_exprParams[0].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[0].m_direction = Direction::kNormal;
  logicalState.m_exprParams[0].m_heelValue = 0;
  logicalState.m_exprParams[0].m_toeValue = 1023;
  logicalState.m_exprParams[1].m_state = ExprState::kInactive;
  logicalState.m_exprParams[1].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[1].m_direction = Direction::kNormal;
  logicalState.m_exprParams[1].m_heelValue = 0;
  logicalState.m_exprParams[1].m_toeValue = 1023;
  logicalState.m_exprParams[2].m_state = ExprState::kInactive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[2].m_direction = Direction::kNormal;
  logicalState.m_exprParams[2].m_heelValue = 0;
  logicalState.m_exprParams[2].m_toeValue = 1023;
  logicalState.m_exprParams[3].m_state = ExprState::kInactive;
  logicalState.m_exprParams[3].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[3].m_direction = Direction::kNormal;
  logicalState.m_exprParams[3].m_heelValue = 0;
  logicalState.m_exprParams[3].m_toeValue = 1023;
  logicalState.m_exprParams[4].m_state = ExprState::kInactive;
  logicalState.m_exprParams[4].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[4].m_direction = Direction::kNormal;
  logicalState.m_exprParams[4].m_heelValue = 0;
  logicalState.m_exprParams[4].m_toeValue = 1023;
  logicalState.m_exprParams[5].m_state = ExprState::kInactive;
  logicalState.m_exprParams[5].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[5].m_direction = Direction::kNormal;
  logicalState.m_exprParams[5].m_heelValue = 0;
  logicalState.m_exprParams[5].m_toeValue = 1023;
  logicalState.m_exprParams[6].m_state = ExprState::kInactive;
  logicalState.m_exprParams[6].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[6].m_direction = Direction::kNormal;
  logicalState.m_exprParams[6].m_heelValue = 0;
  logicalState.m_exprParams[6].m_toeValue = 1023;
  logicalState.m_exprParams[7].m_state = ExprState::kInactive;
  logicalState.m_exprParams[7].m_mappedPot = MappedPot::kPot0;
  logicalState.m_exprParams[7].m_direction = Direction::kNormal;
  logicalState.m_exprParams[7].m_heelValue = 0;
  logicalState.m_exprParams[7].m_toeValue = 1023;
  logicalState.m_potParams[0][0].m_state = PotState::kActive;
  logicalState.m_potParams[0][0].m_value = 0;
  logicalState.m_potParams[0][0].m_minValue = 0;
  logicalState.m_potParams[0][0].m_maxValue = 1023;
  logicalState.m_potParams[0][1].m_state = PotState::kActive;
  logicalState.m_potParams[0][1].m_value = 0;
  logicalState.m_potParams[0][1].m_minValue = 0;
  logicalState.m_potParams[0][1].m_maxValue = 1023;
  logicalState.m_potParams[0][2].m_state = PotState::kActive;
  logicalState.m_potParams[0][2].m_value = 0;
  logicalState.m_potParams[0][2].m_minValue = 0;
  logicalState.m_potParams[0][2].m_maxValue = 1023;
  logicalState.m_potParams[0][3].m_state = PotState::kActive;
  logicalState.m_potParams[0][3].m_value = 0;
  logicalState.m_potParams[0][3].m_minValue = 0;
  logicalState.m_potParams[0][3].m_maxValue = 1023;
  logicalState.m_potParams[1][0].m_state = PotState::kActive;
  logicalState.m_potParams[1][0].m_value = 0;
  logicalState.m_potParams[1][0].m_minValue = 0;
  logicalState.m_potParams[1][0].m_maxValue = 1023;
  logicalState.m_potParams[1][1].m_state = PotState::kActive;
  logicalState.m_potParams[1][1].m_value = 0;
  logicalState.m_potParams[1][1].m_minValue = 0;
  logicalState.m_potParams[1][1].m_maxValue = 1023;
  logicalState.m_potParams[1][2].m_state = PotState::kActive;
  logicalState.m_potParams[1][2].m_value = 0;
  logicalState.m_potParams[1][2].m_minValue = 0;
  logicalState.m_potParams[1][2].m_maxValue = 1023;
  logicalState.m_potParams[1][3].m_state = PotState::kActive;
  logicalState.m_potParams[1][3].m_value = 0;
  logicalState.m_potParams[1][3].m_minValue = 0;
  logicalState.m_potParams[1][3].m_maxValue = 1023;
  logicalState.m_potParams[2][0].m_state = PotState::kActive;
  logicalState.m_potParams[2][0].m_value = 0;
  logicalState.m_potParams[2][0].m_minValue = 0;
  logicalState.m_potParams[2][0].m_maxValue = 1023;
  logicalState.m_potParams[2][1].m_state = PotState::kActive;
  logicalState.m_potParams[2][1].m_value = 0;
  logicalState.m_potParams[2][1].m_minValue = 0;
  logicalState.m_potParams[2][1].m_maxValue = 1023;
  logicalState.m_potParams[2][2].m_state = PotState::kActive;
  logicalState.m_potParams[2][2].m_value = 0;
  logicalState.m_potParams[2][2].m_minValue = 0;
  logicalState.m_potParams[2][2].m_maxValue = 1023;
  logicalState.m_potParams[2][3].m_state = PotState::kActive;
  logicalState.m_potParams[2][3].m_value = 0;
  logicalState.m_potParams[2][3].m_minValue = 0;
  logicalState.m_potParams[2][3].m_maxValue = 1023;
  logicalState.m_potParams[3][0].m_state = PotState::kActive;
  logicalState.m_potParams[3][0].m_value = 0;
  logicalState.m_potParams[3][0].m_minValue = 0;
  logicalState.m_potParams[3][0].m_maxValue = 1023;
  logicalState.m_potParams[3][1].m_state = PotState::kActive;
  logicalState.m_potParams[3][1].m_value = 0;
  logicalState.m_potParams[3][1].m_minValue = 0;
  logicalState.m_potParams[3][1].m_maxValue = 1023;
  logicalState.m_potParams[3][2].m_state = PotState::kActive;
  logicalState.m_potParams[3][2].m_value = 0;
  logicalState.m_potParams[3][2].m_minValue = 0;
  logicalState.m_potParams[3][2].m_maxValue = 1023;
  logicalState.m_potParams[3][3].m_state = PotState::kActive;
  logicalState.m_potParams[3][3].m_value = 0;
  logicalState.m_potParams[3][3].m_minValue = 0;
  logicalState.m_potParams[3][3].m_maxValue = 1023;
  logicalState.m_potParams[4][0].m_state = PotState::kActive;
  logicalState.m_potParams[4][0].m_value = 0;
  logicalState.m_potParams[4][0].m_minValue = 0;
  logicalState.m_potParams[4][0].m_maxValue = 1023;
  logicalState.m_potParams[4][1].m_state = PotState::kActive;
  logicalState.m_potParams[4][1].m_value = 0;
  logicalState.m_potParams[4][1].m_minValue = 0;
  logicalState.m_potParams[4][1].m_maxValue = 1023;
  logicalState.m_potParams[4][2].m_state = PotState::kActive;
  logicalState.m_potParams[4][2].m_value = 0;
  logicalState.m_potParams[4][2].m_minValue = 0;
  logicalState.m_potParams[4][2].m_maxValue = 1023;
  logicalState.m_potParams[4][3].m_state = PotState::kActive;
  logicalState.m_potParams[4][3].m_value = 0;
  logicalState.m_potParams[4][3].m_minValue = 0;
  logicalState.m_potParams[4][3].m_maxValue = 1023;
  logicalState.m_potParams[5][0].m_state = PotState::kActive;
  logicalState.m_potParams[5][0].m_value = 0;
  logicalState.m_potParams[5][0].m_minValue = 0;
  logicalState.m_potParams[5][0].m_maxValue = 1023;
  logicalState.m_potParams[5][1].m_state = PotState::kActive;
  logicalState.m_potParams[5][1].m_value = 0;
  logicalState.m_potParams[5][1].m_minValue = 0;
  logicalState.m_potParams[5][1].m_maxValue = 1023;
  logicalState.m_potParams[5][2].m_state = PotState::kActive;
  logicalState.m_potParams[5][2].m_value = 0;
  logicalState.m_potParams[5][2].m_minValue = 0;
  logicalState.m_potParams[5][2].m_maxValue = 1023;
  logicalState.m_potParams[5][3].m_state = PotState::kActive;
  logicalState.m_potParams[5][3].m_value = 0;
  logicalState.m_potParams[5][3].m_minValue = 0;
  logicalState.m_potParams[5][3].m_maxValue = 1023;
  logicalState.m_potParams[6][0].m_state = PotState::kActive;
  logicalState.m_potParams[6][0].m_value = 0;
  logicalState.m_potParams[6][0].m_minValue = 0;
  logicalState.m_potParams[6][0].m_maxValue = 1023;
  logicalState.m_potParams[6][1].m_state = PotState::kActive;
  logicalState.m_potParams[6][1].m_value = 0;
  logicalState.m_potParams[6][1].m_minValue = 0;
  logicalState.m_potParams[6][1].m_maxValue = 1023;
  logicalState.m_potParams[6][2].m_state = PotState::kActive;
  logicalState.m_potParams[6][2].m_value = 0;
  logicalState.m_potParams[6][2].m_minValue = 0;
  logicalState.m_potParams[6][2].m_maxValue = 1023;
  logicalState.m_potParams[6][3].m_state = PotState::kActive;
  logicalState.m_potParams[6][3].m_value = 0;
  logicalState.m_potParams[6][3].m_minValue = 0;
  logicalState.m_potParams[6][3].m_maxValue = 1023;
  logicalState.m_potParams[7][0].m_state = PotState::kActive;
  logicalState.m_potParams[7][0].m_value = 0;
  logicalState.m_potParams[7][0].m_minValue = 0;
  logicalState.m_potParams[7][0].m_maxValue = 1023;
  logicalState.m_potParams[7][1].m_state = PotState::kActive;
  logicalState.m_potParams[7][1].m_value = 0;
  logicalState.m_potParams[7][1].m_minValue = 0;
  logicalState.m_potParams[7][1].m_maxValue = 1023;
  logicalState.m_potParams[7][2].m_state = PotState::kActive;
  logicalState.m_potParams[7][2].m_value = 0;
  logicalState.m_potParams[7][2].m_minValue = 0;
  logicalState.m_potParams[7][2].m_maxValue = 1023;
  logicalState.m_potParams[7][3].m_state = PotState::kActive;
  logicalState.m_potParams[7][3].m_value = 0;
  logicalState.m_potParams[7][3].m_minValue = 0;
  logicalState.m_potParams[7][3].m_maxValue = 1023;
  memoryService.handleEvent({EventType::kSaveLogicalState, 0, {}});

  logicalState.m_bypassState = BypassState::kBypassed;
  logicalState.m_currentProgram = 1;
  logicalState.m_currentPreset = 3;
  logicalState.m_currentPresetBank = 4;
  logicalState.m_midiChannel = 7;
  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kEight;
  logicalState.m_interval = 512;
  logicalState.m_divInterval = 256;
  logicalState.m_tempo = 256;

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(2, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPresetBank);
  TEST_ASSERT_EQUAL(0, logicalState.m_midiChannel);
  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, logicalState.m_divValue);
  TEST_ASSERT_EQUAL(0, logicalState.m_interval);
  TEST_ASSERT_EQUAL(0, logicalState.m_divInterval);
  TEST_ASSERT_EQUAL(256, logicalState.m_tempo);

  for (uint8_t i = 0; i < ProgramConstants::c_maxPrograms; i++) {
    auto& expr = logicalState.m_exprParams[i];

    TEST_ASSERT_EQUAL(ExprState::kInactive, expr.m_state);
    TEST_ASSERT_EQUAL(MappedPot::kPot0, expr.m_mappedPot);
    TEST_ASSERT_EQUAL(Direction::kNormal, expr.m_direction);
    TEST_ASSERT_EQUAL(0, expr.m_heelValue);
    TEST_ASSERT_EQUAL(1023, expr.m_toeValue);
  }

  for (uint8_t i = 0; i < ProgramConstants::c_maxPrograms; i++) {
    for (uint8_t j = 0; j < PotConstants::c_potCount; i++) {
      auto& pot = logicalState.m_potParams[i][j];

      TEST_ASSERT_EQUAL(PotState::kActive, pot.m_state);
      TEST_ASSERT_EQUAL(0, pot.m_value);
      TEST_ASSERT_EQUAL(0, pot.m_minValue);
      TEST_ASSERT_EQUAL(1023, pot.m_maxValue);
    }
  }
}

void setup() {
  delay(2000);

  UNITY_BEGIN();
  RUN_TEST(test_logical_state);
  UNITY_END();
}

void loop() {
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(500);
}
