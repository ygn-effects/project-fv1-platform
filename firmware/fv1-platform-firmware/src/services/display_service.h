#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "hal/display.h"
#include "ui/menu_renderer.h"

class DisplayService : public Service {
  private:
    const LogicalState& m_logicalState;
    DisplayDriver& m_driver;
    ui::MenuRenderer m_renderer;

  public:
    DisplayService(const LogicalState& t_lState, DisplayDriver& t_driver);

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};
