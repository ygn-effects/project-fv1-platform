#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "ui/menu_model.h"
#include "ui/menu_stack.h"

namespace UiConstants {
  static constexpr uint32_t cMenuTimeout = 30000u;
}

enum class UiMode : uint8_t {
  kLocked,
  kProgramEdit,
  kPresetEdit
};

enum class SubState : uint8_t {
  kSelecting,
  kEditing
};

struct EditContext {
  ParamId m_paramId;
  uint16_t m_currentValue;
};

class MenuService : public Service {
  private:
    LogicalState& m_logicState;
    Menu m_rootMenu{};
    MenuItem m_rootMenubuf[10];
    const Menu* p_currentMenu;
    UiMode m_mode;
    SubState m_subState;
    MenuStack<6> m_stack;
    uint8_t m_cursor;
    EditContext m_edit;
    uint32_t m_lastInputTime;

    void rebuildView();
    void rebuildRootMenu();

    void lockUi(const Event& t_event);
    void unlockUi(const Event& t_event);

    void handleLocked(const Event& t_event);
    void handleUnlocked(const Event& t_event);
    void handleSelecting(const Event& t_event);
    void handleEditing(const Event& t_event);
    void moveCursor(int8_t t_delta);

    void pushMenu(const Menu* t_menu);
    void popMenu();

    const ProgramParameter& currentParam();
    void beginEdit();
    void commitEdit();

  public:
    MenuService(LogicalState& t_lState);

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;

    // Debug methods
    const Menu* currentMenu() const;
    UiMode getUiMode() const;
    uint8_t getCursor() const;
    SubState getSubState() const;
    EditContext getEditContext() const;
};
