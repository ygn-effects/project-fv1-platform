#include "hal/hal.h"

void HAL::pollSwitch(MomentarySwitch& t_switch, bool& t_pressFlag) {
  t_switch.poll();

  t_pressFlag = t_switch.isPushed();
}

void HAL::pollSwitch(MomentarySwitch& t_switch, bool& t_pressFlag, bool& t_longPressFlag) {
  t_switch.poll();

  t_pressFlag = t_switch.isPushed();
  t_longPressFlag = t_switch.isLongPress();
}

void HAL::pollFootSwitches() {
  pollSwitch(m_deviceState.getBypassSwitch(), m_bypassFootSwitchPress);
  pollSwitch(m_deviceState.getTapSwitch(), m_tapFootSwitchPress, m_tapFootSwitchLongPress);
}

void HAL::pollMenuEncoderSwitch() {
  pollSwitch(m_deviceState.getMenuEncoderSwitch(), m_menuEncoderSwitchPress, m_menuEncoderSwitchLongPress);
}

void HAL::pollMenuEncoder() {
  if (m_deviceState.getMenuEncoder().poll()) {
    m_menuEncoderMove = true;

    if (m_deviceState.getMenuEncoder().isMovedRight()) {
      m_menuEncoderMoveRight = true;
    }
    else {
      m_menuEncoderMoveLeft = true;
    }
  }
}

void HAL::transitionToState(AppState t_state) {
  m_fsm.setState(t_state);
}

void HAL::processRestoreState() {
  m_memoryManager.loadDeviceState(m_deviceState);

  switch (m_deviceState.getProgramMode()) {
    case ProgramMode::kProgram:
      transitionToState(AppState::kProgramIdle);
      break;

    case ProgramMode::kPreset:
      transitionToState(AppState::kPresetIdle);
      break;

    default:
      transitionToState(AppState::kProgramIdle);
      break;
  }
}

void HAL::processProgramIdleState() {
  if (m_bypassFootSwitchPress) {
    m_deviceState.switchBypass();
    m_memoryManager.saveDeviceState(m_deviceState);
  }
}

void HAL::setup() {
  m_deviceState.getPot0().setup();
  m_deviceState.getPot1().setup();
  m_deviceState.getPot2().setup();
  m_deviceState.getMixPot().setup();
  m_deviceState.getMenuEncoder().setup();
  m_deviceState.getMenuEncoderSwitch().setup();
  m_deviceState.getBypassSwitch().setup();
  m_deviceState.getTapSwitch().setup();
  m_deviceState.getBypassLed().setup();
  m_deviceState.getTapLed().setup();
}

void HAL::startup() {
  m_deviceState.getPot0().read();
  m_deviceState.getPot1().read();
  m_deviceState.getPot2().read();
  m_deviceState.getMixPot().read();
  m_deviceState.getMenuEncoder().poll();
  m_deviceState.getMenuEncoderSwitch().poll();
  m_deviceState.getBypassSwitch().poll();
  m_deviceState.getTapSwitch().poll();

  transitionToState(AppState::kRestoreState);
}

void HAL::poll() {
  switch (m_fsm.getState()) {
    case AppState::kProgramIdle:
    case AppState::kPresetIdle:
      pollFootSwitches();
      pollMenuEncoderSwitch();
      break;

    case AppState::kProgramEdit:
      pollFootSwitches();
      pollMenuEncoder();
      pollMenuEncoderSwitch();
      break;

    case AppState::kBoot:
    case AppState::kRestoreState:
    default:
      break;
  }
}

void HAL::process() {
  switch (m_fsm.getState()) {
    case AppState::kRestoreState:
      processRestoreState();
      break;

    case AppState::kProgramIdle:
      processProgramIdleState();
      break;

    case AppState::kPresetIdle:
      processPresetIdleState();

    default:
      break;
  }
}

void HAL::resetTriggers() {
  m_bypassFootSwitchPress = false;
  m_tapFootSwitchPress = false;
  m_tapFootSwitchLongPress = false;
  m_menuEncoderMove = false;
  m_menuEncoderMoveRight = false;
  m_menuEncoderMoveLeft = false;
  m_menuEncoderSwitchPress = false;
  m_menuEncoderSwitchLongPress = false;
}
