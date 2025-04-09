#pragma once

#include <Arduino.h>
#include "tap_handler.h"
#include "expr_handler.h"
#include "program.h"

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
    BypassState m_bypassState{BypassState::kActive};    // Whether the device was bypassed or not
    ProgramMode m_programMode{ProgramMode::kProgram};   // Whether the device was on program or preset mode
    uint8_t m_currentProgram{0};                        // The current program the deviced was on
    uint8_t m_currentPreset{0};                         // The current preset the device was on
    uint8_t m_midiChannel{0};                           // MIDI channel of the device

    TapHandler* m_tapHandler{nullptr};                  // Object handling the tap state
    ExprHandler* m_exprHandler[c_maxPrograms]{nullptr}; // Array of ExprHandlers, one for each program

  public:
    /**
     * @brief Default constructor
     */
    DeviceState();

    BypassState getBypassState() const;
    ProgramMode getProgramMode() const;
    uint8_t getCurrentProgram() const;
    uint8_t getCurrentPreset() const;
    uint8_t getMidiChannel() const;

    TapHandler* getTapHandler() const;
    ExprHandler* getExprHandler(uint8_t t_index) const;

    void setBypassState(BypassState t_state);
    void setProgramMode(ProgramMode t_mode);
    void setCurrentProgram(uint8_t t_program);
    void setCurrentPreset(uint8_t t_preset);
    void setMidiChannel(uint8_t t_channel);

    void setTapHandler(TapHandler* t_handler);
    void setExprHandler(uint8_t t_index, ExprHandler* t_handler);
};