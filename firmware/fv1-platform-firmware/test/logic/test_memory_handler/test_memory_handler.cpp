#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"

#include "../src/logic/memory_handler.cpp"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_serialize() {
  LogicalState logicalState;
  MemoryHandler memoryHandler;

  // Bypass
  RegionInfo info = memoryHandler.calculateRegionInfo(MemoryRegion::kBypass);
  TEST_ASSERT_EQUAL(info.m_address, 0);
  TEST_ASSERT_EQUAL(info.m_length, 1);

  uint8_t buffer[info.m_length];
  memoryHandler.serializeRegion(MemoryRegion::kBypass, logicalState, buffer);
  TEST_ASSERT_EQUAL(1, buffer[0]);

  logicalState.m_bypassState = BypassState::kBypassed;
  memoryHandler.serializeRegion(MemoryRegion::kBypass, logicalState, buffer);
  TEST_ASSERT_EQUAL(0, buffer[0]);

  // Program mode
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kProgramMode);
  TEST_ASSERT_EQUAL(info.m_address, 1);
  TEST_ASSERT_EQUAL(info.m_length, 1);

  memoryHandler.serializeRegion(MemoryRegion::kProgramMode, logicalState, buffer);
  TEST_ASSERT_EQUAL(0, buffer[0]);

  logicalState.m_programMode = ProgramMode::kPreset;
  memoryHandler.serializeRegion(MemoryRegion::kProgramMode, logicalState, buffer);
  TEST_ASSERT_EQUAL(1, buffer[0]);

  // Current  program
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kCurrentProgram);
  TEST_ASSERT_EQUAL(info.m_address, 2);
  TEST_ASSERT_EQUAL(info.m_length, 1);

  memoryHandler.serializeRegion(MemoryRegion::kCurrentProgram, logicalState, buffer);
  TEST_ASSERT_EQUAL(0, buffer[0]);

  logicalState.m_currentProgram = 2;
  memoryHandler.serializeRegion(MemoryRegion::kCurrentProgram, logicalState, buffer);
  TEST_ASSERT_EQUAL(2, buffer[0]);

  // Current  preset
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kCurrentPreset);
  TEST_ASSERT_EQUAL(info.m_address, 3);
  TEST_ASSERT_EQUAL(info.m_length, 1);

  memoryHandler.serializeRegion(MemoryRegion::kCurrentPreset, logicalState, buffer);
  TEST_ASSERT_EQUAL(0, buffer[0]);

  logicalState.m_currentPreset = 3;
  memoryHandler.serializeRegion(MemoryRegion::kCurrentPreset, logicalState, buffer);
  TEST_ASSERT_EQUAL(3, buffer[0]);

  // MIDI channel
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kMidiChannel);
  TEST_ASSERT_EQUAL(info.m_address, 4);
  TEST_ASSERT_EQUAL(info.m_length, 1);

  memoryHandler.serializeRegion(MemoryRegion::kMidiChannel, logicalState, buffer);
  TEST_ASSERT_EQUAL(0, buffer[0]);

  logicalState.m_midiChannel = 7;
  memoryHandler.serializeRegion(MemoryRegion::kMidiChannel, logicalState, buffer);
  TEST_ASSERT_EQUAL(7, buffer[0]);

  // Tap
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kTap);
  TEST_ASSERT_EQUAL(info.m_address, 5);
  TEST_ASSERT_EQUAL(info.m_length, 7);

  uint8_t tapBuffer[info.m_length];
  memoryHandler.serializeRegion(MemoryRegion::kTap, logicalState, tapBuffer);
  TEST_ASSERT_EQUAL(0, tapBuffer[0]);
  TEST_ASSERT_EQUAL(0, tapBuffer[1]);
  TEST_ASSERT_EQUAL(0, tapBuffer[2]);
  TEST_ASSERT_EQUAL(0, tapBuffer[3]);
  TEST_ASSERT_EQUAL(0, tapBuffer[4]);
  TEST_ASSERT_EQUAL(0, tapBuffer[5]);
  TEST_ASSERT_EQUAL(0, tapBuffer[6]);

  logicalState.m_tapState = TapState::kEnabled;
  logicalState.m_divState = DivState::kEnabled;
  logicalState.m_divValue = DivValue::kEight;
  logicalState.m_interval = 1024;
  logicalState.m_divInterval = 512;
  memoryHandler.serializeRegion(MemoryRegion::kTap, logicalState, tapBuffer);
  TEST_ASSERT_EQUAL(1, tapBuffer[0]);
  TEST_ASSERT_EQUAL(1, tapBuffer[1]);
  TEST_ASSERT_EQUAL(1, tapBuffer[2]);
  uint16_t interval = 0;
  Utils::unpack16(tapBuffer[3], tapBuffer[4], interval);
  TEST_ASSERT_EQUAL(1024, interval);
  uint16_t divInterval = 0;
  Utils::unpack16(tapBuffer[5], tapBuffer[6], divInterval);
  TEST_ASSERT_EQUAL(512, divInterval);

  // Tempo
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kTempo);
  TEST_ASSERT_EQUAL(info.m_address, 12);
  TEST_ASSERT_EQUAL(info.m_length, 2);

  uint8_t tempoBuffer[info.m_length];
  memoryHandler.serializeRegion(MemoryRegion::kTempo, logicalState, tempoBuffer);
  TEST_ASSERT_EQUAL(0, tempoBuffer[0]);
  TEST_ASSERT_EQUAL(0, tempoBuffer[1]);

  logicalState.m_tempo = 568;
  memoryHandler.serializeRegion(MemoryRegion::kTempo, logicalState, tempoBuffer);
  uint16_t tempo = 0;
  Utils::unpack16(tempoBuffer[0], tempoBuffer[1], tempo);
  TEST_ASSERT_EQUAL(568, tempo);

  // Expr
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kExpr, 2);
  TEST_ASSERT_EQUAL(info.m_address, 28);
  TEST_ASSERT_EQUAL(info.m_length, 7);

  uint8_t exprBuffer[info.m_length];
  memoryHandler.serializeRegion(MemoryRegion::kExpr, logicalState, exprBuffer, 2);
  TEST_ASSERT_EQUAL(0, exprBuffer[0]);
  TEST_ASSERT_EQUAL(0, exprBuffer[1]);
  TEST_ASSERT_EQUAL(0, exprBuffer[2]);
  TEST_ASSERT_EQUAL(0, exprBuffer[3]);
  TEST_ASSERT_EQUAL(0, exprBuffer[4]);
  uint16_t toe = 0;
  Utils::unpack16(exprBuffer[5], exprBuffer[6], toe);
  TEST_ASSERT_EQUAL(1023, toe);

  logicalState.m_exprParams[2].m_state = ExprState::kActive;
  logicalState.m_exprParams[2].m_mappedPot = MappedPot::kPot1;
  logicalState.m_exprParams[2].m_direction = Direction::kInverted;
  logicalState.m_exprParams[2].m_heelValue = 600;
  logicalState.m_exprParams[2].m_toeValue = 800;
  memoryHandler.serializeRegion(MemoryRegion::kExpr, logicalState, exprBuffer, 2);
  TEST_ASSERT_EQUAL(1, exprBuffer[0]);
  TEST_ASSERT_EQUAL(1, exprBuffer[1]);
  TEST_ASSERT_EQUAL(1, exprBuffer[2]);
  uint16_t heel = 0;
  Utils::unpack16(exprBuffer[3], exprBuffer[4], heel);
  TEST_ASSERT_EQUAL(600, heel);
  Utils::unpack16(exprBuffer[5], exprBuffer[6], toe);
  TEST_ASSERT_EQUAL(800, toe);

  // Pot
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kPot, 3, 2);
  TEST_ASSERT_EQUAL(168, info.m_address);
  TEST_ASSERT_EQUAL(7, info.m_length);

  uint8_t potBuffer[info.m_length];
  memoryHandler.serializeRegion(MemoryRegion::kPot, logicalState, potBuffer, 3, 2);
  TEST_ASSERT_EQUAL(1, potBuffer[0]);
  TEST_ASSERT_EQUAL(0, potBuffer[1]);
  TEST_ASSERT_EQUAL(0, potBuffer[2]);
  TEST_ASSERT_EQUAL(0, potBuffer[3]);
  TEST_ASSERT_EQUAL(0, potBuffer[4]);
  TEST_ASSERT_EQUAL(255, potBuffer[5]);
  TEST_ASSERT_EQUAL(3, potBuffer[6]);

  logicalState.m_potParams[3][1].m_state = PotState::kDisabled;
  logicalState.m_potParams[3][1].m_minValue = 128;
  logicalState.m_potParams[3][1].m_maxValue = 768;
  logicalState.m_potParams[3][1].m_value = 512;
  memoryHandler.serializeRegion(MemoryRegion::kPot, logicalState, potBuffer, 3, 1);
  TEST_ASSERT_EQUAL(0, potBuffer[0]);
  TEST_ASSERT_EQUAL(0, potBuffer[1]);
  TEST_ASSERT_EQUAL(2, potBuffer[2]);
  TEST_ASSERT_EQUAL(128, potBuffer[3]);
  TEST_ASSERT_EQUAL(0, potBuffer[4]);
  TEST_ASSERT_EQUAL(0, potBuffer[5]);
  TEST_ASSERT_EQUAL(3, potBuffer[6]);
}

