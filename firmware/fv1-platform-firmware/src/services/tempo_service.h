#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/tempo_handler.h"
#include "periphs/adjustable.h"
#include "periphs/clock.h"
#include "utils/utils.h"

class TempoService : public Service {
  private:
    LogicalState& m_logicState;
    Adjustable& m_tempoLed;
    Clock& m_clock;
    TempoHandler m_handler;

    void syncHandler();

    void publishTempoEvent(uint16_t t_interval) const;
    void publishSaveTempoEvent(uint16_t t_interval) const;

  public:
    TempoService(LogicalState& t_lState, Adjustable& t_led, Clock& t_clock);

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};
