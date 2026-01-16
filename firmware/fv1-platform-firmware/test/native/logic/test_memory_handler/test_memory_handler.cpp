#include <unity.h>
#include <cstring>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/memory_handler.h"
#include "logic/preset_handler.h"

#include "../src/logic/memory_handler.cpp"
#include "../src/logic/preset_handler.cpp"

// =============================================================================
// Helper Functions
// =============================================================================

LogicalState makeDefaultLogicalState() {
  return LogicalState();
}

LogicalState makeLogicalStateWithTap(TapState t_tapState, DivState t_divState,
                                      DivValue t_divValue, uint16_t t_interval,
                                      uint16_t t_divInterval) {
  LogicalState state;
  state.m_tapState = t_tapState;
  state.m_divState = t_divState;
  state.m_divValue = t_divValue;
  state.m_interval = t_interval;
  state.m_divInterval = t_divInterval;
  return state;
}

LogicalState makeLogicalStateWithExpr(uint8_t t_programIndex, ExprState t_state,
                                       MappedPot t_mappedPot, Direction t_direction,
                                       uint16_t t_heelValue, uint16_t t_toeValue) {
  LogicalState state;
  state.m_exprParams[t_programIndex].m_state = t_state;
  state.m_exprParams[t_programIndex].m_mappedPot = t_mappedPot;
  state.m_exprParams[t_programIndex].m_direction = t_direction;
  state.m_exprParams[t_programIndex].m_heelValue = t_heelValue;
  state.m_exprParams[t_programIndex].m_toeValue = t_toeValue;
  return state;
}

Preset makeDefaultPreset() {
  return Preset();
}

Preset makePresetWithValues(uint8_t t_programIndex, TapState t_tapState,
                             uint16_t t_interval, ExprState t_exprState) {
  Preset preset;
  preset.m_programIndex = t_programIndex;
  preset.m_tapState = t_tapState;
  preset.m_interval = t_interval;
  preset.m_exprState = t_exprState;
  return preset;
}

void setUp() {
  Event event;
  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {}

// =============================================================================
// calculateRegionInfo Tests
// =============================================================================

void test_calculateRegionInfo_bypass() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kBypass);

  TEST_ASSERT_EQUAL(0, info.m_address);
  TEST_ASSERT_EQUAL(1, info.m_length);
}

void test_calculateRegionInfo_programMode() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kProgramMode);

  TEST_ASSERT_EQUAL(1, info.m_address);
  TEST_ASSERT_EQUAL(1, info.m_length);
}

void test_calculateRegionInfo_currentProgram() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kCurrentProgram);

  TEST_ASSERT_EQUAL(2, info.m_address);
  TEST_ASSERT_EQUAL(1, info.m_length);
}

void test_calculateRegionInfo_currentPreset() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kCurrentPreset);

  TEST_ASSERT_EQUAL(3, info.m_address);
  TEST_ASSERT_EQUAL(1, info.m_length);
}

void test_calculateRegionInfo_currentPresetBank() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kCurrentPresetBank);

  TEST_ASSERT_EQUAL(4, info.m_address);
  TEST_ASSERT_EQUAL(1, info.m_length);
}

void test_calculateRegionInfo_midiChannel() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kMidiChannel);

  TEST_ASSERT_EQUAL(5, info.m_address);
  TEST_ASSERT_EQUAL(1, info.m_length);
}

void test_calculateRegionInfo_tap() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kTap);

  TEST_ASSERT_EQUAL(6, info.m_address);
  TEST_ASSERT_EQUAL(7, info.m_length);
}

void test_calculateRegionInfo_tempo() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kTempo);

  TEST_ASSERT_EQUAL(13, info.m_address);
  TEST_ASSERT_EQUAL(2, info.m_length);
}

void test_calculateRegionInfo_expr_program_index_0() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kExpr, 0);

  TEST_ASSERT_EQUAL(15, info.m_address);
  TEST_ASSERT_EQUAL(7, info.m_length);
}

void test_calculateRegionInfo_expr_program_index_7() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kExpr, 7);

  TEST_ASSERT_EQUAL(15 + 7 * 7, info.m_address);
  TEST_ASSERT_EQUAL(7, info.m_length);
}

void test_calculateRegionInfo_pot_program_0_pot_0() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kPot, 0, 0);

  TEST_ASSERT_EQUAL(71, info.m_address);
  TEST_ASSERT_EQUAL(7, info.m_length);
}

void test_calculateRegionInfo_pot_program_3_pot_2() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kPot, 3, 2);

  TEST_ASSERT_EQUAL(169, info.m_address);
  TEST_ASSERT_EQUAL(7, info.m_length);
}

void test_calculateRegionInfo_deviceState() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kDeviceState);

  TEST_ASSERT_EQUAL(0, info.m_address);
  TEST_ASSERT_EQUAL(6, info.m_length);
}

void test_calculateRegionInfo_logicalState() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kLogicalState);

  TEST_ASSERT_EQUAL(0, info.m_address);
  TEST_ASSERT_EQUAL(295, info.m_length);
}

void test_calculateRegionInfo_preset() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kPreset, 2, 2);

  TEST_ASSERT_EQUAL(759, info.m_address);
  TEST_ASSERT_EQUAL(46, info.m_length);
}

void test_calculateRegionInfo_presetBank() {
  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kPresetBank, 2);

  TEST_ASSERT_EQUAL(666, info.m_address);
  TEST_ASSERT_EQUAL(185, info.m_length);
}

// =============================================================================
// Serialize: Bypass
// =============================================================================

void test_serialize_bypass_active() {
  LogicalState state;
  state.m_bypassState = BypassState::kActive;
  MemoryHandler handler;

  uint8_t buffer[1];
  handler.serializeRegion(MemoryRegion::kBypass, state, buffer);

  TEST_ASSERT_EQUAL(1, buffer[0]);
}

void test_serialize_bypass_bypassed() {
  LogicalState state;
  state.m_bypassState = BypassState::kBypassed;
  MemoryHandler handler;

  uint8_t buffer[1];
  handler.serializeRegion(MemoryRegion::kBypass, state, buffer);

  TEST_ASSERT_EQUAL(0, buffer[0]);
}

// =============================================================================
// Serialize: ProgramMode
// =============================================================================

void test_serialize_programMode_program() {
  LogicalState state;
  state.m_programMode = ProgramMode::kProgram;
  MemoryHandler handler;

  uint8_t buffer[1];
  handler.serializeRegion(MemoryRegion::kProgramMode, state, buffer);

  TEST_ASSERT_EQUAL(0, buffer[0]);
}

void test_serialize_programMode_preset() {
  LogicalState state;
  state.m_programMode = ProgramMode::kPreset;
  MemoryHandler handler;

  uint8_t buffer[1];
  handler.serializeRegion(MemoryRegion::kProgramMode, state, buffer);

  TEST_ASSERT_EQUAL(1, buffer[0]);
}

// =============================================================================
// Serialize: CurrentProgram, CurrentPreset, CurrentPresetBank, MidiChannel
// =============================================================================