void test_deserialize() {
  LogicalState logicalState;
  MemoryHandler memoryHandler;

  // Bypass
  RegionInfo info = memoryHandler.calculateRegionInfo(MemoryRegion::kBypass);
  uint8_t bypassBuffer[info.m_length];
  bypassBuffer[0] = static_cast<uint8_t>(BypassState::kBypassed);
  memoryHandler.deserializeRegion(MemoryRegion::kBypass, logicalState, bypassBuffer);

  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);

  bypassBuffer[0] = static_cast<uint8_t>(BypassState::kActive);
  memoryHandler.deserializeRegion(MemoryRegion::kBypass, logicalState, bypassBuffer);

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);

  // Program mode
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kProgramMode);
  uint8_t programModeBuffer[info.m_length];
  programModeBuffer[0] = static_cast<uint8_t>(ProgramMode::kProgram);
  memoryHandler.deserializeRegion(MemoryRegion::kProgramMode, logicalState, programModeBuffer);

  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);

  programModeBuffer[0] = static_cast<uint8_t>(ProgramMode::kPreset);
  memoryHandler.deserializeRegion(MemoryRegion::kProgramMode, logicalState, programModeBuffer);

  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);

  // Current program
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kCurrentProgram);
  uint8_t currentProgramBuffer[info.m_length];
  currentProgramBuffer[0] = 1;
  memoryHandler.deserializeRegion(MemoryRegion::kCurrentProgram, logicalState, currentProgramBuffer);

  TEST_ASSERT_EQUAL(1, logicalState.m_currentProgram);

  currentProgramBuffer[0] = 2;
  memoryHandler.deserializeRegion(MemoryRegion::kCurrentProgram, logicalState, currentProgramBuffer);

  TEST_ASSERT_EQUAL(2, logicalState.m_currentProgram);

  // Current preset
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kCurrentPreset);
  uint8_t currentPresetBuffer[info.m_length];
  currentPresetBuffer[0] = 3;
  memoryHandler.deserializeRegion(MemoryRegion::kCurrentPreset, logicalState, currentPresetBuffer);

  TEST_ASSERT_EQUAL(3, logicalState.m_currentPreset);

  currentPresetBuffer[0] = 4;
  memoryHandler.deserializeRegion(MemoryRegion::kCurrentPreset, logicalState, currentPresetBuffer);

  TEST_ASSERT_EQUAL(4, logicalState.m_currentPreset);

  // Midi Channel
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kMidiChannel);
  uint8_t midiChannelBuffer[info.m_length];
  midiChannelBuffer[0] = 5;
  memoryHandler.deserializeRegion(MemoryRegion::kMidiChannel, logicalState, midiChannelBuffer);

  TEST_ASSERT_EQUAL(5, logicalState.m_midiChannel);

  midiChannelBuffer[0] = 6;
  memoryHandler.deserializeRegion(MemoryRegion::kMidiChannel, logicalState, midiChannelBuffer);

  TEST_ASSERT_EQUAL(6, logicalState.m_midiChannel);

  // Device state
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kDeviceState);
  uint8_t deviceStateBuffer[info.m_length];
  deviceStateBuffer[0] = static_cast<uint8_t>(BypassState::kActive);
  deviceStateBuffer[1] = static_cast<uint8_t>(ProgramMode::kProgram);
  deviceStateBuffer[2] = 3;
  deviceStateBuffer[3] = 0;
  deviceStateBuffer[4] = 7;
  memoryHandler.deserializeRegion(MemoryRegion::kDeviceState, logicalState, deviceStateBuffer);

  TEST_ASSERT_EQUAL(BypassState::kActive, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, logicalState.m_programMode);
  TEST_ASSERT_EQUAL(3, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(7, logicalState.m_midiChannel);

  deviceStateBuffer[0] = static_cast<uint8_t>(BypassState::kBypassed);
  deviceStateBuffer[1] = static_cast<uint8_t>(ProgramMode::kPreset);
  deviceStateBuffer[2] = 0;
  deviceStateBuffer[3] = 5;
  deviceStateBuffer[4] = 2;
  memoryHandler.deserializeRegion(MemoryRegion::kDeviceState, logicalState, deviceStateBuffer);

  TEST_ASSERT_EQUAL(BypassState::kBypassed, logicalState.m_bypassState);
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, logicalState.m_programMode);
  TEST_ASSERT_EQUAL(0, logicalState.m_currentProgram);
  TEST_ASSERT_EQUAL(5, logicalState.m_currentPreset);
  TEST_ASSERT_EQUAL(2, logicalState.m_midiChannel);

  // Tap
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kTap);
  uint8_t tapBuffer[info.m_length];
  tapBuffer[0] = static_cast<uint8_t>(TapState::kEnabled);
  tapBuffer[1] = static_cast<uint8_t>(DivState::kDisabled);
  tapBuffer[2] = static_cast<uint8_t>(DivValue::kQuarter);
  tapBuffer[3] = 0;
  tapBuffer[4] = 1;
  tapBuffer[5] = 0;
  tapBuffer[6] = 0;
  memoryHandler.deserializeRegion(MemoryRegion::kTap, logicalState, tapBuffer);

  TEST_ASSERT_EQUAL(TapState::kEnabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, logicalState.m_divValue);
  TEST_ASSERT_EQUAL(256, logicalState.m_interval);
  TEST_ASSERT_EQUAL(0, logicalState.m_divInterval);

  tapBuffer[0] = static_cast<uint8_t>(TapState::kDisabled);
  tapBuffer[1] = static_cast<uint8_t>(DivState::kEnabled);
  tapBuffer[2] = static_cast<uint8_t>(DivValue::kEight);
  tapBuffer[3] = 0;
  tapBuffer[4] = 2;
  tapBuffer[5] = 0;
  tapBuffer[6] = 1;
  memoryHandler.deserializeRegion(MemoryRegion::kTap, logicalState, tapBuffer);

  TEST_ASSERT_EQUAL(TapState::kDisabled, logicalState.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, logicalState.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, logicalState.m_divValue);
  TEST_ASSERT_EQUAL(512, logicalState.m_interval);
  TEST_ASSERT_EQUAL(256, logicalState.m_divInterval);

  // Tempo
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kTempo);
  uint8_t tempoBuffer[info.m_length];
  tempoBuffer[0] = 0;
  tempoBuffer[1] = 1;
  memoryHandler.deserializeRegion(MemoryRegion::kTempo, logicalState, tempoBuffer);

  TEST_ASSERT_EQUAL(256, logicalState.m_tempo);

  tempoBuffer[0] = 0;
  tempoBuffer[1] = 2;
  memoryHandler.deserializeRegion(MemoryRegion::kTempo, logicalState, tempoBuffer);

  TEST_ASSERT_EQUAL(512, logicalState.m_tempo);

  // Expr
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kExpr, 2);
  uint8_t exprBuffer[info.m_length];
  exprBuffer[0] = static_cast<uint8_t>(ExprState::kActive);
  exprBuffer[1] = static_cast<uint8_t>(MappedPot::kPot1);
  exprBuffer[2] = static_cast<uint8_t>(Direction::kNormal);
  exprBuffer[3] = 0;
  exprBuffer[4] = 1;
  exprBuffer[5] = 0;
  exprBuffer[6] = 2;
  memoryHandler.deserializeRegion(MemoryRegion::kExpr, logicalState, exprBuffer, 2);

  TEST_ASSERT_EQUAL(ExprState::kActive, logicalState.m_exprParams[2].m_state);
  TEST_ASSERT_EQUAL(MappedPot::kPot1, logicalState.m_exprParams[2].m_mappedPot);
  TEST_ASSERT_EQUAL(Direction::kNormal, logicalState.m_exprParams[2].m_direction);
  TEST_ASSERT_EQUAL(256, logicalState.m_exprParams[2].m_heelValue);
  TEST_ASSERT_EQUAL(512, logicalState.m_exprParams[2].m_toeValue);

  exprBuffer[0] = static_cast<uint8_t>(ExprState::kInactive);
  exprBuffer[1] = static_cast<uint8_t>(MappedPot::kMixPot);
  exprBuffer[2] = static_cast<uint8_t>(Direction::kInverted);
  exprBuffer[3] = 0;
  exprBuffer[4] = 1;
  exprBuffer[5] = 0;
  exprBuffer[6] = 2;
  memoryHandler.deserializeRegion(MemoryRegion::kExpr, logicalState, exprBuffer, 1);

  TEST_ASSERT_EQUAL(ExprState::kActive, logicalState.m_exprParams[2].m_state);
  TEST_ASSERT_EQUAL(MappedPot::kPot1, logicalState.m_exprParams[2].m_mappedPot);
  TEST_ASSERT_EQUAL(Direction::kNormal, logicalState.m_exprParams[2].m_direction);
  TEST_ASSERT_EQUAL(256, logicalState.m_exprParams[2].m_heelValue);
  TEST_ASSERT_EQUAL(512, logicalState.m_exprParams[2].m_toeValue);

  TEST_ASSERT_EQUAL(ExprState::kInactive, logicalState.m_exprParams[1].m_state);
  TEST_ASSERT_EQUAL(MappedPot::kMixPot, logicalState.m_exprParams[1].m_mappedPot);
  TEST_ASSERT_EQUAL(Direction::kInverted, logicalState.m_exprParams[1].m_direction);
  TEST_ASSERT_EQUAL(256, logicalState.m_exprParams[1].m_heelValue);
  TEST_ASSERT_EQUAL(512, logicalState.m_exprParams[1].m_toeValue);

  // Pot
  info = memoryHandler.calculateRegionInfo(MemoryRegion::kExpr, 3, 2);
  uint8_t potBuffer[info.m_length];
  potBuffer[0] = static_cast<uint8_t>(PotState::kActive);
  potBuffer[1] = 0;
  potBuffer[2] = 2;
  potBuffer[3] = 128;
  potBuffer[4] = 0;
  potBuffer[5] = 0;
  potBuffer[6] = 3;
  memoryHandler.deserializeRegion(MemoryRegion::kPot, logicalState, potBuffer, 3, 2);

  TEST_ASSERT_EQUAL(PotState::kActive, logicalState.m_potParams[3][2].m_state);
  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[3][2].m_value);
  TEST_ASSERT_EQUAL(128, logicalState.m_potParams[3][2].m_minValue);
  TEST_ASSERT_EQUAL(768, logicalState.m_potParams[3][2].m_maxValue);

  potBuffer[0] = static_cast<uint8_t>(PotState::kDisabled);
  potBuffer[1] = 0;
  potBuffer[2] = 1;
  potBuffer[3] = 127;
  potBuffer[4] = 0;
  potBuffer[5] = 0;
  potBuffer[6] = 2;
  memoryHandler.deserializeRegion(MemoryRegion::kPot, logicalState, potBuffer, 3, 3);

  TEST_ASSERT_EQUAL(PotState::kDisabled, logicalState.m_potParams[3][3].m_state);
  TEST_ASSERT_EQUAL(256, logicalState.m_potParams[3][3].m_value);
  TEST_ASSERT_EQUAL(127, logicalState.m_potParams[3][3].m_minValue);
  TEST_ASSERT_EQUAL(512, logicalState.m_potParams[3][3].m_maxValue);
}

