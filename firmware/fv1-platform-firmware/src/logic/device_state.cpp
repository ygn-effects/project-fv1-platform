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

TapHandler& DeviceState::getTapHandler() {
  return m_tapHandler;
}

ExprHandler& DeviceState::getExprHandler(uint8_t t_index) {
  if (t_index < ProgramConstants::c_maxPrograms) {
    return m_exprHandler[t_index];
  }

  return m_exprHandler[ProgramConstants::c_maxPrograms];
}

Potentiometer& DeviceState::getPot0() {
  return m_fv1Pot0;
}

Potentiometer& DeviceState::getPot1() {
  return m_fv1Pot1;
}

Potentiometer& DeviceState::getPot2() {
  return m_fv1Pot2;
}

Potentiometer& DeviceState::getMixPot() {
  return m_mixPot;
}

MomentarySwitch& DeviceState::getBypassSwitch() {
  return m_bypassSwitch;
}

MomentarySwitch& DeviceState::getTapSwitch() {
  return m_tapSwitch;
}

Led& DeviceState::getBypassLed() {
  return m_bypassLed;
}

PwmLed& DeviceState::getTapLed() {
  return m_tapLed;
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