void test_serialize_currentProgram() {
  LogicalState state;
  state.m_currentProgram = 5;
  MemoryHandler handler;

  uint8_t buffer[1];
  handler.serializeRegion(MemoryRegion::kCurrentProgram, state, buffer);

  TEST_ASSERT_EQUAL(5, buffer[0]);
}

void test_serialize_currentPreset() {
  LogicalState state;
  state.m_currentPreset = 3;
  MemoryHandler handler;

  uint8_t buffer[1];
  handler.serializeRegion(MemoryRegion::kCurrentPreset, state, buffer);

  TEST_ASSERT_EQUAL(3, buffer[0]);
}

void test_serialize_currentPresetBank() {
  LogicalState state;
  state.m_currentPresetBank = 4;
  MemoryHandler handler;

  uint8_t buffer[1];
  handler.serializeRegion(MemoryRegion::kCurrentPresetBank, state, buffer);

  TEST_ASSERT_EQUAL(4, buffer[0]);
}

void test_serialize_midiChannel() {
  LogicalState state;
  state.m_midiChannel = 7;
  MemoryHandler handler;

  uint8_t buffer[1];
  handler.serializeRegion(MemoryRegion::kMidiChannel, state, buffer);

  TEST_ASSERT_EQUAL(7, buffer[0]);
}

// =============================================================================
// Serialize: DeviceState
// =============================================================================

void test_serialize_deviceState() {
  LogicalState state;
  state.m_bypassState = BypassState::kBypassed;
  state.m_programMode = ProgramMode::kPreset;
  state.m_currentProgram = 3;
  state.m_currentPreset = 2;
  state.m_currentPresetBank = 1;
  state.m_midiChannel = 5;
  MemoryHandler handler;

  uint8_t buffer[6];
  handler.serializeRegion(MemoryRegion::kDeviceState, state, buffer);

  TEST_ASSERT_EQUAL(0, buffer[0]);  // kBypassed
  TEST_ASSERT_EQUAL(1, buffer[1]);  // kPreset
  TEST_ASSERT_EQUAL(3, buffer[2]);  // currentProgram
  TEST_ASSERT_EQUAL(2, buffer[3]);  // currentPreset
  TEST_ASSERT_EQUAL(1, buffer[4]);  // currentPresetBank
  TEST_ASSERT_EQUAL(5, buffer[5]);  // midiChannel
}

// =============================================================================
// Serialize: Tap
// =============================================================================

void test_serialize_tap_disabled() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7];
  handler.serializeRegion(MemoryRegion::kTap, state, buffer);

  TEST_ASSERT_EQUAL(0, buffer[0]);  // tapState disabled
  TEST_ASSERT_EQUAL(0, buffer[1]);  // divState disabled
  TEST_ASSERT_EQUAL(0, buffer[2]);  // divValue quarter
  TEST_ASSERT_EQUAL(0, buffer[3]);  // interval low
  TEST_ASSERT_EQUAL(0, buffer[4]);  // interval high
  TEST_ASSERT_EQUAL(0, buffer[5]);  // divInterval low
  TEST_ASSERT_EQUAL(0, buffer[6]);  // divInterval high
}

void test_serialize_tap_enabled_with_values() {
  LogicalState state = makeLogicalStateWithTap(
    TapState::kEnabled, DivState::kEnabled, DivValue::kEight, 1024, 512
  );
  MemoryHandler handler;

  uint8_t buffer[7];
  handler.serializeRegion(MemoryRegion::kTap, state, buffer);

  TEST_ASSERT_EQUAL(1, buffer[0]);  // tapState enabled
  TEST_ASSERT_EQUAL(1, buffer[1]);  // divState enabled
  TEST_ASSERT_EQUAL(1, buffer[2]);  // divValue eighth

  uint16_t interval = 0;
  Utils::unpack16(buffer[3], buffer[4], interval);
  TEST_ASSERT_EQUAL(1024, interval);

  uint16_t divInterval = 0;
  Utils::unpack16(buffer[5], buffer[6], divInterval);
  TEST_ASSERT_EQUAL(512, divInterval);
}

// =============================================================================
// Serialize: Tempo
// =============================================================================

void test_serialize_tempo_zero() {
  LogicalState state;
  state.m_tempo = 0;
  MemoryHandler handler;

  uint8_t buffer[2];
  handler.serializeRegion(MemoryRegion::kTempo, state, buffer);

  TEST_ASSERT_EQUAL(0, buffer[0]);
  TEST_ASSERT_EQUAL(0, buffer[1]);
}

void test_serialize_tempo_value() {
  LogicalState state;
  state.m_tempo = 568;
  MemoryHandler handler;

  uint8_t buffer[2];
  handler.serializeRegion(MemoryRegion::kTempo, state, buffer);

  uint16_t tempo = 0;
  Utils::unpack16(buffer[0], buffer[1], tempo);
  TEST_ASSERT_EQUAL(568, tempo);
}

// =============================================================================
// Serialize: Expr
// =============================================================================

void test_serialize_expr_default() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7];
  handler.serializeRegion(MemoryRegion::kExpr, state, buffer, 0);

  TEST_ASSERT_EQUAL(0, buffer[0]);  // state inactive
  TEST_ASSERT_EQUAL(0, buffer[1]);  // mappedPot pot0
  TEST_ASSERT_EQUAL(0, buffer[2]);  // direction normal
  TEST_ASSERT_EQUAL(0, buffer[3]);  // heel low
  TEST_ASSERT_EQUAL(0, buffer[4]);  // heel high

  uint16_t toe = 0;
  Utils::unpack16(buffer[5], buffer[6], toe);
  TEST_ASSERT_EQUAL(1023, toe);  // default toe value
}

void test_serialize_expr_with_values() {
  LogicalState state = makeLogicalStateWithExpr(
    2, ExprState::kActive, MappedPot::kPot1, Direction::kInverted, 600, 800
  );
  MemoryHandler handler;

  uint8_t buffer[7];
  handler.serializeRegion(MemoryRegion::kExpr, state, buffer, 2);

  TEST_ASSERT_EQUAL(1, buffer[0]);  // state active
  TEST_ASSERT_EQUAL(1, buffer[1]);  // mappedPot pot1
  TEST_ASSERT_EQUAL(1, buffer[2]);  // direction inverted

  uint16_t heel = 0;
  Utils::unpack16(buffer[3], buffer[4], heel);
  TEST_ASSERT_EQUAL(600, heel);

  uint16_t toe = 0;
  Utils::unpack16(buffer[5], buffer[6], toe);
  TEST_ASSERT_EQUAL(800, toe);
}

void test_serialize_expr_program_index_0() {
  LogicalState state;
  state.m_exprParams[0].m_state = ExprState::kActive;
  state.m_exprParams[0].m_mappedPot = MappedPot::kMixPot;
  MemoryHandler handler;

  uint8_t buffer[7];
  handler.serializeRegion(MemoryRegion::kExpr, state, buffer, 0);

  TEST_ASSERT_EQUAL(1, buffer[0]);  // active
  TEST_ASSERT_EQUAL(3, buffer[1]);  // mixPot
}

