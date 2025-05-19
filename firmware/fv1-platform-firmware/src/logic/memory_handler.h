#pragma once

#include <stdint.h>
#include "logic/logical_state.h"
#include "logic/preset.h"
#include "utils/utils.h"

/*
 * Memory Map for Device state Storage in EEPROM
 *
 * Byte Range       Field Name         Description
 * -----------------------------------------------------------------
 * 0                bypassState        Bypass state of the device
 * 1                programMode        Program/Preset mode
 * 2                currentProgram     Last used program
 * 3                currentPreset      Last used preset
 * 4                currentPresetBank  Last used preset bank
 * 5                midiChannel        MIDI channel of the device
 * 6-12             tapHandler         Tap handler data
 *                   | tapState        Tap state
 *                   | divState        Div state
 *                   | divValue        Div value
 *                   | interval        Interval low byte
 *                   | interval        Interval high byte
 *                   | divInterval     Div interval low byte
 *                   | divinterval     Div interval high byte
 * 13               tempo              Current tempo low byte
 * 14               tempo              Current tempo high byte
 * 15-70            exprParam          Expr handler data x8
 *                   | state           Expr state
 *                   | mappedPot       Expr mapped pot
 *                   | direction       Expr direction
 *                   | heelValue       Heel value low byte
 *                   | heelValue       Heel value high byte
 *                   | toeValue        Toe value low byte
 *                   | toeValue        Toe value high byte
 * 71-294           potParam           Pot param data
 *                   | state           Pot state
 *                   | value           Pot value low byte
 *                   | value           Pot value high byte
 *                   | minValue        Min value low bytes
 *                   | minValue        Min value high byte
 *                   | maxValue        Max value low bytes
 *                   | maxValue        Max value high byte
 */

/*
 * Memory Map for Preset Bank Storage in EEPROM
 *
 * Byte Range       Field Name         Description
 * -----------------------------------------------------------------
 * 0                id                 Bank ID
 * 1 - N            presets            Array of presets (see below)
 */

/*
 * Memory Map for Single Preset in EEPROM
 *
 * Byte Range       Field Name         Description
 * -----------------------------------------------------------------
 * 0                id                 Preset ID
 * 1                programIndex       Active program
 * 2 - 8            tapHandler         Tap handler data
 *                   | tapState        Tap state
 *                   | divState        Div state
 *                   | divValue        Div value
 *                   | interval        Interval low byte
 *                   | interval        Interval high byte
 *                   | divInterval     Div interval low byte
 *                   | divinterval     Div interval high byte
 * 9                tempo              Current tempo low byte
 * 10               tempo              Current tempo high byte
 * 11 - 17          exprParam          Expr handler data
 *                   | state           Expr state
 *                   | mappedPot       Expr mapped pot
 *                   | direction       Expr direction
 *                   | heelValue       Heel value low byte
 *                   | heelValue       Heel value high byte
 *                   | toeValue        Toe value low byte
 *                   | toeValue        Toe value high byte
 * 18 - 45          potParam           Pot param data x4 (4 × 7 bytes)
 *                   | state           Pot state
 *                   | value           Pot value low byte
 *                   | value           Pot value high byte
 *                   | minValue        Min value low bytes
 *                   | minValue        Min value high byte
 *                   | maxValue        Max value low bytes
 *                   | maxValue        Max value high byte
 */

enum class MemoryRegion : uint8_t {
  kDeviceState,
  kLogicalState,
  kBypass,
  kProgramMode,
  kCurrentProgram,
  kCurrentPreset,
  kCurrentPresetBank,
  kMidiChannel,
  kState,
  kTap,
  kTempo,
  kPot,
  kExpr,
  kPreset,
  kPresetBank
};

namespace MemoryLayout {
  constexpr uint16_t c_deviceStateStart = 0;
  constexpr uint8_t c_deviceStateSize = 6;
  constexpr uint16_t c_bypassState = c_deviceStateStart;
  constexpr uint16_t c_programMode = c_deviceStateStart + 1;
  constexpr uint16_t c_currentProgram = c_deviceStateStart + 2;
  constexpr uint16_t c_currentPreset = c_deviceStateStart + 3;
  constexpr uint16_t c_currentPresetBank = c_deviceStateStart + 4;
  constexpr uint16_t c_midiChannel = c_deviceStateStart + 5;

