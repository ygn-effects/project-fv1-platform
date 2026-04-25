#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/menu_handler.h"
#include "periphs/clock.h"
#include "periphs/toggleable.h"

/**
 * @brief Manages menu navigation, UI state, and user input routing.
 *
 * Delegates business logic to MenuHandler. Owns time-based behavior
 * (timeouts, overlay dismissal) and event publishing.
 *
 * Listens for: Physical Encoder, Physical Switch (MenuLock), Physical Pot,
 *              Logic Tempo, Logic Bypass, Logic Program.
 *
 * Publishes: UI Menu Locked, UI Menu Unlocked, UI Menu Updated.
 */
class MenuService : public Service {
  private:
    LogicalState& m_logicState;
    Toggleable& m_menuLockLed;
    Clock& m_clock;
    MenuHandler m_handler;

    bool m_previousMenuStateUnlocked{false};
    uint32_t m_lastInputTime{0};
    uint32_t m_lastPotMoveTime{0};
    uint32_t m_lastTempoChangeTime{0};
    bool m_potMenuActive{false};
    bool m_tempoMenuActive{false};
    bool m_savePresetMenuActive{false};

    void resetOverlayState();

    void publishUIMenuLockedEvent() const;
    void publishUIMenuUnlockedEvent() const;
    void publishUIMenuUpdatedEvent();
    void publishViewUpdate();

    void handleLocked(const Event& t_event);
    void handleUnlocked(const Event& t_event);
    void handleSelecting(const Event& t_event);
    void handleEditing(const Event& t_event);
    void handlePotsMoving(const Event& t_event);
    void handleTempoChange(const Event& t_event);
    void handlePresetSaving(const Event& t_event);

  public:
    MenuService(LogicalState& t_lState, Toggleable& t_led, Clock& t_clock) :
      m_logicState(t_lState),
      m_menuLockLed(t_led),
      m_clock(t_clock) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(const Event& t_event) const override;

    // Tests
    MenuHandler* getMenuHandler() { return &m_handler; }
};