void test_serialize_expr_program_index_7() {
  LogicalState state;
  state.m_exprParams[7].m_state = ExprState::kActive;
  state.m_exprParams[7].m_mappedPot = MappedPot::kPot2;
  MemoryHandler handler;

  uint8_t buffer[7];
  handler.serializeRegion(MemoryRegion::kExpr, state, buffer, 7);

  TEST_ASSERT_EQUAL(1, buffer[0]);  // active
  TEST_ASSERT_EQUAL(2, buffer[1]);  // pot2
}

// =============================================================================
// Serialize: Pot
// =============================================================================

void test_serialize_pot_default() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7];
  handler.serializeRegion(MemoryRegion::kPot, state, buffer, 0, 0);

  TEST_ASSERT_EQUAL(1, buffer[0]);  // state active (default)
  TEST_ASSERT_EQUAL(0, buffer[1]);  // value low
  TEST_ASSERT_EQUAL(0, buffer[2]);  // value high
  TEST_ASSERT_EQUAL(0, buffer[3]);  // min low
  TEST_ASSERT_EQUAL(0, buffer[4]);  // min high
  TEST_ASSERT_EQUAL(255, buffer[5]); // max low (1023 & 0xFF)
  TEST_ASSERT_EQUAL(3, buffer[6]);   // max high (1023 >> 8)
}

void test_serialize_pot_with_values() {
  LogicalState state;
  state.m_potParams[3][1].m_state = PotState::kDisabled;
  state.m_potParams[3][1].m_value = 512;
  state.m_potParams[3][1].m_minValue = 128;
  state.m_potParams[3][1].m_maxValue = 768;
  MemoryHandler handler;

  uint8_t buffer[7];
  handler.serializeRegion(MemoryRegion::kPot, state, buffer, 3, 1);

  TEST_ASSERT_EQUAL(0, buffer[0]);  // state disabled

  uint16_t value = 0;
  Utils::unpack16(buffer[1], buffer[2], value);
  TEST_ASSERT_EQUAL(512, value);

  uint16_t minValue = 0;
  Utils::unpack16(buffer[3], buffer[4], minValue);
  TEST_ASSERT_EQUAL(128, minValue);

  uint16_t maxValue = 0;
  Utils::unpack16(buffer[5], buffer[6], maxValue);
  TEST_ASSERT_EQUAL(768, maxValue);
}

// =============================================================================
// Deserialize: Bypass
// =============================================================================

void test_deserialize_bypass_active() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {1};
  handler.deserializeRegion(MemoryRegion::kBypass, state, buffer);

  TEST_ASSERT_EQUAL(BypassState::kActive, state.m_bypassState);
}

void test_deserialize_bypass_bypassed() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {0};
  handler.deserializeRegion(MemoryRegion::kBypass, state, buffer);

  TEST_ASSERT_EQUAL(BypassState::kBypassed, state.m_bypassState);
}

void test_deserialize_bypass_invalid_defaults_to_active() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {255};
  handler.deserializeRegion(MemoryRegion::kBypass, state, buffer);

  TEST_ASSERT_EQUAL(BypassState::kActive, state.m_bypassState);
}

// =============================================================================
// Deserialize: ProgramMode
// =============================================================================

void test_deserialize_programMode_program() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {0};
  handler.deserializeRegion(MemoryRegion::kProgramMode, state, buffer);

  TEST_ASSERT_EQUAL(ProgramMode::kProgram, state.m_programMode);
}

void test_deserialize_programMode_preset() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {1};
  handler.deserializeRegion(MemoryRegion::kProgramMode, state, buffer);

  TEST_ASSERT_EQUAL(ProgramMode::kPreset, state.m_programMode);
}

void test_deserialize_programMode_invalid_defaults_to_program() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {99};
  handler.deserializeRegion(MemoryRegion::kProgramMode, state, buffer);

  TEST_ASSERT_EQUAL(ProgramMode::kProgram, state.m_programMode);
}

// =============================================================================
// Deserialize: CurrentProgram, CurrentPreset, CurrentPresetBank, MidiChannel
// =============================================================================

void test_deserialize_currentProgram() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {5};
  handler.deserializeRegion(MemoryRegion::kCurrentProgram, state, buffer);

  TEST_ASSERT_EQUAL(5, state.m_currentProgram);
}

void test_deserialize_currentProgram_clamps_above_max() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {255};
  handler.deserializeRegion(MemoryRegion::kCurrentProgram, state, buffer);

  TEST_ASSERT_EQUAL(ProgramConstants::c_maxPrograms, state.m_currentProgram);
}

void test_deserialize_currentPreset() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {3};
  handler.deserializeRegion(MemoryRegion::kCurrentPreset, state, buffer);

  TEST_ASSERT_EQUAL(3, state.m_currentPreset);
}

void test_deserialize_currentPreset_clamps_above_max() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {255};
  handler.deserializeRegion(MemoryRegion::kCurrentPreset, state, buffer);

  TEST_ASSERT_EQUAL(PresetConstants::c_maxPreset, state.m_currentPreset);
}

void test_deserialize_currentPresetBank() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {2};
  handler.deserializeRegion(MemoryRegion::kCurrentPresetBank, state, buffer);

  TEST_ASSERT_EQUAL(2, state.m_currentPresetBank);
}

void test_deserialize_currentPresetBank_clamps_above_max() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {255};
  handler.deserializeRegion(MemoryRegion::kCurrentPresetBank, state, buffer);

  TEST_ASSERT_EQUAL(PresetConstants::c_presetBankCount, state.m_currentPresetBank);
}

void test_deserialize_midiChannel() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {5};
  handler.deserializeRegion(MemoryRegion::kMidiChannel, state, buffer);

  TEST_ASSERT_EQUAL(5, state.m_midiChannel);
}

void test_deserialize_midiChannel_clamps_above_7() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[1] = {255};
  handler.deserializeRegion(MemoryRegion::kMidiChannel, state, buffer);

  TEST_ASSERT_EQUAL(7, state.m_midiChannel);
}

// =============================================================================
// Deserialize: DeviceState
// =============================================================================

void test_deserialize_deviceState() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[6] = {0, 1, 3, 2, 1, 5};
  handler.deserializeRegion(MemoryRegion::kDeviceState, state, buffer);

  TEST_ASSERT_EQUAL(BypassState::kBypassed, state.m_bypassState);
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, state.m_programMode);
  TEST_ASSERT_EQUAL(3, state.m_currentProgram);
  TEST_ASSERT_EQUAL(2, state.m_currentPreset);
  TEST_ASSERT_EQUAL(1, state.m_currentPresetBank);
  TEST_ASSERT_EQUAL(5, state.m_midiChannel);
}

// =============================================================================
// Deserialize: Tap
// =============================================================================