  constexpr uint16_t c_tapStart = c_deviceStateSize;
  constexpr uint8_t c_tapSize = 7;
  constexpr uint16_t c_tapState = c_tapStart;
  constexpr uint16_t c_divState = c_tapStart + 1;
  constexpr uint16_t c_divValue = c_tapStart + 2;
  constexpr uint16_t c_intervalL = c_tapStart + 3;
  constexpr uint16_t c_intervalH = c_tapStart + 4;
  constexpr uint16_t c_divIntervalL = c_tapStart + 5;
  constexpr uint16_t c_divIntervalH = c_tapStart + 6;

  constexpr uint16_t c_tempoStart = c_tapStart + c_tapSize;
  constexpr uint8_t c_tempoSize = 2;
  constexpr uint16_t c_tempoL = c_tempoStart;
  constexpr uint16_t c_tempoH = c_tempoStart + 1;

  constexpr uint16_t c_exprStart = c_tempoStart + c_tempoSize;
  constexpr uint8_t c_exprSize = 7;
  constexpr uint16_t c_exprCount = ProgramConstants::c_maxPrograms;

  constexpr uint16_t c_potParamStart = c_exprStart + c_exprSize * c_exprCount;
  constexpr uint8_t c_potParamSize = 7;
  constexpr uint16_t c_potParamTotal = PotConstants::c_potCount * ProgramConstants::c_maxPrograms;
  constexpr uint16_t c_potParamEnd = c_potParamStart + c_potParamSize * c_potParamTotal;

  constexpr uint16_t getExprParamOffset(uint8_t t_programIndex) {
    return c_exprStart + (t_programIndex * c_exprSize);
  }

  constexpr uint16_t getPotParamOffset(uint8_t t_programIndex, uint8_t t_potIndex) {
    return c_potParamStart + ((t_programIndex * PotConstants::c_potCount + t_potIndex) * c_potParamSize);
  }
}

namespace PresetLayout {
  constexpr uint16_t c_presetBankStart = MemoryLayout::c_potParamEnd + 1;
  constexpr uint8_t c_presetSize = 46;
  constexpr uint8_t c_presetsPerBank = PresetConstants::c_presetPerBank;
  constexpr uint8_t c_bankHeaderSize = 1;

  constexpr uint16_t c_id = 0;
  constexpr uint16_t c_programIndex = 1;

  constexpr uint16_t c_tapStart = 2;
  constexpr uint16_t c_tapState = c_tapStart;
  constexpr uint16_t c_divState = c_tapStart + 1;
  constexpr uint16_t c_divValue = c_tapStart + 2;
  constexpr uint16_t c_intervalL = c_tapStart + 3;
  constexpr uint16_t c_intervalH = c_tapStart + 4;
  constexpr uint16_t c_divIntervalL = c_tapStart + 5;
  constexpr uint16_t c_divIntervalH = c_tapStart + 6;

  constexpr uint16_t c_tempoL = 9;
  constexpr uint16_t c_tempoH = 10;

  constexpr uint16_t c_exprStart = 11;
  constexpr uint16_t c_exprState = c_exprStart;
  constexpr uint16_t c_mappedPot = c_exprStart + 1;
  constexpr uint16_t c_direction = c_exprStart + 2;
  constexpr uint16_t c_heelValueL = c_exprStart + 3;
  constexpr uint16_t c_heelValueH = c_exprStart + 4;
  constexpr uint16_t c_toeValueL = c_exprStart + 5;
  constexpr uint16_t c_toeValueH = c_exprStart + 6;

  constexpr uint16_t c_potParamStart = 18;
  constexpr uint16_t c_potParamSize = 7;

  constexpr uint16_t getPresetOffset(uint8_t t_bankIndex, uint8_t t_presetIndex) {
    return c_presetBankStart + ((t_bankIndex * (c_bankHeaderSize + c_presetsPerBank * c_presetSize)) +
           c_bankHeaderSize + (t_presetIndex * c_presetSize));
  }

