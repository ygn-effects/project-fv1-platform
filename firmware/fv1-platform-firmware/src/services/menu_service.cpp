#include "services/menu_service.h"

void MenuService::rebuildView() {

}

void MenuService::rebuildRootMenu() {
  uint8_t index = 0;
  const Program* program = m_logicState.m_activeProgram;

  m_rootMenubuf[index++] = {program->m_name, ItemKind::kParam, ParamId::kProgramName, nullptr, nullptr};

  if (program->m_isDelayEffect) {
    m_rootMenubuf[index++] = {"Tempo", ItemKind::kParam, ParamId::kTempo, nullptr, nullptr};
  }
  else {
    m_rootMenubuf[index++] = {program->m_params[0].m_label, ItemKind::kParam, ParamId::kPot0Value, nullptr, nullptr};
  }

  m_rootMenubuf[index++] = {program->m_params[1].m_label, ItemKind::kParam, ParamId::kPot0Value, nullptr, nullptr};
  m_rootMenubuf[index++] = {program->m_params[2].m_label, ItemKind::kParam, ParamId::kPot0Value, nullptr, nullptr};
  m_rootMenubuf[index++] = {program->m_params[3].m_label, ItemKind::kParam, ParamId::kPot0Value, nullptr, nullptr};

  if (program->m_supportsExpr) {
    const char* state = m_logicState.m_exprParams[m_logicState.m_currentProgram].m_state == ExprState::kActive
                          ? "Expr On"
                          : "Expr Off";

    m_rootMenubuf[index++] = {state, ItemKind::kSubMenu, ParamId::kNone, nullptr, nullptr};
  }

  m_rootMenu.m_items = m_rootMenubuf;
  m_rootMenu.m_count = index + 1;

  m_stack.clear();
  m_stack.push(&m_rootMenu);
}

void MenuService::lockUi(const Event& t_event) {
  m_mode = UiMode::kLocked;
  m_stack = {};
  m_stack.push(&m_rootMenu);
  m_cursor = 0;

  EventBus::publish({EventType::kMenuLocked, t_event.m_timestamp, {}});
}

void MenuService::unlockUi(const Event& t_event) {
  m_mode = m_logicState.m_programMode == ProgramMode::kProgram
            ? UiMode::kProgramEdit
            : UiMode::kPresetEdit;

  EventBus::publish({EventType::kMenuUnlocked, t_event.m_timestamp, {}});
}

void MenuService::handleLocked(const Event& t_event) {
  if (t_event.m_type == EventType::kMenuEncoderLongPressed) {
    unlockUi(t_event);
    m_lastInputTime = t_event.m_timestamp;
  }
}

void MenuService::handleUnlocked(const Event& t_event) {
  if (t_event.m_type == EventType::kMenuEncoderLongPressed) {
    lockUi(t_event);
    return;
  }

  switch (m_subState) {
    case SubState::kSelecting:
      handleSelecting(t_event);
      break;

    case SubState::kEditing:
      handleEditing(t_event);
      break;
  }

  m_lastInputTime = t_event.m_timestamp;
}

void MenuService::handleSelecting(const Event& t_event) {
  if (t_event.m_type == EventType::kMenuEncoderMoved) {
    moveCursor(t_event.m_data.delta);

    m_lastInputTime = t_event.m_timestamp;
  }
}

void MenuService::handleEditing(const Event& t_event) {

}

void MenuService::moveCursor(int8_t t_delta) {
  int16_t cursor = static_cast<int8_t>(m_cursor) + t_delta;
  if (cursor < 0) cursor = p_currentMenu->m_count - 1;
  if (cursor >= p_currentMenu->m_count) cursor = 0;

  m_cursor = static_cast<uint8_t>(cursor);
}

MenuService::MenuService(LogicalState& t_lState)
    : m_logicState(t_lState), m_mode(UiMode::kLocked), m_subState(SubState::kSelecting),
      m_cursor(0), m_lastInputTime(0) {
  m_stack.push(nullptr);
  p_currentMenu = m_stack.top();
}

void MenuService::init() {
  rebuildRootMenu();

  p_currentMenu = m_stack.top();
}

void MenuService::handleEvent(const Event& t_event) {
  switch (m_mode) {
  case UiMode::kLocked:
    handleLocked(t_event);
    break;

  case UiMode::kPresetEdit:
  case UiMode::kProgramEdit:
    handleUnlocked(t_event);

  default:
    break;
  }
}

void MenuService::update() {
  if (m_mode == UiMode::kLocked) return;

  uint32_t now = 31000; // Dummy value for testing
  if ((now - m_lastInputTime) > UiConstants::cMenuTimeout) {
    lockUi({EventType::kMenuEncoderMoved, 500, {.delta = 1}});
  }
}

const Menu* MenuService::currentMenu() const {
  return p_currentMenu;
}

UiMode MenuService::getUiMode() const {
  return m_mode;
}

uint8_t MenuService::getCursor() const {
  return m_cursor;
}
