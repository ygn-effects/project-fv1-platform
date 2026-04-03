#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "periphs/toggleable.h"
#include "services/midi_service.h"

class ProgramModeService : public Service {
  private:
    LogicalState& m_logicalState;
    Toggleable& m_programModeLed;

    void publishSaveLogicalStateEvent(const Event& t_event) const;
    void publishLoadLogicalStateEvent(const Event& t_event) const;
    void publishSaveProgramModeEvent(const Event& t_event) const;
    void publishProgramModeToggledEvent(const Event& t_event) const;

  public:
    ProgramModeService(LogicalState& t_lState, Toggleable& t_led)
      : m_logicalState(t_lState),
        m_programModeLed(t_led) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(const Event& t_event) const override;
};
