#pragma once

#include <Arduino.h>
#include "peripherals/eeprom.h"
#include "logic/device_state.h"

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
 *                   | interval        Interval byte 1
 *                   | interval        Interval byte 2
 *                   | divInterval     Div interval byte 1
 *                   | divinterval     Div interval byte 2
 */

namespace MemoryManagerConstants {
  constexpr uint8_t c_deviceStateStartAddress = 0x0;
  constexpr uint8_t c_deviceStateSize = 0xA;
}

class MemoryManager {
  private:


  public:
};
