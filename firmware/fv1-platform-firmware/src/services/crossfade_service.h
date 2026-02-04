#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/crossfade_handler.h"
#include "periphs/dac.h"
#include "ui/inputs.h"

class CrossfadeService : public Service {
  private:
    const LogicalState& m_logicalState;
    CrossfadeHandler m_handler;
    Dac& m_dacWet;
    Dac& m_dacDry;

    void syncHandler();
    void applyMix(uint16_t t_mixValue);

  public:
    CrossfadeService(LogicalState& t_lState, Dac& t_dacD, Dac& t_dacW) :
      m_logicalState(t_lState),
      m_dacDry(t_dacD),
      m_dacWet(t_dacW) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(const Event& t_event) const override;

    CrossfadeHandler& getHandler() { return m_handler; }
};
