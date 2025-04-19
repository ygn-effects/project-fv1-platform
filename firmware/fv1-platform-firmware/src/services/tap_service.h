#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"

class TapService : public Service {
  private:
    LogicalState& m_logicalState;
    TapHandler m_tapHandler;

    void publishTempoEvent(const Event& t_event) const;

  public:
    TapService(LogicalState& t_lState) :
      m_logicalState(t_lState) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
};
