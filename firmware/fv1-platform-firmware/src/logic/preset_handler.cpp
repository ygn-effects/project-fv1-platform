#include "logic/preset_handler.h"

void PresetHandler::snapshotFromState(const LogicalState& t_lState) {
  m_snapshot.m_programIndex = t_lState.m_currentProgram;
  m_snapshot.m_tapState = t_lState.m_tapState;
  m_snapshot.m_divState = t_lState.m_divState;
  m_snapshot.m_divValue = t_lState.m_divValue;
  m_snapshot.m_interval = t_lState.m_interval;
  m_snapshot.m_divInterval = t_lState.m_divInterval;
  m_snapshot.m_tempo = t_lState.m_tempo;
  m_snapshot.m_exprState = t_lState.m_exprParams[t_lState.m_currentProgram].m_state;
  m_snapshot.m_mappedPot = t_lState.m_exprParams[t_lState.m_currentProgram].m_mappedPot;
  m_snapshot.m_direction = t_lState.m_exprParams[t_lState.m_currentProgram].m_direction;
  m_snapshot.m_heelValue = t_lState.m_exprParams[t_lState.m_currentProgram].m_heelValue;
  m_snapshot.m_toeValue = t_lState.m_exprParams[t_lState.m_currentProgram].m_toeValue;

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    m_snapshot.m_potParams[i].m_state = t_lState.m_potParams[t_lState.m_currentProgram][i].m_state;
    m_snapshot.m_potParams[i].m_value = t_lState.m_potParams[t_lState.m_currentProgram][i].m_value;
    m_snapshot.m_potParams[i].m_minValue = t_lState.m_potParams[t_lState.m_currentProgram][i].m_minValue;
    m_snapshot.m_potParams[i].m_maxValue = t_lState.m_potParams[t_lState.m_currentProgram][i].m_maxValue;
  }
}

void PresetHandler::applyToState(LogicalState& t_lState, uint8_t t_presetIndex) {
  t_lState.m_currentProgram = m_currentPresetBank->m_presets[t_presetIndex].m_programIndex;
  t_lState.m_tapState = m_currentPresetBank->m_presets[t_presetIndex].m_tapState;
  t_lState.m_divState = m_currentPresetBank->m_presets[t_presetIndex].m_divState;
  t_lState.m_divValue = m_currentPresetBank->m_presets[t_presetIndex].m_divValue;
  t_lState.m_interval = m_currentPresetBank->m_presets[t_presetIndex].m_interval;
  t_lState.m_divInterval = m_currentPresetBank->m_presets[t_presetIndex].m_divInterval;
  t_lState.m_tempo = m_currentPresetBank->m_presets[t_presetIndex].m_tempo;
  t_lState.m_exprParams[t_lState.m_currentProgram].m_state = m_currentPresetBank->m_presets[t_presetIndex].m_exprState;
  t_lState.m_exprParams[t_lState.m_currentProgram].m_mappedPot = m_currentPresetBank->m_presets[t_presetIndex].m_mappedPot;
  t_lState.m_exprParams[t_lState.m_currentProgram].m_direction = m_currentPresetBank->m_presets[t_presetIndex].m_direction;
  t_lState.m_exprParams[t_lState.m_currentProgram].m_heelValue = m_currentPresetBank->m_presets[t_presetIndex].m_heelValue;
  t_lState.m_exprParams[t_lState.m_currentProgram].m_toeValue = m_currentPresetBank->m_presets[t_presetIndex].m_toeValue;

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    t_lState.m_potParams[t_lState.m_currentProgram][i].m_state = m_currentPresetBank->m_presets[t_presetIndex].m_potParams[i].m_state;
    t_lState.m_potParams[t_lState.m_currentProgram][i].m_value = m_currentPresetBank->m_presets[t_presetIndex].m_potParams[i].m_value;
    t_lState.m_potParams[t_lState.m_currentProgram][i].m_minValue = m_currentPresetBank->m_presets[t_presetIndex].m_potParams[i].m_minValue;
    t_lState.m_potParams[t_lState.m_currentProgram][i].m_maxValue = m_currentPresetBank->m_presets[t_presetIndex].m_potParams[i].m_maxValue;
  }
}
