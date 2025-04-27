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
  constexpr uint8_t c_maxParameters = 4;
}

enum class ParamUnit : uint8_t {
  kNone,
  kRaw,
  kPercent,
  kHz,
  kDb,
  kMs
};

struct ProgramParameter {
  const char* m_label;
  uint16_t m_min;
  uint16_t m_max;
  uint8_t m_fineStep;
  uint8_t m_coarseStep;
  ParamUnit m_unit;
  bool m_editable;
};

struct Program {
  const char* m_name;
  ProgramParameter m_params[ProgramConstants::c_maxParameters];
  bool m_isDelayEffect;
  uint16_t m_minDelayMs;
  uint16_t m_maxDelayMs;
  bool m_supportsTap;
  bool m_supportsExpr;
};
