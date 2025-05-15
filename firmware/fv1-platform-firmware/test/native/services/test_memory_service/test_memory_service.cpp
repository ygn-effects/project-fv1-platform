#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"
#include "logic/preset_handler.h"
#include "services/memory_service.h"
#include "mock/mock_eeprom.h"
#include <algorithm>

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
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  logicalState.m_bypassState = BypassState::kActive;
  logicalState.m_currentProgram = 2;
  memoryService.handleEvent({EventType::kSaveLogicalState, 0, {}});

  logicalState.m_bypassState = BypassState::kBypassed;
  logicalState.m_currentProgram = 0;

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
}

void test_bypass() {
  LogicalState logicalState;
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  logicalState.m_bypassState = BypassState::kBypassed;
  memoryService.handleEvent({EventType::kSaveBypass, 0, {}});

  logicalState.m_bypassState = BypassState::kActive;

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
}

void test_program_mode() {
  LogicalState logicalState;
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  logicalState.m_programMode = ProgramMode::kPreset;
  memoryService.handleEvent({EventType::kSaveProgramMode, 0, {}});

  logicalState.m_programMode = ProgramMode::kProgram;

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);
}

void test_current_program() {
  LogicalState logicalState;
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  logicalState.m_currentProgram = 3;
  memoryService.handleEvent({EventType::kSaveCurrentProgram, 0, {}});

  logicalState.m_currentProgram = 0;

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(3, logicalState.m_currentProgram);
}

void test_current_preset() {
  LogicalState logicalState;
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  logicalState.m_currentPreset = 4;
  memoryService.handleEvent({EventType::kSaveCurrentPreset, 0, {}});

  logicalState.m_currentPreset = 0;

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(4, logicalState.m_currentPreset);
}

void test_midi_channel() {
  LogicalState logicalState;
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  logicalState.m_midiChannel = 7;
  memoryService.handleEvent({EventType::kSaveMidiChannel, 0, {}});

  logicalState.m_midiChannel = 0;

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(7, logicalState.m_midiChannel);
}

void test_tap() {
  LogicalState logicalState;
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kEight;
  logicalState.m_interval = 512;
  logicalState.m_divInterval = 256;
  memoryService.handleEvent({EventType::kSaveTap, 0, {}});

  logicalState.m_tapState = TapState::kDisabled;
  logicalState.m_divState = DivState::kDisabled;
  logicalState.m_divValue = DivValue::kQuarter;
  logicalState.m_interval = 0;
  logicalState.m_divInterval = 0;

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, logicalState.m_divValue);
  TEST_ASSERT_EQUAL(512, logicalState.m_interval);
  TEST_ASSERT_EQUAL(256, logicalState.m_divInterval);
}

void test_tempo() {
  LogicalState logicalState;
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  logicalState.m_tempo = 512;
  memoryService.handleEvent({EventType::kSaveTempo, 0, {}});

  logicalState.m_tempo = 0;

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(512, logicalState.m_tempo);
}

void test_expr() {
  LogicalState logicalState;
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  logicalState.m_currentProgram = 2;
  logicalState.m_exprParams[2].m_state = ExprState::kActive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot1;
  logicalState.m_exprParams[2].m_direction = Direction::kInverted;
  logicalState.m_exprParams[2].m_heelValue = 256;
  logicalState.m_exprParams[2].m_toeValue = 512;
  memoryService.handleEvent({EventType::kSaveExpr, 0, {}});

  logicalState.m_currentProgram = 0;
  logicalState.m_exprParams[2].m_state = ExprState::kActive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot1;
  logicalState.m_exprParams[2].m_direction = Direction::kInverted;
  logicalState.m_exprParams[2].m_heelValue = 256;
  logicalState.m_exprParams[2].m_toeValue = 512;

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(ExprState::kActive, logicalState.m_exprParams[2].m_state);
  TEST_ASSERT_EQUAL(MappedPot::kPot1, logicalState.m_exprParams[2].m_mappedPot);
  TEST_ASSERT_EQUAL(Direction::kInverted, logicalState.m_exprParams[2].m_direction);
  TEST_ASSERT_EQUAL(256, logicalState.m_exprParams[2].m_heelValue);
  TEST_ASSERT_EQUAL(512, logicalState.m_exprParams[2].m_toeValue);
}

void test_pot() {
  LogicalState logicalState;
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  logicalState.m_currentProgram = 3;
  logicalState.m_potParams[3][2].m_state = PotState::kDisabled;
  logicalState.m_potParams[3][2].m_value = 512;
  logicalState.m_potParams[3][2].m_minValue = 256;
  logicalState.m_potParams[3][2].m_maxValue = 768;
  memoryService.handleEvent({EventType::kSavePot, 0, {.value=2}});

  logicalState.m_currentProgram = 0;
  logicalState.m_potParams[3][2].m_state = PotState::kActive;
  logicalState.m_potParams[3][2].m_value = 0;
  logicalState.m_potParams[3][2].m_minValue = 0;
  logicalState.m_potParams[3][2].m_maxValue = 0;

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(PotState::kDisabled, logicalState.m_potParams[3][2].m_state);
  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[3][2].m_value);
  TEST_ASSERT_EQUAL(256, logicalState.m_potParams[3][2].m_minValue);
  TEST_ASSERT_EQUAL(768, logicalState.m_potParams[3][2].m_maxValue);
}