void test_deserialize_tap_disabled() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7] = {0, 0, 0, 0, 0, 0, 0};
  handler.deserializeRegion(MemoryRegion::kTap, state, buffer);

  TEST_ASSERT_EQUAL(TapState::kDisabled, state.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kDisabled, state.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kQuarter, state.m_divValue);
  TEST_ASSERT_EQUAL(0, state.m_interval);
  TEST_ASSERT_EQUAL(0, state.m_divInterval);
}

void test_deserialize_tap_enabled_with_values() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7] = {1, 1, 1, 0, 2, 0, 1};  // interval=512, divInterval=256
  handler.deserializeRegion(MemoryRegion::kTap, state, buffer);

  TEST_ASSERT_EQUAL(TapState::kEnabled, state.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, state.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, state.m_divValue);
  TEST_ASSERT_EQUAL(512, state.m_interval);
  TEST_ASSERT_EQUAL(256, state.m_divInterval);
}

void test_deserialize_tap_clamps_interval_above_1000() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7] = {1, 0, 0, 0xFF, 0xFF, 0, 0};  // interval=65535
  handler.deserializeRegion(MemoryRegion::kTap, state, buffer);

  TEST_ASSERT_EQUAL(1000, state.m_interval);
}

void test_deserialize_tap_invalid_divValue_defaults() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7] = {0, 0, 99, 0, 0, 0, 0};  // invalid divValue
  handler.deserializeRegion(MemoryRegion::kTap, state, buffer);

  TEST_ASSERT_EQUAL(DivValue::kQuarter, state.m_divValue);
}

// =============================================================================
// Deserialize: Tempo
// =============================================================================

void test_deserialize_tempo() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[2] = {0, 2};  // tempo=512
  handler.deserializeRegion(MemoryRegion::kTempo, state, buffer);

  TEST_ASSERT_EQUAL(512, state.m_tempo);
}

void test_deserialize_tempo_clamps_above_1000() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[2] = {0xFF, 0xFF};  // tempo=65535
  handler.deserializeRegion(MemoryRegion::kTempo, state, buffer);

  TEST_ASSERT_EQUAL(1000, state.m_tempo);
}

// =============================================================================
// Deserialize: Expr
// =============================================================================

void test_deserialize_expr() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7] = {1, 2, 1, 0, 1, 0, 2};  // heel=256, toe=512
  handler.deserializeRegion(MemoryRegion::kExpr, state, buffer, 2);

  TEST_ASSERT_EQUAL(ExprState::kActive, state.m_exprParams[2].m_state);
  TEST_ASSERT_EQUAL(MappedPot::kPot2, state.m_exprParams[2].m_mappedPot);
  TEST_ASSERT_EQUAL(Direction::kInverted, state.m_exprParams[2].m_direction);
  TEST_ASSERT_EQUAL(256, state.m_exprParams[2].m_heelValue);
  TEST_ASSERT_EQUAL(512, state.m_exprParams[2].m_toeValue);
}

void test_deserialize_expr_clamps_values_above_1023() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7] = {1, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF};  // heel=65535, toe=65535
  handler.deserializeRegion(MemoryRegion::kExpr, state, buffer, 0);

  TEST_ASSERT_EQUAL(1023, state.m_exprParams[0].m_heelValue);
  TEST_ASSERT_EQUAL(1023, state.m_exprParams[0].m_toeValue);
}

void test_deserialize_expr_invalid_mappedPot_defaults() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7] = {0, 99, 0, 0, 0, 0, 0};  // invalid mappedPot
  handler.deserializeRegion(MemoryRegion::kExpr, state, buffer, 0);

  TEST_ASSERT_EQUAL(MappedPot::kPot0, state.m_exprParams[0].m_mappedPot);
}

void test_deserialize_expr_different_program_indices_are_independent() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer1[7] = {1, 1, 0, 0, 0, 0, 0};  // Active, Pot1
  uint8_t buffer2[7] = {0, 2, 1, 0, 0, 0, 0};  // Inactive, Pot2

  handler.deserializeRegion(MemoryRegion::kExpr, state, buffer1, 0);
  handler.deserializeRegion(MemoryRegion::kExpr, state, buffer2, 1);

  TEST_ASSERT_EQUAL(ExprState::kActive, state.m_exprParams[0].m_state);
  TEST_ASSERT_EQUAL(MappedPot::kPot1, state.m_exprParams[0].m_mappedPot);
  TEST_ASSERT_EQUAL(ExprState::kInactive, state.m_exprParams[1].m_state);
  TEST_ASSERT_EQUAL(MappedPot::kPot2, state.m_exprParams[1].m_mappedPot);
}

// =============================================================================
// Deserialize: Pot
// =============================================================================

void test_deserialize_pot() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7] = {1, 0, 2, 128, 0, 0, 3};  // value=512, min=128, max=768
  handler.deserializeRegion(MemoryRegion::kPot, state, buffer, 3, 2);

  TEST_ASSERT_EQUAL(PotState::kActive, state.m_potParams[3][2].m_state);
  TEST_ASSERT_EQUAL(512, state.m_potParams[3][2].m_value);
  TEST_ASSERT_EQUAL(128, state.m_potParams[3][2].m_minValue);
  TEST_ASSERT_EQUAL(768, state.m_potParams[3][2].m_maxValue);
}

void test_deserialize_pot_clamps_values_above_1023() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7] = {1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  handler.deserializeRegion(MemoryRegion::kPot, state, buffer, 0, 0);

  TEST_ASSERT_EQUAL(1023, state.m_potParams[0][0].m_value);
  TEST_ASSERT_EQUAL(1023, state.m_potParams[0][0].m_minValue);
  TEST_ASSERT_EQUAL(1023, state.m_potParams[0][0].m_maxValue);
}

void test_deserialize_pot_invalid_state_defaults_to_active() {
  LogicalState state;
  MemoryHandler handler;

  uint8_t buffer[7] = {99, 0, 0, 0, 0, 0, 0};  // invalid state
  handler.deserializeRegion(MemoryRegion::kPot, state, buffer, 0, 0);

  TEST_ASSERT_EQUAL(PotState::kActive, state.m_potParams[0][0].m_state);
}

// =============================================================================
// Serialize/Deserialize: LogicalState (full roundtrip)
// =============================================================================

void test_serialize_logicalState() {
  LogicalState state;
  state.m_currentProgram = 2;
  state.m_currentPreset = 3;
  state.m_currentPresetBank = 4;
  state.m_midiChannel = 7;
  state.m_tapState = TapState::kEnabled;
  state.m_divState = DivState::kEnabled;
  state.m_divValue = DivValue::kEight;
  state.m_interval = 512;
  state.m_divInterval = 256;
  state.m_tempo = 256;
  state.m_exprParams[1].m_state = ExprState::kActive;
  state.m_exprParams[1].m_mappedPot = MappedPot::kPot2;
  state.m_potParams[1][1].m_state = PotState::kActive;
  state.m_potParams[1][1].m_value = 512;
  MemoryHandler handler;

  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kLogicalState);
  uint8_t buffer[info.m_length];
  handler.serializeRegion(MemoryRegion::kLogicalState, state, buffer);

  TEST_ASSERT_EQUAL(2, buffer[MemoryLayout::c_currentProgram]);
  TEST_ASSERT_EQUAL(3, buffer[MemoryLayout::c_currentPreset]);
  TEST_ASSERT_EQUAL(4, buffer[MemoryLayout::c_currentPresetBank]);
  TEST_ASSERT_EQUAL(7, buffer[MemoryLayout::c_midiChannel]);
  TEST_ASSERT_EQUAL(1, buffer[MemoryLayout::c_tapState]);
  TEST_ASSERT_EQUAL(1, buffer[MemoryLayout::c_divState]);
  TEST_ASSERT_EQUAL(1, buffer[MemoryLayout::c_divValue]);
}

