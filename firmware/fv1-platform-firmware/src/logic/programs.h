#pragma once
#include "logic/program.h"

namespace ProgramsDefinitions {
  constexpr Program kPrograms[ProgramConstants::c_maxPrograms]{
    {
      "Digital delay", // m_name
      {
        // m_label, m_min, m_max, m_fineStep, m_coarseStrep, m_unit, m_editable
        { "Delay time", 20, 1000, 1, 5, ParamUnit::ms, true },
        { "Feedback", 0, 100, 1, 5, ParamUnit::Percent, true },
        { "Low pass", 100, 1000, 1, 5, ParamUnit::Hz, true },
        { "Mix", 0, 100, 1, 5, ParamUnit::Percent, true }
      },
      true,  // m_isDelayEffect
      20,    // m_minDelayMs
      1000,  // m_maxDelayMs
      true,  // m_supportsTap
      true   // m_supportsExpr
    },
    {
      "Analog delay", // m_name
      {
        // m_label, m_min, m_max, m_fineStep, m_coarseStrep, m_unit, m_editable
        { "Delay time", 100, 800, 1, 5, ParamUnit::ms, true },
        { "Feedback", 0, 100, 1, 5, ParamUnit::Percent, true },
        { "Low pass", 100, 1000, 1, 5, ParamUnit::Hz, true },
        { "Mix", 0, 100, 1, 5, ParamUnit::Percent, true }
      },
      true,  // m_isDelayEffect
      100,   // m_minDelayMs
      800,   // m_maxDelayMs
      true,  // m_supportsTap
      true   // m_supportsExpr
    }
  };
}
