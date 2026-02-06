#include "logic/preset_handler.h"

void PresetHandler::snapshotFromState(LogicalState& t_lState, uint8_t t_presetIndex) {
  t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_programIndex = t_lState.m_currentProgram;
  t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_tapState = t_lState.m_tapState;
  t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_divState = t_lState.m_divState;
  t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_divValue = t_lState.m_divValue;
  t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_interval = t_lState.m_interval;
  t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_divInterval = t_lState.m_divInterval;
  t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_tempo = t_lState.m_tempo;
  t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_exprState = t_lState.m_exprParams[t_lState.m_currentProgram].m_state;
  t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_mappedPot = t_lState.m_exprParams[t_lState.m_currentProgram].m_mappedPot;
  t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_direction = t_lState.m_exprParams[t_lState.m_currentProgram].m_direction;
  t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_heelValue = t_lState.m_exprParams[t_lState.m_currentProgram].m_heelValue;
  t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_toeValue = t_lState.m_exprParams[t_lState.m_currentProgram].m_toeValue;

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_potParams[i].m_state = t_lState.m_potParams[t_lState.m_currentProgram][i].m_state;
    t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_potParams[i].m_value = t_lState.m_potParams[t_lState.m_currentProgram][i].m_value;
    t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_potParams[i].m_minValue = t_lState.m_potParams[t_lState.m_currentProgram][i].m_minValue;
    t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_potParams[i].m_maxValue = t_lState.m_potParams[t_lState.m_currentProgram][i].m_maxValue;
  }
}

void PresetHandler::applyToState(LogicalState& t_lState, uint8_t t_presetIndex) {
  t_lState.m_currentProgram = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_programIndex;
  t_lState.m_tapState = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_tapState;
  t_lState.m_divState = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_divState;
  t_lState.m_divValue = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_divValue;
  t_lState.m_interval = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_interval;
  t_lState.m_divInterval = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_divInterval;
  t_lState.m_tempo = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_tempo;
  t_lState.m_exprParams[t_lState.m_currentProgram].m_state = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_exprState;
  t_lState.m_exprParams[t_lState.m_currentProgram].m_mappedPot = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_mappedPot;
  t_lState.m_exprParams[t_lState.m_currentProgram].m_direction = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_direction;
  t_lState.m_exprParams[t_lState.m_currentProgram].m_heelValue = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_heelValue;
  t_lState.m_exprParams[t_lState.m_currentProgram].m_toeValue = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_toeValue;

  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    t_lState.m_potParams[t_lState.m_currentProgram][i].m_state = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_potParams[i].m_state;
    t_lState.m_potParams[t_lState.m_currentProgram][i].m_value = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_potParams[i].m_value;
    t_lState.m_potParams[t_lState.m_currentProgram][i].m_minValue = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_potParams[i].m_minValue;
    t_lState.m_potParams[t_lState.m_currentProgram][i].m_maxValue = t_lState.m_loadedPresetBank.m_presets[t_presetIndex].m_potParams[i].m_maxValue;
  }
}