void test_validators() {
  TEST_ASSERT_EQUAL(BypassState::kBypassed, LogicalStateValidator::setSafeBypassState(0));
  TEST_ASSERT_EQUAL(BypassState::kActive, LogicalStateValidator::setSafeBypassState(1));
  TEST_ASSERT_EQUAL(BypassState::kActive, LogicalStateValidator::setSafeBypassState(2));

  TEST_ASSERT_EQUAL(ProgramMode::kProgram, LogicalStateValidator::setSafeProgramMode(0));
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, LogicalStateValidator::setSafeProgramMode(1));
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, LogicalStateValidator::setSafeProgramMode(2));

  TEST_ASSERT_EQUAL(ExprState::kInactive, LogicalStateValidator::setSafeExprState(0));
  TEST_ASSERT_EQUAL(ExprState::kActive, LogicalStateValidator::setSafeExprState(1));
  TEST_ASSERT_EQUAL(ExprState::kInactive, LogicalStateValidator::setSafeExprState(2));

  TEST_ASSERT_EQUAL(MappedPot::kPot0, LogicalStateValidator::setSafeMappedPot(0));
  TEST_ASSERT_EQUAL(MappedPot::kPot1, LogicalStateValidator::setSafeMappedPot(1));
  TEST_ASSERT_EQUAL(MappedPot::kPot2, LogicalStateValidator::setSafeMappedPot(2));
  TEST_ASSERT_EQUAL(MappedPot::kMixPot, LogicalStateValidator::setSafeMappedPot(3));
  TEST_ASSERT_EQUAL(MappedPot::kPot0, LogicalStateValidator::setSafeMappedPot(4));

  TEST_ASSERT_EQUAL(Direction::kNormal, LogicalStateValidator::setSafeDirection(0));
  TEST_ASSERT_EQUAL(Direction::kInverted, LogicalStateValidator::setSafeDirection(1));
  TEST_ASSERT_EQUAL(Direction::kNormal, LogicalStateValidator::setSafeDirection(2));

  TEST_ASSERT_EQUAL(PotState::kDisabled, LogicalStateValidator::setSafePotState(0));
  TEST_ASSERT_EQUAL(PotState::kActive, LogicalStateValidator::setSafePotState(1));
  TEST_ASSERT_EQUAL(PotState::kActive, LogicalStateValidator::setSafePotState(2));

  TEST_ASSERT_EQUAL(TapState::kDisabled, LogicalStateValidator::setSafeTapState(0));
  TEST_ASSERT_EQUAL(TapState::kEnabled, LogicalStateValidator::setSafeTapState(1));
  TEST_ASSERT_EQUAL(TapState::kDisabled, LogicalStateValidator::setSafeTapState(2));

  TEST_ASSERT_EQUAL(DivState::kDisabled, LogicalStateValidator::setSafeDivState(0));
  TEST_ASSERT_EQUAL(DivState::kEnabled, LogicalStateValidator::setSafeDivState(1));
  TEST_ASSERT_EQUAL(DivState::kDisabled, LogicalStateValidator::setSafeDivState(2));

  TEST_ASSERT_EQUAL(DivValue::kQuarter, LogicalStateValidator::setSafeDivValue(0));
  TEST_ASSERT_EQUAL(DivValue::kEight, LogicalStateValidator::setSafeDivValue(1));
  TEST_ASSERT_EQUAL(DivValue::kSixteenth, LogicalStateValidator::setSafeDivValue(2));
  TEST_ASSERT_EQUAL(DivValue::kDottedEight, LogicalStateValidator::setSafeDivValue(3));
  TEST_ASSERT_EQUAL(DivValue::kEightTriplet, LogicalStateValidator::setSafeDivValue(4));
  TEST_ASSERT_EQUAL(DivValue::kQuarter, LogicalStateValidator::setSafeDivValue(5));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_serialize);
  RUN_TEST(test_deserialize);
  RUN_TEST(test_validators);
  UNITY_END();
}
