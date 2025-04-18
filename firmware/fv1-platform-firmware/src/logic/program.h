#pragma once

#include <stdint.h>

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
  const char* m_name;
  const bool m_isDelayEffect;
  const bool m_supportsTap;
  const bool m_supportsExpr;
  const bool m_isPot0Enabled;
  const bool m_isPot1Enabled;
  const bool m_isPot2Enabled;
  const bool m_isMixPotEnabled;
  const uint16_t m_minDelayInterval;
  const uint16_t m_maxDelayInterval;

  constexpr Program(const char* t_name, bool t_isDelay, bool t_suppTap, bool t_suppExpr,
                    bool t_pot0En, bool t_pot1En, bool t_pot2En, bool t_mixPotEn,
                    uint16_t t_minDelay, uint16_t t_maxDelay) :
    m_name(t_name),
    m_isDelayEffect(t_isDelay),
    m_supportsTap(t_suppTap),
    m_supportsExpr(t_suppExpr),
    m_isPot0Enabled(t_pot0En),
    m_isPot1Enabled(t_pot1En),
    m_isPot2Enabled(t_pot2En),
    m_isMixPotEnabled(t_mixPotEn),
    m_minDelayInterval(t_minDelay),
    m_maxDelayInterval(t_maxDelay) {}
};

