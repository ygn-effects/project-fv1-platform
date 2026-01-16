#pragma once

#include <stdint.h>
#include "logic/logical_state.h"
#include "ui/inputs.h"
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

struct MenuHandler {
  private:
    uint8_t getVisibleItemCount(const LogicalState& t_state) const;
    uint8_t visibleToRealIndex(uint8_t t_visibleIndex, const LogicalState& t_state) const;

    void buildListView(const ui::MenuPage& t_page, const LogicalState& t_state);
    void buildTwoColumnsView(const ui::MenuPage& t_page, const LogicalState& t_state);
    void buildLabelValueView(const ui::MenuPage& t_page, const LogicalState& t_state);

  public:
    MenuStack<4> m_menuStack;
    ui::MenuView m_view;
    UiMode m_mode{UiMode::kLocked};
    SubState m_subState{SubState::kSelecting};
    uint8_t m_cursor{0};
    uint8_t m_first{0};

    void init();

    void lock();
    void unlock(const LogicalState& t_state);

    void moveCursor(int8_t t_delta, const LogicalState& t_state);

    bool handleSelect(const LogicalState& t_state);

    void beginEditing();
    void endEditing();
    void applyEditDelta(int8_t t_delta, const LogicalState& t_state);

    void pushPotOverlay(uint8_t t_potId, const LogicalState& t_state);
    void pushTempoOverlay();
    void popOverlay();

    void buildView(const LogicalState& t_state);
};
