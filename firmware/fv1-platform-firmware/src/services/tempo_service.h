#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "utils/utils.h"

enum class TempoSource : uint8_t {
  kTap,
  kPot,
  kMenu
};

class TempoService : public Service {
  private:
    LogicalState& m_logicState;
    uint16_t m_interval;
    uint16_t m_minInterval;
    uint16_t m_maxInterval;
    TempoSource m_source;

    void publishTempoEvent(const Event& t_event) const;

  public:
    TempoService(LogicalState& t_lState);

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};
