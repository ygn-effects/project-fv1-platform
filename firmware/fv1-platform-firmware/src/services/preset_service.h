#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/preset_handler.h"
#include "utils/utils.h"

class PresetService : public Service {
  private:
    LogicalState& m_logicalState;
    PresetHandler m_handler;

    void applyPreset();
    void publishLoadBankEvent(const Event& t_event);
    void publishSaveCurrentPresetBank(const Event& t_event);
    void publishSaveCurrentPreset(const Event& t_event);

  public:
    PresetService(LogicalState& t_lState);

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};
