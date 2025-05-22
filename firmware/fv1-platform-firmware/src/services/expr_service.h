#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/expr_handler.h"

class ExprService : public Service {
  private:
    LogicalState& m_logicState;
    ExprHandler m_exprHandler;

    void syncHandler();
    void publishSaveExprEvent(const Event& t_event);

  public:
    ExprService(LogicalState& t_lState) :
      m_logicState(t_lState) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};
