#include "services/menu_service.h"

void MenuService::rebuildView() {

}

void MenuService::rebuildRootMenu() {
  uint8_t index = 0;
  const Program* program = m_logicState.m_activeProgram;

  m_rootMenubuf[index++] = {"Program", ItemKind::kParam, ParamId::kProgramName, m_logicState.m_currentProgram, ParamUnit::kNone, nullptr, nullptr};

  if (program->m_isDelayEffect) {
    m_rootMenubuf[index++] = {"Tempo", ItemKind::kParam, ParamId::kTempo, m_logicState.m_tempo, ParamUnit::kMs, nullptr, nullptr};

    if (m_logicState.m_divState == DivState::kEnabled) {
      m_rootMenubuf[index++] = {"Tempo Div", ItemKind::kParam, ParamId::kDivValue, static_cast<uint8_t>(m_logicState.m_divValue), ParamUnit::kMs, nullptr, nullptr};
    }
  }
  else {
    m_rootMenubuf[index++] = {program->m_params[0].m_label, ItemKind::kParam, ParamId::kPot0Value, 0, ParamUnit::kPercent, nullptr, nullptr};
  }

  m_rootMenubuf[index++] = {program->m_params[1].m_label, ItemKind::kParam, ParamId::kPot0Value, 0, ParamUnit::kPercent, nullptr, nullptr};
  m_rootMenubuf[index++] = {program->m_params[2].m_label, ItemKind::kParam, ParamId::kPot1Value, 0, ParamUnit::kPercent, nullptr, nullptr};
  m_rootMenubuf[index++] = {program->m_params[3].m_label, ItemKind::kParam, ParamId::kPot2Value, 0, ParamUnit::kPercent, nullptr, nullptr};

  if (program->m_supportsExpr) {
    m_rootMenubuf[index++] = {"Expr state", ItemKind::kSubMenu, ParamId::kNone, static_cast<uint8_t>(m_logicState.m_exprParams[m_logicState.m_currentPreset].m_state), ParamUnit::kPercent, nullptr, nullptr};
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
  else if (t_event.m_type == EventType::kMenuEncoderPressed) {
    const MenuItem& item = p_currentMenu->m_items[m_cursor];

    switch (item.m_kind) {
      case ItemKind::kParam:
        beginEdit();
        break;

      default:
        break;
    }

    m_lastInputTime = t_event.m_timestamp;
  }
}

void MenuService::handleEditing(const Event& t_event) {
  if (t_event.m_type == EventType::kMenuEncoderMoved) {
    const ProgramParameter& param = currentParam();

    int16_t value = static_cast<int16_t>(m_edit.m_currentValue) + (static_cast<int16_t>(t_event.m_data.delta) * (static_cast<int16_t>(param.m_fineStep ? param.m_fineStep : 1)));
    m_edit.m_currentValue = value;
    m_lastInputTime = t_event.m_timestamp;
  }
  else if (t_event.m_type == EventType::kMenuEncoderPressed) {
    commitEdit();
  }
}

void MenuService::moveCursor(int8_t t_delta) {
  int16_t cursor = static_cast<int8_t>(m_cursor) + t_delta;
  if (cursor < 0) cursor = p_currentMenu->m_count - 1;
  if (cursor >= p_currentMenu->m_count) cursor = 0;

  m_cursor = static_cast<uint8_t>(cursor);
}

void MenuService::pushMenu(const Menu* t_menu) {

}

void MenuService::popMenu() {

}

const ProgramParameter& MenuService::currentParam() {
  switch (p_currentMenu->m_items[m_cursor].m_paramId) {
    case ParamId::kPot0Value:
    case ParamId::kTempo:
      return m_logicState.m_activeProgram->m_params[0];

    case ParamId::kPot1Value:
      return m_logicState.m_activeProgram->m_params[1];

    case ParamId::kPot2Value:
      return m_logicState.m_activeProgram->m_params[2];

    case ParamId::kMixPotValue:
      return m_logicState.m_activeProgram->m_params[3];

    default:
      return m_logicState.m_activeProgram->m_params[0];
  }
}

void MenuService::beginEdit() {
  m_edit.m_currentValue = p_currentMenu->m_items[m_cursor].m_value;
  m_edit.m_paramId = p_currentMenu->m_items[m_cursor].m_paramId;

  m_subState = SubState::kEditing;
}

void MenuService::commitEdit() {
  m_subState = SubState::kSelecting;
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

SubState MenuService::getSubState() const {
  return m_subState;
}

EditContext MenuService::getEditContext() const {
  return m_edit;
}