void test_deserialize_logicalState() {
  LogicalState state;
  MemoryHandler handler;

  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kLogicalState);
  uint8_t buffer[info.m_length];
  memset(buffer, 0, info.m_length);

  buffer[MemoryLayout::c_bypassState] = 0;
  buffer[MemoryLayout::c_programMode] = 1;
  buffer[MemoryLayout::c_currentProgram] = 2;
  buffer[MemoryLayout::c_currentPreset] = 3;
  buffer[MemoryLayout::c_midiChannel] = 5;
  buffer[MemoryLayout::c_tapState] = 1;
  buffer[MemoryLayout::c_divState] = 1;
  buffer[MemoryLayout::c_divValue] = 2;
  buffer[MemoryLayout::c_intervalL] = 0;
  buffer[MemoryLayout::c_intervalH] = 2;

  handler.deserializeRegion(MemoryRegion::kLogicalState, state, buffer);

  TEST_ASSERT_EQUAL(BypassState::kBypassed, state.m_bypassState);
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, state.m_programMode);
  TEST_ASSERT_EQUAL(2, state.m_currentProgram);
  TEST_ASSERT_EQUAL(3, state.m_currentPreset);
  TEST_ASSERT_EQUAL(5, state.m_midiChannel);
  TEST_ASSERT_EQUAL(TapState::kEnabled, state.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, state.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kSixteenth, state.m_divValue);
  TEST_ASSERT_EQUAL(512, state.m_interval);
}

void test_logicalState_roundtrip_preserves_data() {
  LogicalState original;
  original.m_bypassState = BypassState::kBypassed;
  original.m_programMode = ProgramMode::kPreset;
  original.m_currentProgram = 5;
  original.m_currentPreset = 2;
  original.m_currentPresetBank = 3;
  original.m_midiChannel = 6;
  original.m_tapState = TapState::kEnabled;
  original.m_divState = DivState::kEnabled;
  original.m_divValue = DivValue::kDottedEight;
  original.m_interval = 750;
  original.m_divInterval = 375;
  original.m_tempo = 500;
  original.m_exprParams[3].m_state = ExprState::kActive;
  original.m_exprParams[3].m_mappedPot = MappedPot::kMixPot;
  original.m_exprParams[3].m_direction = Direction::kInverted;
  original.m_exprParams[3].m_heelValue = 100;
  original.m_exprParams[3].m_toeValue = 900;
  original.m_potParams[2][1].m_state = PotState::kDisabled;
  original.m_potParams[2][1].m_value = 400;
  original.m_potParams[2][1].m_minValue = 50;
  original.m_potParams[2][1].m_maxValue = 800;

  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kLogicalState);
  uint8_t buffer[info.m_length];

  handler.serializeRegion(MemoryRegion::kLogicalState, original, buffer);

  LogicalState restored;
  handler.deserializeRegion(MemoryRegion::kLogicalState, restored, buffer);

  TEST_ASSERT_EQUAL(original.m_bypassState, restored.m_bypassState);
  TEST_ASSERT_EQUAL(original.m_programMode, restored.m_programMode);
  TEST_ASSERT_EQUAL(original.m_currentProgram, restored.m_currentProgram);
  TEST_ASSERT_EQUAL(original.m_currentPreset, restored.m_currentPreset);
  TEST_ASSERT_EQUAL(original.m_currentPresetBank, restored.m_currentPresetBank);
  TEST_ASSERT_EQUAL(original.m_midiChannel, restored.m_midiChannel);
  TEST_ASSERT_EQUAL(original.m_tapState, restored.m_tapState);
  TEST_ASSERT_EQUAL(original.m_divState, restored.m_divState);
  TEST_ASSERT_EQUAL(original.m_divValue, restored.m_divValue);
  TEST_ASSERT_EQUAL(original.m_interval, restored.m_interval);
  TEST_ASSERT_EQUAL(original.m_divInterval, restored.m_divInterval);
  TEST_ASSERT_EQUAL(original.m_tempo, restored.m_tempo);
  TEST_ASSERT_EQUAL(original.m_exprParams[3].m_state, restored.m_exprParams[3].m_state);
  TEST_ASSERT_EQUAL(original.m_exprParams[3].m_mappedPot, restored.m_exprParams[3].m_mappedPot);
  TEST_ASSERT_EQUAL(original.m_exprParams[3].m_direction, restored.m_exprParams[3].m_direction);
  TEST_ASSERT_EQUAL(original.m_exprParams[3].m_heelValue, restored.m_exprParams[3].m_heelValue);
  TEST_ASSERT_EQUAL(original.m_exprParams[3].m_toeValue, restored.m_exprParams[3].m_toeValue);
  TEST_ASSERT_EQUAL(original.m_potParams[2][1].m_state, restored.m_potParams[2][1].m_state);
  TEST_ASSERT_EQUAL(original.m_potParams[2][1].m_value, restored.m_potParams[2][1].m_value);
  TEST_ASSERT_EQUAL(original.m_potParams[2][1].m_minValue, restored.m_potParams[2][1].m_minValue);
  TEST_ASSERT_EQUAL(original.m_potParams[2][1].m_maxValue, restored.m_potParams[2][1].m_maxValue);
}

// =============================================================================
// Validators
// =============================================================================

void test_validator_bypassState() {
  TEST_ASSERT_EQUAL(BypassState::kBypassed, LogicalStateValidator::setSafeBypassState(0));
  TEST_ASSERT_EQUAL(BypassState::kActive, LogicalStateValidator::setSafeBypassState(1));
  TEST_ASSERT_EQUAL(BypassState::kActive, LogicalStateValidator::setSafeBypassState(2));
  TEST_ASSERT_EQUAL(BypassState::kActive, LogicalStateValidator::setSafeBypassState(255));
}

void test_validator_programMode() {
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, LogicalStateValidator::setSafeProgramMode(0));
  TEST_ASSERT_EQUAL(ProgramMode::kPreset, LogicalStateValidator::setSafeProgramMode(1));
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, LogicalStateValidator::setSafeProgramMode(2));
  TEST_ASSERT_EQUAL(ProgramMode::kProgram, LogicalStateValidator::setSafeProgramMode(255));
}

void test_validator_exprState() {
  TEST_ASSERT_EQUAL(ExprState::kInactive, LogicalStateValidator::setSafeExprState(0));
  TEST_ASSERT_EQUAL(ExprState::kActive, LogicalStateValidator::setSafeExprState(1));
  TEST_ASSERT_EQUAL(ExprState::kInactive, LogicalStateValidator::setSafeExprState(2));
}