void test_corrupted_eeprom() {
  LogicalState logicalState;
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  uint8_t buffer[MemoryLayout::c_potParamEnd];

  for (uint16_t i = 0; i < sizeof(buffer); i++) {
    buffer[i] = 255;
  }

  eeprom.write(0, buffer, sizeof(buffer));

  memoryService.handleEvent({EventType::kRestoreState, 0, {}});
  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
}

void test_preset() {
  LogicalState logicalState;
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  PresetBank& bank = presetHandler.m_currentPresetBank;
  Preset& preset = presetHandler.m_currentPresetBank.m_presets[0];

  bank.m_id = 0;
  preset.m_id = 0;
  preset.m_programIndex = 3;
  preset.m_tapState = TapState::kEnabled;
  preset.m_divState = DivState::kEnabled;
  preset.m_divValue = DivValue::kEight;
  preset.m_interval = 512;
  preset.m_divInterval = 256;
  preset.m_tempo = 256;
  preset.m_exprState = ExprState::kActive;
  preset.m_mappedPot = MappedPot::kPot2;
  preset.m_direction = Direction::kInverted;
  preset.m_heelValue = 256;
  preset.m_toeValue = 512;
  preset.m_potParams[3].m_state = PotState::kActive;
  preset.m_potParams[3].m_value = 512;
  preset.m_potParams[3].m_minValue = 256;
  preset.m_potParams[3].m_maxValue = 768;

  for (uint8_t i = 0; i < PresetConstants::c_presetPerBank; i++) {
    uint16_t preset = 0;
    Utils::unpack16(i, 0, preset);
    memoryService.handleEvent({EventType::kSavePreset, 0, {.value=preset}});
  }

  preset = presetHandler.m_currentPresetBank.m_presets[0];

  bank.m_id = 1;
  preset.m_id = 2;
  preset.m_programIndex = 4;
  preset.m_tapState = TapState::kDisabled;
  preset.m_divState = DivState::kDisabled;
  preset.m_divValue = DivValue::kQuarter;
  preset.m_interval = 0;
  preset.m_divInterval = 0;
  preset.m_tempo = 0;
  preset.m_exprState = ExprState::kInactive;
  preset.m_mappedPot = MappedPot::kPot0;
  preset.m_direction = Direction::kNormal;
  preset.m_heelValue = 0;
  preset.m_toeValue = 0;
  preset.m_potParams[3].m_state = PotState::kDisabled;
  preset.m_potParams[3].m_value = 0;
  preset.m_potParams[3].m_minValue = 0;
  preset.m_potParams[3].m_maxValue = 0;

  memoryService.handleEvent({EventType::kLoadPresetBank, 0, {.value=0}});

  preset = presetHandler.m_currentPresetBank.m_presets[0];

  TEST_ASSERT_EQUAL(0, bank.m_id);
  TEST_ASSERT_EQUAL(0, preset.m_id);
  TEST_ASSERT_EQUAL(3, preset.m_programIndex);
  TEST_ASSERT_EQUAL(TapState::kEnabled, preset.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, preset.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, preset.m_divValue);
  TEST_ASSERT_EQUAL(512, preset.m_interval);
  TEST_ASSERT_EQUAL(256, preset.m_divInterval);
  TEST_ASSERT_EQUAL(256, preset.m_tempo);
  TEST_ASSERT_EQUAL(ExprState::kActive, preset.m_exprState);
  TEST_ASSERT_EQUAL(MappedPot::kPot2, preset.m_mappedPot);
  TEST_ASSERT_EQUAL(Direction::kInverted, preset.m_direction);
  TEST_ASSERT_EQUAL(256, preset.m_heelValue);
  TEST_ASSERT_EQUAL(512, preset.m_toeValue);
  TEST_ASSERT_EQUAL(PotState::kActive, preset.m_potParams[3].m_state);
  TEST_ASSERT_EQUAL(512, preset.m_potParams[3].m_value);
  TEST_ASSERT_EQUAL(256, preset.m_potParams[3].m_minValue);
  TEST_ASSERT_EQUAL(768, preset.m_potParams[3].m_maxValue);
}

void test_interested_in() {
  LogicalState logicalState;
  PresetHandler presetHandler;
  MockEEPROM eeprom;

  MemoryService memoryService(logicalState, presetHandler.m_currentPresetBank, eeprom);

  Event e{EventType::kSaveBypass, 500, {.value=100}};
  TEST_ASSERT_TRUE(memoryService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kRestoreState, 500, {.delta=1}};
  TEST_ASSERT_TRUE(memoryService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kBootCompleted, 500, {.delta=1}};
  TEST_ASSERT_TRUE(memoryService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kExprMoved, 500, {.delta=1}};
  TEST_ASSERT_FALSE(memoryService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));

  e = {EventType::kMenuExprMappedPotMoved, 500, {.delta=1}};
  TEST_ASSERT_FALSE(memoryService.interestedIn(eventToCategory(e.m_type), EventToSubCategory(e.m_type)));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_logical_state);
  RUN_TEST(test_bypass);
  RUN_TEST(test_program_mode);
  RUN_TEST(test_current_program);
  RUN_TEST(test_current_preset);
  RUN_TEST(test_midi_channel);
  RUN_TEST(test_tap);
  RUN_TEST(test_tempo);
  RUN_TEST(test_expr);
  RUN_TEST(test_pot);
  RUN_TEST(test_corrupted_eeprom);
  RUN_TEST(test_preset);
  RUN_TEST(test_interested_in);
  UNITY_END();
}
