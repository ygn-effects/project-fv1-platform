#pragma once

#include <stdint.h>
#include "core/event_bus.h"
#include "core/service.h"
#include "periphs/bypass.h"
#include "logic/logical_state.h"
#include "services/midi_service.h"

class BypassService : public Service {
  private:
    LogicalState& m_logicalState;
    Bypass& m_bypass;

    void publishSaveBypassEvent(const Event& t_event);

  public:
    BypassService(LogicalState& t_lState, Bypass& t_bypass) :
      m_logicalState(t_lState), m_bypass(t_bypass) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};
