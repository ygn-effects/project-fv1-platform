#include "logic/device_state.h"

BypassState DeviceState::getBypassState() const {
  return m_bypassState;
}

ProgramMode DeviceState::getProgramMode() const {
  return m_programMode;
}

uint8_t DeviceState::getCurrentProgram() const {
  return m_currentProgram;
}

uint8_t DeviceState::getCurrentPreset() const {
  return m_currentPreset;
}

uint8_t DeviceState::getMidiChannel() const {
  return m_midiChannel;
}

TapHandler* DeviceState::getTapHandler() const {
  return m_tapHandler;
}

ExprHandler* DeviceState::getExprHandler(uint8_t t_index) const {
  if (t_index < c_maxPrograms) {
    return m_exprHandler[t_index];
  }

  return nullptr;
}

void DeviceState::setBypassState(BypassState t_state) {
  m_bypassState = t_state;
}

void DeviceState::setProgramMode(ProgramMode t_mode) {
  m_programMode = t_mode;
}

void DeviceState::setCurrentProgram(uint8_t t_program) {
  m_currentProgram = t_program;
}

void DeviceState::setCurrentPreset(uint8_t t_preset) {
  m_currentPreset = t_preset;
}

void DeviceState::setMidiChannel(uint8_t t_channel) {
  m_midiChannel = t_channel;
}

void DeviceState::setTapHandler(TapHandler* t_handler) {
  m_tapHandler = t_handler;
}

void DeviceState::setExprHandler(uint8_t t_index, ExprHandler* t_handler) {
  if (t_index < c_maxPrograms) {
    m_exprHandler[t_index] = t_handler;
  }
}
