#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "periphs/display.h"
#include "ui/menu_renderer.h"

class DisplayService : public Service {
  private:
    const LogicalState& m_logicalState;
    Display& m_display;
    ui::MenuRenderer m_renderer;

  public:
    DisplayService(const LogicalState& t_lState, Display& t_display);

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};
