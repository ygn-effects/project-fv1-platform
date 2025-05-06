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
  RUN_TEST(test_validators);
  UNITY_END();
}
