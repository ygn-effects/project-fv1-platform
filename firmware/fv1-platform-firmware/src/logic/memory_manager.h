#pragma once

#include <Arduino.h>
#include "peripherals/eeprom.h"
#include "logic/device_state.h"
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
 * 5-10             tapHandler         Tap handler data
 *                   | divState        Div state
 *                   | divValue        Div value
 *                   | interval        Interval low byte
 *                   | interval        Interval high byte
 *                   | divInterval     Div interval low byte
 *                   | divinterval     Div interval high byte
 * 11-66            exprHandler        Expr handler data x8
 *                   | state           Expr state
 *                   | mappedPot       Expr mapped pot
 *                   | direction       Expr direction
 *                   | heelValue       Heel value low byte
 *                   | heelValue       Heel value high byte
 *                   | toeValue        Toe value low byte
 *                   | toeValue        Toe value high byte
 */

namespace MemoryManagerConstants {
  constexpr uint8_t c_memoryInitializedAddress = 0x0;
  constexpr uint8_t c_deviceStateStartAddress = 0x1;
  constexpr uint8_t c_stateSize = 5;
  constexpr uint8_t c_tapSize = 6;
  constexpr uint8_t c_exprSize = 7;
  constexpr uint8_t c_exprCount = ProgramConstants::c_maxPrograms;
  constexpr uint8_t c_deviceStateSize = MemoryManagerConstants::c_stateSize + MemoryManagerConstants::c_tapSize + (MemoryManagerConstants::c_exprSize * MemoryManagerConstants::c_exprCount);
}

class MemoryManager {
  private:
    Eeprom eeprom;

    void serializeDeviceState(const DeviceState& t_state, uint8_t* t_buffer) const;
    void deserializeDeviceState(DeviceState& t_state, const uint8_t* t_buffer) const;

  public:
    MemoryManager(uint8_t t_eepromCsPin) :
      eeprom(t_eepromCsPin) {}

    void setup();
    bool isMemoryInitialized();

    void saveDeviceState(const DeviceState& t_state);
    void loadDeviceState(DeviceState& t_state);
};
