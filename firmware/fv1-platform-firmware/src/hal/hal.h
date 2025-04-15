#pragma once

#include <Arduino.h>
#include "logic/device_state.h"
#include "logic/memory_manager.h"
#include "logic/fsm.h"

class HAL {
  private:
    DeviceState m_deviceState;
    MemoryManager m_memoryManager;
    FSM m_fsm;

    bool m_bypassFootSwitchPress{false};
    bool m_tapFootSwitchPress{false};
    bool m_tapFootSwitchLongPress{false};
    bool m_menuEncoderMove{false};
    bool m_menuEncoderMoveRight{false};
    bool m_menuEncoderMoveLeft{false};
    bool m_menuEncoderSwitchPress{false};
    bool m_menuEncoderSwitchLongPress{false};

    void pollSwitch(MomentarySwitch& t_switch, bool& t_pressFlag);
    void pollSwitch(MomentarySwitch& t_switch, bool& t_pressFlag, bool& t_longPressFlag);

    void pollFootSwitches();
    void pollMenuEncoderSwitch();
    void pollMenuEncoder();

    void transitionToState(AppState t_state);

    void processRestoreState();
    void processProgramIdleState();
    void processPresetIdleState();

  public:
    HAL(uint8_t t_rPin, uint8_t t_okPin, uint8_t t_pot0, uint8_t t_pot1, uint8_t t_pot2, uint8_t t_mixPot, uint8_t t_encPinA, uint8_t t_encPinB, uint8_t t_encSwitch,  uint8_t t_bypSPpin, uint8_t t_tapSPin, uint8_t t_bypLPin, uint8_t t_tapLPin, uint8_t t_eepromCsPin) :
      m_deviceState(t_rPin, t_okPin, t_pot0, t_pot1, t_pot2, t_mixPot,t_encPinA, t_encPinB, t_encSwitch, t_bypSPpin, t_tapSPin , t_bypLPin, t_tapLPin),
      m_memoryManager(t_eepromCsPin),
      m_fsm() {}

    void setup();
    void startup();
    void poll();
    void process();
    void resetTriggers();
};