void test_validator_mappedPot() {
  TEST_ASSERT_EQUAL(MappedPot::kPot0, LogicalStateValidator::setSafeMappedPot(0));
  TEST_ASSERT_EQUAL(MappedPot::kPot1, LogicalStateValidator::setSafeMappedPot(1));
  TEST_ASSERT_EQUAL(MappedPot::kPot2, LogicalStateValidator::setSafeMappedPot(2));
  TEST_ASSERT_EQUAL(MappedPot::kMixPot, LogicalStateValidator::setSafeMappedPot(3));
  TEST_ASSERT_EQUAL(MappedPot::kPot0, LogicalStateValidator::setSafeMappedPot(4));
}

void test_validator_direction() {
  TEST_ASSERT_EQUAL(Direction::kNormal, LogicalStateValidator::setSafeDirection(0));
  TEST_ASSERT_EQUAL(Direction::kInverted, LogicalStateValidator::setSafeDirection(1));
  TEST_ASSERT_EQUAL(Direction::kNormal, LogicalStateValidator::setSafeDirection(2));
}

void test_validator_potState() {
  TEST_ASSERT_EQUAL(PotState::kDisabled, LogicalStateValidator::setSafePotState(0));
  TEST_ASSERT_EQUAL(PotState::kActive, LogicalStateValidator::setSafePotState(1));
  TEST_ASSERT_EQUAL(PotState::kActive, LogicalStateValidator::setSafePotState(2));
}

void test_validator_tapState() {
  TEST_ASSERT_EQUAL(TapState::kDisabled, LogicalStateValidator::setSafeTapState(0));
  TEST_ASSERT_EQUAL(TapState::kEnabled, LogicalStateValidator::setSafeTapState(1));
  TEST_ASSERT_EQUAL(TapState::kDisabled, LogicalStateValidator::setSafeTapState(2));
}

void test_validator_divState() {
  TEST_ASSERT_EQUAL(DivState::kDisabled, LogicalStateValidator::setSafeDivState(0));
  TEST_ASSERT_EQUAL(DivState::kEnabled, LogicalStateValidator::setSafeDivState(1));
  TEST_ASSERT_EQUAL(DivState::kDisabled, LogicalStateValidator::setSafeDivState(2));
}

void test_validator_divValue() {
  TEST_ASSERT_EQUAL(DivValue::kQuarter, LogicalStateValidator::setSafeDivValue(0));
  TEST_ASSERT_EQUAL(DivValue::kEight, LogicalStateValidator::setSafeDivValue(1));
  TEST_ASSERT_EQUAL(DivValue::kSixteenth, LogicalStateValidator::setSafeDivValue(2));
  TEST_ASSERT_EQUAL(DivValue::kDottedEight, LogicalStateValidator::setSafeDivValue(3));
  TEST_ASSERT_EQUAL(DivValue::kEightTriplet, LogicalStateValidator::setSafeDivValue(4));
  TEST_ASSERT_EQUAL(DivValue::kQuarter, LogicalStateValidator::setSafeDivValue(5));
}

// =============================================================================
// Preset Serialization
// =============================================================================

void test_serialize_preset() {
  Preset preset;
  preset.m_programIndex = 2;
  preset.m_tapState = TapState::kEnabled;
  preset.m_divState = DivState::kEnabled;
  preset.m_divValue = DivValue::kEight;
  preset.m_interval = 512;
  preset.m_divInterval = 256;
  preset.m_tempo = 256;
  preset.m_exprState = ExprState::kActive;
  preset.m_mappedPot = MappedPot::kPot0;
  preset.m_direction = Direction::kInverted;
  preset.m_heelValue = 256;
  preset.m_toeValue = 512;
  preset.m_potParams[3].m_state = PotState::kActive;
  preset.m_potParams[3].m_value = 512;
  preset.m_potParams[3].m_minValue = 256;
  preset.m_potParams[3].m_maxValue = 768;

  MemoryHandler handler;
  uint8_t buffer[PresetLayout::c_presetSize];
  handler.serializePreset(preset, buffer, 0, 1, 0);

  TEST_ASSERT_EQUAL(2, buffer[PresetLayout::c_programIndex]);
  TEST_ASSERT_EQUAL(1, buffer[PresetLayout::c_tapState]);
  TEST_ASSERT_EQUAL(1, buffer[PresetLayout::c_divState]);
  TEST_ASSERT_EQUAL(1, buffer[PresetLayout::c_divValue]);

  uint16_t interval = 0;
  Utils::unpack16(buffer[PresetLayout::c_intervalL], buffer[PresetLayout::c_intervalH], interval);
  TEST_ASSERT_EQUAL(512, interval);

  TEST_ASSERT_EQUAL(1, buffer[PresetLayout::c_exprState]);
  TEST_ASSERT_EQUAL(0, buffer[PresetLayout::c_mappedPot]);
  TEST_ASSERT_EQUAL(1, buffer[PresetLayout::c_direction]);

  uint8_t potOffset = PresetLayout::c_potParamStart + PresetLayout::c_potParamSize * 3;
  TEST_ASSERT_EQUAL(1, buffer[potOffset]);  // state
}

void test_deserialize_preset() {
  MemoryHandler handler;
  Preset preset;

  uint8_t buffer[PresetLayout::c_presetSize];
  memset(buffer, 0, PresetLayout::c_presetSize);

  buffer[PresetLayout::c_id] = 2;
  buffer[PresetLayout::c_programIndex] = 3;
  buffer[PresetLayout::c_tapState] = 1;
  buffer[PresetLayout::c_divState] = 1;
  buffer[PresetLayout::c_divValue] = 1;
  buffer[PresetLayout::c_intervalL] = 0;
  buffer[PresetLayout::c_intervalH] = 2;
  buffer[PresetLayout::c_divIntervalL] = 0;
  buffer[PresetLayout::c_divIntervalH] = 1;
  buffer[PresetLayout::c_tempoL] = 0;
  buffer[PresetLayout::c_tempoH] = 1;
  buffer[PresetLayout::c_exprState] = 1;
  buffer[PresetLayout::c_mappedPot] = 1;
  buffer[PresetLayout::c_direction] = 0;
  buffer[PresetLayout::c_heelValueL] = 0;
  buffer[PresetLayout::c_heelValueH] = 1;
  buffer[PresetLayout::c_toeValueL] = 0;
  buffer[PresetLayout::c_toeValueH] = 2;

  uint8_t potOffset = PresetLayout::c_potParamStart + PresetLayout::c_potParamSize * 3;
  buffer[potOffset] = 1;
  buffer[potOffset + 1] = 0;
  buffer[potOffset + 2] = 2;
  buffer[potOffset + 3] = 0;
  buffer[potOffset + 4] = 1;
  buffer[potOffset + 5] = 0;
  buffer[potOffset + 6] = 3;

  handler.deserializePreset(preset, buffer, 0, 2, 0);

  TEST_ASSERT_EQUAL(2, preset.m_id);
  TEST_ASSERT_EQUAL(3, preset.m_programIndex);
  TEST_ASSERT_EQUAL(TapState::kEnabled, preset.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, preset.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kEight, preset.m_divValue);
  TEST_ASSERT_EQUAL(512, preset.m_interval);
  TEST_ASSERT_EQUAL(256, preset.m_divInterval);
  TEST_ASSERT_EQUAL(256, preset.m_tempo);
  TEST_ASSERT_EQUAL(ExprState::kActive, preset.m_exprState);
  TEST_ASSERT_EQUAL(MappedPot::kPot1, preset.m_mappedPot);
  TEST_ASSERT_EQUAL(Direction::kNormal, preset.m_direction);
  TEST_ASSERT_EQUAL(256, preset.m_heelValue);
  TEST_ASSERT_EQUAL(512, preset.m_toeValue);
  TEST_ASSERT_EQUAL(PotState::kActive, preset.m_potParams[3].m_state);
  TEST_ASSERT_EQUAL(512, preset.m_potParams[3].m_value);
  TEST_ASSERT_EQUAL(256, preset.m_potParams[3].m_minValue);
  TEST_ASSERT_EQUAL(768, preset.m_potParams[3].m_maxValue);
}

