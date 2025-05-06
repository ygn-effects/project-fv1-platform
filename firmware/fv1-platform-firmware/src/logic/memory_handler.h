#pragma once

#include <stdint.h>
#include "logic/logical_state.h"
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
 * 4                midiChannel        MIDI channel of the device
 * 5-11             tapHandler         Tap handler data
 *                   | tapState        Tap state
 *                   | divState        Div state
 *                   | divValue        Div value
 *                   | interval        Interval low byte
 *                   | interval        Interval high byte
 *                   | divInterval     Div interval low byte
 *                   | divinterval     Div interval high byte
 * 12               tempo              Current tempo low byte
 * 13               tempo              Current tempo high byte
 * 14-69            exprParam          Expr handler data x8
 *                   | state           Expr state
 *                   | mappedPot       Expr mapped pot
 *                   | direction       Expr direction
 *                   | heelValue       Heel value low byte
 *                   | heelValue       Heel value high byte
 *                   | toeValue        Toe value low byte
 *                   | toeValue        Toe value high byte
 * 70-293           potParam           Pot param data
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
  kBypass,
  kProgramMode,
  kCurrentProgram,
  kCurrentPreset,
  kMidiChannel,
  kState,
  kTap,
  kTempo,
  kPot,
  kExpr
};

namespace MemoryLayout {
  constexpr uint16_t c_deviceStateStart = 0;
  constexpr uint8_t c_deviceStateSize = 5;
  constexpr uint16_t c_bypassState = c_deviceStateStart;
  constexpr uint16_t c_programMode = c_deviceStateStart + 1;
  constexpr uint16_t c_currentProgram = c_deviceStateStart + 2;
  constexpr uint16_t c_currentPreset = c_deviceStateStart + 3;
  constexpr uint16_t c_midiChannel = c_deviceStateStart + 4;

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

class MemoryHandler {
  private:
    void serializeBypass(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeProgramMode(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeCurrentProgram(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeCurrentPreset(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeMidiChannel(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeDeviceState(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeTap(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeTempo(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex);
    void serializeExprParam(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex, uint8_t t_programIndex);
    void serializePotParam(const LogicalState& t_lState, uint8_t* t_buffer, uint16_t t_startIndex, uint8_t t_potIndex);

  public:
    RegionInfo calculateRegionInfo(MemoryRegion t_region, uint8_t t_programIndex = 0, uint8_t t_index = 0);
    void serializeRegion(MemoryRegion t_region, const LogicalState& t_lState, uint8_t* t_buffer, uint8_t t_programIndex = 0, uint8_t t_index = 0);
};