  constexpr uint16_t getBankOffset(uint8_t t_bankIndex) {
    return c_presetBankStart + (t_bankIndex * (c_bankHeaderSize + c_presetsPerBank * c_presetSize));
  }
}

struct RegionInfo {
  uint16_t m_address;
  uint16_t m_length;
};

namespace LogicalStateValidator {
  inline BypassState setSafeBypassState(uint8_t t_eepromValue) {
    return BypassStateValidator::sanitize(t_eepromValue, BypassState::kActive);
  }

  inline ProgramMode setSafeProgramMode(uint8_t t_eepromValue) {
    return ProgramModeValidator::sanitize(t_eepromValue, ProgramMode::kProgram);
  }

  inline ExprState setSafeExprState(uint8_t t_eepromValue) {
    return ExprStateValidator::sanitize(t_eepromValue, ExprState::kInactive);
  }

  inline MappedPot setSafeMappedPot(uint8_t t_eepromValue) {
    return MappedPotValidator::sanitize(t_eepromValue, MappedPot::kPot0);
  }

  inline Direction setSafeDirection(uint8_t t_eepromValue) {
    return DirectionValidator::sanitize(t_eepromValue, Direction::kNormal);
  }

  inline PotState setSafePotState(uint8_t t_eepromValue) {
    return PotStateValidator::sanitize(t_eepromValue, PotState::kActive);
  }

  inline TapState setSafeTapState(uint8_t t_eepromValue) {
    return TapStateValidator::sanitize(t_eepromValue, TapState::kDisabled);
  }

  inline DivState setSafeDivState(uint8_t t_eepromValue) {
    return DivStateValidator::sanitize(t_eepromValue, DivState::kDisabled);
  }

  inline DivValue setSafeDivValue(uint8_t t_eepromValue) {
    return DivValueValidator::sanitize(t_eepromValue, DivValue::kQuarter);
  }
};

struct MemoryHandler {
  private:
    void serializeBypass(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeProgramMode(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeCurrentProgram(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeCurrentPreset(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeCurrentPresetBank(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeMidiChannel(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeDeviceState(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeTap(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeTempo(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeExprParam(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex, uint8_t t_programIndex);
    void serializePotParam(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex, uint8_t t_programIndex, uint8_t t_potIndex);
    void serializeLogicalState(const LogicalState& t_lState, uint8_t* t_buffer);

    void deserializeBypass(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex);
    void deserializeProgramMode(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex);
    void deserializeCurrentProgram(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex);
    void deserializeCurrentPreset(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex);
    void deserializeCurrentPresetBank(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex);
    void deserializeMidiChannel(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex);
    void deserializeDeviceState(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex);
    void deserializeTap(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex);
    void deserializeTempo(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex);
    void deserializeExprParam(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex, uint8_t t_programIndex);
    void deserializePotParam(LogicalState& t_lState, const uint8_t* t_buffer, uint16_t t_startIndex, uint8_t t_programIndex, uint8_t t_potIndex);
    void deserializeLogicalState(LogicalState& t_lState, const uint8_t* t_buffer);

  public:
    RegionInfo calculateRegionInfo(MemoryRegion t_region, uint8_t t_programIndex = 0, uint8_t t_index = 0);
    void serializeRegion(MemoryRegion t_region, const LogicalState& t_lState, uint8_t* t_buffer, uint8_t t_programIndex = 0, uint8_t t_index = 0);
    void deserializeRegion(MemoryRegion t_region, LogicalState& t_lState, const uint8_t* t_buffer, uint8_t t_programIndex = 0, uint8_t t_index = 0);
    void serializePreset(const Preset& t_preset, uint8_t* t_buffer, uint8_t t_bankIndex, uint8_t t_presetIndex, uint8_t t_startIndex);
    void deserializePreset(Preset& t_preset, const uint8_t* t_buffer, uint8_t t_bankIndex, uint8_t t_presetIndex, uint8_t t_startIndex);
    void serializePresetBank(const PresetBank& t_presetBank, uint8_t* t_buffer, uint8_t t_bankIndex, uint8_t t_startIndex);
    void deserializePresetBank(PresetBank& t_presetBank, const uint8_t* t_buffer, uint8_t t_bankIndex, uint8_t t_startIndex);
};
