#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/fsm.h"
#include "ui/inputs.h"

class FsmService : public Service {
  private:
    LogicalState& m_logicalState;
    AppState m_state;

    void transitionTo(AppState t_state);

    bool isPressed(const Event& t_event, const SwitchId t_id) const;
    bool isLongPressed(const Event& t_event, const SwitchId t_id) const;
    bool isDeltaChanged(const Event& t_event) const;
    bool isPotValueChanged(const Event& t_event) const;
    bool isExprValueChanged(const Event& t_event) const;
    bool isExprConnected(const Event& t_event) const;
    bool isExprDisconnected(const Event& t_event) const;
    bool isBypassToggled(const Event& t_event) const;
    bool isMenuUnlocked(const Event& t_event) const;
    bool isMenuLocked(const Event& t_event) const;
    bool isProgramModeToggled(const Event& t_event) const;

    void rePublishPhysicalEvent(const Event& t_event) const;

  public:
    FsmService(LogicalState& t_lState) :
      m_logicalState(t_lState), m_state(AppState::kBoot) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(const Event& t_event) const override;

    // Debug
    AppState getAppState() const;
    void setAppState(const AppState t_state);
};
