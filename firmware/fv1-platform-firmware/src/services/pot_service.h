#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/pot_handler.h"

class PotService : public Service {
  private:
    LogicalState& m_logicalState;
    PotHandler m_handler;

    void syncHandler(uint8_t t_potIndex);

  public:
  PotService(LogicalState& t_lState) :
      m_logicalState(t_lState) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};
