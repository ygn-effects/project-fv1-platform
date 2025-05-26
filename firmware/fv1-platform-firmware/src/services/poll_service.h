#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "ui/inputs.h"

class PollService : public Service {
  private:
    const LogicalState& m_logicalState;

  public:
    PollService(const LogicalState& t_lState);

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};
