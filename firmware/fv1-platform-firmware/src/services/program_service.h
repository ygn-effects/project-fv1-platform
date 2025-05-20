#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"

class ProgramService : public Service {
  private:
    LogicalState& m_logicState;

    void syncActiveProgram();
    void publishProgramChangeEvent(const Event& t_event);

  public:
    ProgramService(LogicalState& t_lState) :
      m_logicState(t_lState) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};
