#pragma once

#include <Arduino.h>
#include "logic/tap_handler.h"
#include "logic/expr_handler.h"
#include "logic/program.h"
#include "peripherals/bypass.h"
#include "peripherals/switch.h"
#include "peripherals/potentiometer.h"
#include "peripherals/led.h"

/**
 * @brief State of the device bypass system.
 */
enum class BypassState : uint8_t {
  kBypassed,
  kActive
};

/**
 * @brief Current working mode of the device.
 */
enum class ProgramMode : uint8_t {
  kProgram,
  kPreset
};

/**
 * @brief Stores the state of the various moving parts of the device.
 */
class DeviceState {
  private:
    BypassState m_bypassState{BypassState::kActive};            // Whether the device was bypassed or not
    ProgramMode m_programMode{ProgramMode::kProgram};           // Whether the device was on program or preset mode
    Program m_programs[ProgramConstants::c_maxPrograms];
    uint8_t m_currentProgram{0};                                // The current program the deviced was on
    uint8_t m_currentPreset{0};                                 // The current preset the device was on
    uint8_t m_midiChannel{0};                                   // MIDI channel of the device

    TapHandler m_tapHandler;                                    // Object handling the tap state
    ExprHandler m_exprHandler[ProgramConstants::c_maxPrograms]; // Array of ExprHandlers, one for each program

    Bypass bypass;
    Potentiometer m_fv1Pot0;
    Potentiometer m_fv1Pot1;
    Potentiometer m_fv1Pot2;
    Potentiometer m_mixPot;
    MomentarySwitch m_bypassSwitch;
    MomentarySwitch m_tapSwitch;
    Led m_bypassLed;
    PwmLed m_tapLed;

  public:
    /**
     * @brief Default constructor
     */
    DeviceState(uint8_t t_rPin, uint8_t t_okPin, uint8_t t_pot0, uint8_t t_pot1, uint8_t t_pot2, uint8_t t_mixPot, uint8_t t_bypSPpin, uint8_t t_tapSPin, uint8_t t_bypLPin, uint8_t t_tapLPin) :
      bypass(t_rPin, t_okPin),
      m_fv1Pot0(t_pot0),
      m_fv1Pot1(t_pot1),
      m_fv1Pot2(t_pot2),
      m_mixPot(t_mixPot),
      m_bypassSwitch(t_bypSPpin),
      m_tapSwitch(t_tapSPin),
      m_bypassLed(t_bypLPin),
      m_tapLed(t_tapLPin) {}

    TapHandler& getTapHandler();
    const TapHandler& getTapHandler() const;
    ExprHandler& getExprHandler(uint8_t t_index);
    const ExprHandler& getExprHandler(uint8_t t_index) const;
    Potentiometer& getPot0();
    Potentiometer& getPot1();
    Potentiometer& getPot2();
    Potentiometer& getMixPot();
    MomentarySwitch& getBypassSwitch();
    MomentarySwitch& getTapSwitch();
    Led& getBypassLed();
    PwmLed& getTapLed();

    BypassState getBypassState() const;
    ProgramMode getProgramMode() const;
    uint8_t getCurrentProgram() const;
    uint8_t getCurrentPreset() const;
    uint8_t getMidiChannel() const;

    void setBypassState(BypassState t_state);
    void setProgramMode(ProgramMode t_mode);
    void setCurrentProgram(uint8_t t_program);
    void setCurrentPreset(uint8_t t_preset);
    void setMidiChannel(uint8_t t_channel);
};