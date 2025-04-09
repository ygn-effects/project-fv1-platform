#pragma once

#include <Arduino.h>

/**
 * @brief An FV-1 EEPROM will can only contain up to 8 programs, this only needs to be adjusted when :
 *  - The EEPROM contains less than 8 effects
 *  - Multiple EEPROMs are connected (not supported for now)
 */
constexpr uint8_t c_maxPrograms = 8;

struct Program {

};