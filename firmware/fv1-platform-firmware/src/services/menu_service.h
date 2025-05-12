#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "ui/menu_model.h"
#include "ui/menu_stack.h"

enum class UiMode : uint8_t {
  kLocked,
  kUnlocked
};

enum class SubState : uint8_t {
  kSelecting,
  kEditing
};

class MenuService : public Service {
  private:
    LogicalState& m_logicState;
    MenuStack<4> m_menuStack;
    ui::MenuView m_view;
    UiMode m_mode;
    uint8_t m_cursor;
    uint8_t m_first;
    uint32_t m_lastInputTime;
    SubState m_subState;
    uint8_t m_editRow;
    uint16_t m_valueBackup;

    void handleLocked(const Event& t_event);
    void handleUnlocked(const Event& t_event);

    void lockUi(const Event& t_event);
    void unlockUi(const Event& t_event);

    void handleSelecting(const Event& t_event);
    void handleEditing(const Event& t_event);

    void moveCursor(int8_t t_delta);

    void beginEditing();
    void endEditing();

    uint8_t getVisibleItemCount() const;
    uint8_t visibleToRealIndex(uint8_t visibleIndex) const;

    void publishView();

  public:
    MenuService(LogicalState& t_lState);

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;

    // Debug
    const ui::MenuPage& getcurrentMenuPage() const;
    const ui::MenuItem& getcurrentMenuItem() const;
    const SubState getsubState() const;
    const ui::MenuView* getMenuView() const;
    const uint8_t getCursor() const { return m_cursor; }
};
