#include "logic/fv1_handler.h"

uint16_t Fv1Handler::mapTempoValue(uint16_t t_value) {
  return Utils::mapClamped<uint16_t>(t_value,
      m_tempoConfig.m_minLogical,
      m_tempoConfig.m_maxLogical,
      m_tempoConfig.m_minRaw,
      m_tempoConfig.m_maxRaw
    );
}

uint16_t Fv1Handler::mapPotValue(uint8_t t_potIndex, uint16_t t_value) {
  return Utils::mapClamped<uint16_t>(t_value,
      m_potConfigs[t_potIndex].m_minRaw,
      m_potConfigs[t_potIndex].m_maxRaw,
      m_potConfigs[t_potIndex].m_minLogical,
      m_potConfigs[t_potIndex].m_maxLogical
    );
}
