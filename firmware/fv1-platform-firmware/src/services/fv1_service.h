#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "periphs/fv1.h"
#include "logic/logical_state.h"
#include "logic/fv1_handler.h"
#include "ui/inputs.h"

class Fv1Service : public Service {
  private:
    const LogicalState& m_logicalState;

    Fv1& m_fv1;
    Fv1Handler m_handler;

  public:
    Fv1Service(const LogicalState& t_lState, Fv1& t_fv1) :
      m_logicalState(t_lState), m_fv1(t_fv1) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(const Event& t_event) const override;
};
