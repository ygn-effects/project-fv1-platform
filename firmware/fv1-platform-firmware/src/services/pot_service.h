#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/pot_handler.h"

class PotService : public Service {
  private:
    LogicalState& m_logicalState;
    PotHandler m_handler;

    void syncHandler(uint8_t t_potIndex);
    void publishSavePotEvent(uint8_t t_potIndex);

    void handlePhysicalEvent(const Event& t_event);
    void handleMenuEvent(const Event& t_event);
    void handleMidiEvent(const Event& t_event);
    void handleExprEvent(const Event& t_event);

    void handleMenuPotStateToggleEvent(const Event& t_event);
    void handleMenuPotMinValueMove(const Event& t_event);
    void handleMenuPotMaxValueMove(const Event& t_event);

  public:
  PotService(LogicalState& t_lState) :
      m_logicalState(t_lState) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;
};
