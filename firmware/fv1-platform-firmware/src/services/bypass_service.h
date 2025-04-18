#pragma once

#include <stdint.h>
#include "core/event_bus.h"
#include "core/service.h"
#include "logic/logical_state.h"

class BypassService : public Service {
  private:
    LogicalState& m_logicalState;

  public:
    BypassService(LogicalState& t_lState) :
      m_logicalState(t_lState) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
};
