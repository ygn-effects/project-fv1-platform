#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/fsm.h"

class FsmService : public Service {
  private:
    LogicalState& m_logicalState;
    AppState m_state;

    void transitionTo(AppState t_state, uint32_t t_timestamp);

  public:
    FsmService(LogicalState& t_lState) :
      m_logicalState(t_lState), m_state(AppState::kBoot) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};
