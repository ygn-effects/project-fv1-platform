#pragma once

#include <stdint.h>
#include "core/event_bus.h"
#include "core/service.h"
#include "periphs/toggleable.h"
#include "logic/logical_state.h"
#include "services/midi_service.h"
#include "ui/inputs.h"

/**
 * @brief Manages the hardware bypass relay and synchronizes it with LogicalState.
 *
 * Listens for: Physical Switch (Bypass), MIDI CC (Bypass).
 *
 * Publishes: State Toggled, Save Request.
 */
class BypassService : public Service {
  private:
    LogicalState& m_logicalState;
    Toggleable& m_bypass;

    void publishSaveBypassEvent(const Event& t_event) const;
    void publishBypassToggledEvent(const Event& t_event) const;

  public:
    BypassService(LogicalState& t_lState, Toggleable& t_bypass) :
      m_logicalState(t_lState), m_bypass(t_bypass) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(const Event& t_event) const override;
};