void test_preset_roundtrip_preserves_data() {
  Preset original;
  original.m_programIndex = 5;
  original.m_tapState = TapState::kEnabled;
  original.m_divState = DivState::kEnabled;
  original.m_divValue = DivValue::kSixteenth;
  original.m_interval = 400;
  original.m_divInterval = 100;
  original.m_tempo = 600;
  original.m_exprState = ExprState::kActive;
  original.m_mappedPot = MappedPot::kPot2;
  original.m_direction = Direction::kInverted;
  original.m_heelValue = 50;
  original.m_toeValue = 950;
  original.m_potParams[0].m_state = PotState::kDisabled;
  original.m_potParams[0].m_value = 300;
  original.m_potParams[0].m_minValue = 100;
  original.m_potParams[0].m_maxValue = 600;

  MemoryHandler handler;
  uint8_t buffer[PresetLayout::c_presetSize];

  handler.serializePreset(original, buffer, 1, 2, 0);

  Preset restored;
  handler.deserializePreset(restored, buffer, 1, 2, 0);

  TEST_ASSERT_EQUAL(original.m_programIndex, restored.m_programIndex);
  TEST_ASSERT_EQUAL(original.m_tapState, restored.m_tapState);
  TEST_ASSERT_EQUAL(original.m_divState, restored.m_divState);
  TEST_ASSERT_EQUAL(original.m_divValue, restored.m_divValue);
  TEST_ASSERT_EQUAL(original.m_interval, restored.m_interval);
  TEST_ASSERT_EQUAL(original.m_divInterval, restored.m_divInterval);
  TEST_ASSERT_EQUAL(original.m_tempo, restored.m_tempo);
  TEST_ASSERT_EQUAL(original.m_exprState, restored.m_exprState);
  TEST_ASSERT_EQUAL(original.m_mappedPot, restored.m_mappedPot);
  TEST_ASSERT_EQUAL(original.m_direction, restored.m_direction);
  TEST_ASSERT_EQUAL(original.m_heelValue, restored.m_heelValue);
  TEST_ASSERT_EQUAL(original.m_toeValue, restored.m_toeValue);
  TEST_ASSERT_EQUAL(original.m_potParams[0].m_state, restored.m_potParams[0].m_state);
  TEST_ASSERT_EQUAL(original.m_potParams[0].m_value, restored.m_potParams[0].m_value);
  TEST_ASSERT_EQUAL(original.m_potParams[0].m_minValue, restored.m_potParams[0].m_minValue);
  TEST_ASSERT_EQUAL(original.m_potParams[0].m_maxValue, restored.m_potParams[0].m_maxValue);
}

// =============================================================================
// Preset Bank Serialization
// =============================================================================

void test_serialize_presetBank() {
  PresetBank bank;
  bank.m_id = 2;
  bank.m_presets[1].m_id = 1;
  bank.m_presets[1].m_programIndex = 3;
  bank.m_presets[1].m_tapState = TapState::kEnabled;
  bank.m_presets[1].m_interval = 512;

  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kPresetBank, 2);
  uint8_t buffer[info.m_length];

  handler.serializePresetBank(bank, buffer, 2, 0);

  TEST_ASSERT_EQUAL(2, buffer[0]);  // bank id

  uint8_t presetOffset = 1 + PresetLayout::c_presetSize;
  TEST_ASSERT_EQUAL(1, buffer[presetOffset]);      // preset id
  TEST_ASSERT_EQUAL(3, buffer[presetOffset + 1]);  // program index
  TEST_ASSERT_EQUAL(1, buffer[presetOffset + 2]);  // tap state enabled
}

void test_deserialize_presetBank() {
  MemoryHandler handler;
  PresetBank bank;

  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kPresetBank, 2);
  uint8_t buffer[info.m_length];
  memset(buffer, 0, info.m_length);

  buffer[0] = 1;  // bank id (will be overwritten by bankIndex param)

  uint8_t presetOffset = 1 + PresetLayout::c_presetSize * 2;
  buffer[presetOffset + PresetLayout::c_id] = 2;
  buffer[presetOffset + PresetLayout::c_programIndex] = 3;
  buffer[presetOffset + PresetLayout::c_tapState] = 1;
  buffer[presetOffset + PresetLayout::c_divState] = 1;
  buffer[presetOffset + PresetLayout::c_divValue] = 2;
  buffer[presetOffset + PresetLayout::c_intervalL] = 0;
  buffer[presetOffset + PresetLayout::c_intervalH] = 2;
  buffer[presetOffset + PresetLayout::c_exprState] = 1;
  buffer[presetOffset + PresetLayout::c_mappedPot] = 2;

  handler.deserializePresetBank(bank, buffer, 2, 0);

  TEST_ASSERT_EQUAL(2, bank.m_id);

  Preset& preset = bank.m_presets[2];
  TEST_ASSERT_EQUAL(2, preset.m_id);
  TEST_ASSERT_EQUAL(3, preset.m_programIndex);
  TEST_ASSERT_EQUAL(TapState::kEnabled, preset.m_tapState);
  TEST_ASSERT_EQUAL(DivState::kEnabled, preset.m_divState);
  TEST_ASSERT_EQUAL(DivValue::kSixteenth, preset.m_divValue);
  TEST_ASSERT_EQUAL(512, preset.m_interval);
  TEST_ASSERT_EQUAL(ExprState::kActive, preset.m_exprState);
  TEST_ASSERT_EQUAL(MappedPot::kPot2, preset.m_mappedPot);
}

