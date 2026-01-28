#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/midi_service.h"

class ProgramModeService : public Service {
  private:
    LogicalState& m_logicalState;

    void publishSaveLogicalStateEvent(const Event& t_event) const;
    void publishLoadLogicalStateEvent(const Event& t_event) const;
    void publishSaveProgramModeEvent(const Event& t_event) const;
    void publishProgramModeToggledEvent(const Event& t_event) const;

  public:
    ProgramModeService(LogicalState& t_lState)
      : m_logicalState(t_lState) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(const Event& t_event) const override;
};
