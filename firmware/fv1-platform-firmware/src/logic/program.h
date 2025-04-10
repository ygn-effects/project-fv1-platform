#pragma once

#include <Arduino.h>
#include "utils/static_string.h"

/**
 * @brief Constants used for programs managing.
 */
namespace ProgramConstants {
  /**
   * @brief Max number of programs available
   *
   * An FV-1 EEPROM can only contain up to 8 programs, this only needs to be adjusted when :
   *
   * - The EEPROM contains less than 8 effects
   *
   * - Multiple EEPROMs are connected (not supported for now)
   */
  constexpr uint8_t c_maxPrograms = 8;
}

/**
 * @brief Stores a program properties.
 */
struct Program {
  const StaticString m_name;          // Program name
  const bool m_isDelayEffect;         // Is the program a delay effect
  const bool m_supportsTap;           // Does the program supports tap temp
  const bool m_supportsExpr;          // Does the program supports the expression pedal input
  const bool m_isPot0Enabled;         // Is FV-1's POT0 used
  const bool m_isPot1Enabled;         // Is FV-1's POT1 used
  const bool m_isPot2Enabled;         // Is FV-1's POT2 used
  const bool m_isMixPotEnabled;       // Is the MIX pot used
  const uint16_t m_minDelayInterval;  // Minimum interval if delay effect
  const uint16_t m_maxDelayInterval;  // Maximum interval if delay effect

  Program(const char* t_name, bool t_isDelay, bool t_suppTap, bool t_suppExpr, uint8_t t_pot0En, uint8_t t_pot1En, uint8_t t_pot2En, uint8_t t_mixPotEn, uint16_t t_minDelay, uint16_t t_maxDelay) :
    m_name(t_name),
    m_isDelayEffect(t_maxDelay),
    m_supportsTap(t_suppTap),
    m_supportsExpr(t_suppExpr),
    m_isPot0Enabled(t_pot0En),
    m_isPot1Enabled(t_pot1En),
    m_isPot2Enabled(t_pot2En),
    m_isMixPotEnabled(t_mixPotEn),
    m_minDelayInterval(t_minDelay),
    m_maxDelayInterval(t_maxDelay) {}
};