void test_presetBank_roundtrip_preserves_all_presets() {
  PresetBank original;
  original.m_id = 1;

  for (uint8_t i = 0; i < PresetConstants::c_presetPerBank; i++) {
    original.m_presets[i].m_id = i;
    original.m_presets[i].m_programIndex = i + 1;
    original.m_presets[i].m_tapState = (i % 2 == 0) ? TapState::kEnabled : TapState::kDisabled;
    original.m_presets[i].m_interval = 100 * (i + 1);
  }

  MemoryHandler handler;
  RegionInfo info = handler.calculateRegionInfo(MemoryRegion::kPresetBank, 1);
  uint8_t buffer[info.m_length];

  handler.serializePresetBank(original, buffer, 1, 0);

  PresetBank restored;
  handler.deserializePresetBank(restored, buffer, 1, 0);

  TEST_ASSERT_EQUAL(original.m_id, restored.m_id);

  for (uint8_t i = 0; i < PresetConstants::c_presetPerBank; i++) {
    TEST_ASSERT_EQUAL(original.m_presets[i].m_id, restored.m_presets[i].m_id);
    TEST_ASSERT_EQUAL(original.m_presets[i].m_programIndex, restored.m_presets[i].m_programIndex);
    TEST_ASSERT_EQUAL(original.m_presets[i].m_tapState, restored.m_presets[i].m_tapState);
    TEST_ASSERT_EQUAL(original.m_presets[i].m_interval, restored.m_presets[i].m_interval);
  }
}

// =============================================================================
// Main
// =============================================================================

int main() {
  UNITY_BEGIN();

  // calculateRegionInfo
  RUN_TEST(test_calculateRegionInfo_bypass);
  RUN_TEST(test_calculateRegionInfo_programMode);
  RUN_TEST(test_calculateRegionInfo_currentProgram);
  RUN_TEST(test_calculateRegionInfo_currentPreset);
  RUN_TEST(test_calculateRegionInfo_currentPresetBank);
  RUN_TEST(test_calculateRegionInfo_midiChannel);
  RUN_TEST(test_calculateRegionInfo_tap);
  RUN_TEST(test_calculateRegionInfo_tempo);
  RUN_TEST(test_calculateRegionInfo_expr_program_index_0);
  RUN_TEST(test_calculateRegionInfo_expr_program_index_7);
  RUN_TEST(test_calculateRegionInfo_pot_program_0_pot_0);
  RUN_TEST(test_calculateRegionInfo_pot_program_3_pot_2);
  RUN_TEST(test_calculateRegionInfo_deviceState);
  RUN_TEST(test_calculateRegionInfo_logicalState);
  RUN_TEST(test_calculateRegionInfo_preset);
  RUN_TEST(test_calculateRegionInfo_presetBank);

  // Serialize: Bypass
  RUN_TEST(test_serialize_bypass_active);
  RUN_TEST(test_serialize_bypass_bypassed);

  // Serialize: ProgramMode
  RUN_TEST(test_serialize_programMode_program);
  RUN_TEST(test_serialize_programMode_preset);

  // Serialize: Simple values
  RUN_TEST(test_serialize_currentProgram);
  RUN_TEST(test_serialize_currentPreset);
  RUN_TEST(test_serialize_currentPresetBank);
  RUN_TEST(test_serialize_midiChannel);

  // Serialize: DeviceState
  RUN_TEST(test_serialize_deviceState);

  // Serialize: Tap
  RUN_TEST(test_serialize_tap_disabled);
  RUN_TEST(test_serialize_tap_enabled_with_values);

  // Serialize: Tempo
  RUN_TEST(test_serialize_tempo_zero);
  RUN_TEST(test_serialize_tempo_value);

  // Serialize: Expr
  RUN_TEST(test_serialize_expr_default);
  RUN_TEST(test_serialize_expr_with_values);
  RUN_TEST(test_serialize_expr_program_index_0);
  RUN_TEST(test_serialize_expr_program_index_7);

  // Serialize: Pot
  RUN_TEST(test_serialize_pot_default);
  RUN_TEST(test_serialize_pot_with_values);

  // Deserialize: Bypass
  RUN_TEST(test_deserialize_bypass_active);
  RUN_TEST(test_deserialize_bypass_bypassed);
  RUN_TEST(test_deserialize_bypass_invalid_defaults_to_active);

  // Deserialize: ProgramMode
  RUN_TEST(test_deserialize_programMode_program);
  RUN_TEST(test_deserialize_programMode_preset);
  RUN_TEST(test_deserialize_programMode_invalid_defaults_to_program);

  // Deserialize: Simple values with clamping
  RUN_TEST(test_deserialize_currentProgram);
  RUN_TEST(test_deserialize_currentProgram_clamps_above_max);
  RUN_TEST(test_deserialize_currentPreset);
  RUN_TEST(test_deserialize_currentPreset_clamps_above_max);
  RUN_TEST(test_deserialize_currentPresetBank);
  RUN_TEST(test_deserialize_currentPresetBank_clamps_above_max);
  RUN_TEST(test_deserialize_midiChannel);
  RUN_TEST(test_deserialize_midiChannel_clamps_above_7);

  // Deserialize: DeviceState
  RUN_TEST(test_deserialize_deviceState);

  // Deserialize: Tap
  RUN_TEST(test_deserialize_tap_disabled);
  RUN_TEST(test_deserialize_tap_enabled_with_values);
  RUN_TEST(test_deserialize_tap_clamps_interval_above_1000);
  RUN_TEST(test_deserialize_tap_invalid_divValue_defaults);

  // Deserialize: Tempo
  RUN_TEST(test_deserialize_tempo);
  RUN_TEST(test_deserialize_tempo_clamps_above_1000);

  // Deserialize: Expr
  RUN_TEST(test_deserialize_expr);
  RUN_TEST(test_deserialize_expr_clamps_values_above_1023);
  RUN_TEST(test_deserialize_expr_invalid_mappedPot_defaults);
  RUN_TEST(test_deserialize_expr_different_program_indices_are_independent);

  // Deserialize: Pot
  RUN_TEST(test_deserialize_pot);
  RUN_TEST(test_deserialize_pot_clamps_values_above_1023);
  RUN_TEST(test_deserialize_pot_invalid_state_defaults_to_active);

  // LogicalState full serialization
  RUN_TEST(test_serialize_logicalState);
  RUN_TEST(test_deserialize_logicalState);
  RUN_TEST(test_logicalState_roundtrip_preserves_data);

  // Validators
  RUN_TEST(test_validator_bypassState);
  RUN_TEST(test_validator_programMode);
  RUN_TEST(test_validator_exprState);
  RUN_TEST(test_validator_mappedPot);
  RUN_TEST(test_validator_direction);
  RUN_TEST(test_validator_potState);
  RUN_TEST(test_validator_tapState);
  RUN_TEST(test_validator_divState);
  RUN_TEST(test_validator_divValue);

  // Preset serialization
  RUN_TEST(test_serialize_preset);
  RUN_TEST(test_deserialize_preset);
  RUN_TEST(test_preset_roundtrip_preserves_data);

  // Preset bank serialization
  RUN_TEST(test_serialize_presetBank);
  RUN_TEST(test_deserialize_presetBank);
  RUN_TEST(test_presetBank_roundtrip_preserves_all_presets);

  UNITY_END();
}
