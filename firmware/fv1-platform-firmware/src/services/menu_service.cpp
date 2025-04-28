#include "services/menu_service.h"

MenuService::MenuService(LogicalState& t_lState)
    : m_logicState(t_lState), m_cursor(0), m_first(0),
      m_subState(SubState::kSelecting), m_editRow(0),
      m_valueBackup(0), m_mode(UiMode::kLocked) {}

void MenuService::handleLocked(const Event& t_event) {
  if (t_event.m_type == EventType::kMenuEncoderLongPressed) {
    unlockUi(t_event);

    m_lastInputTime = t_event.m_timestamp;
  }
}

void MenuService::handleUnlocked(const Event& t_event) {
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

void MenuService::lockUi(const Event& t_event) {
  m_mode = UiMode::kLocked;

  m_menuStack.clear();
  m_menuStack.push(&ui::LockScreenMenu);

  m_cursor = 0;
  m_first = 0;

  EventBus::publish({EventType::kMenuLocked, t_event.m_timestamp, {}});
}

void MenuService::unlockUi(const Event& t_event) {
  m_mode = UiMode::kUnlocked;

  m_menuStack.clear();
  m_logicState.m_programMode == ProgramMode::kProgram
    ? m_menuStack.push(&ui::ProgramMenuPage)
    : m_menuStack.push(&ui::PresetMenuPage);

  EventBus::publish({EventType::kMenuUnlocked, t_event.m_timestamp, {}});
}

void MenuService::handleSelecting(const Event& t_event) {
  if (t_event.m_type == EventType::kMenuEncoderLongPressed) {
    lockUi(t_event);
    return;
  }

  switch (t_event.m_type) {
    case EventType::kMenuEncoderMoved:
      moveCursor(t_event.m_data.delta);
      break;

    case EventType::kMenuEncoderPressed:
      beginEditing();
      break;
  }
}

void MenuService::handleEditing(const Event& t_event) {
  if (t_event.m_type == EventType::kMenuEncoderMoved) {
    getcurrentMenuItem().m_onMove(t_event.m_data.delta);
  }
  else if (t_event.m_type == EventType::kMenuEncoderPressed) {
    endEditing();
  }
}

void MenuService::moveCursor(int8_t t_delta) {
  uint8_t itemsCount = getcurrentMenuPage().m_count;
  int8_t index = static_cast<int16_t>(m_cursor) + t_delta;

  if (index < 0) index += itemsCount;
  if (index >= itemsCount) index -= itemsCount;

  m_cursor = static_cast<uint8_t>(index);

  if (m_cursor < m_first) {
      m_first = m_cursor;
  }
  else if (m_cursor >= m_first + MenuConstants::c_visibleItemsPerPage) {
      m_first = m_cursor - (MenuConstants::c_visibleItemsPerPage - 1);
  }
}

void MenuService::beginEditing() {
  m_editRow = m_first + m_cursor;
  m_subState = SubState::kEditing;
}

void MenuService::endEditing() {
  m_editRow = 0;
  m_subState = SubState::kSelecting;
}

void MenuService::init() {
  m_menuStack.push(&ui::LockScreenMenu);
}

void MenuService::handleEvent(const Event& t_event) {
  switch (m_mode) {
    case UiMode::kLocked:
      handleLocked(t_event);
      break;

    case UiMode::kUnlocked:
      handleUnlocked(t_event);
      break;
  }
}

void MenuService::update() {
  if (m_mode == UiMode::kLocked) return;

  uint32_t now = 31000; // Dummy value for testing
  if ((now - m_lastInputTime) > MenuConstants::c_menuTimeout) {
    lockUi({EventType::kMenuEncoderMoved, 500, {.delta = 1}});
  }
}

const ui::MenuPage& MenuService::getcurrentMenuPage() const {
  return *m_menuStack.top();
}

const ui::MenuItem& MenuService::getcurrentMenuItem() const {
  return m_menuStack.top()->m_items[m_cursor];
}

const SubState MenuService::getsubState() const {
  return m_subState;
}
