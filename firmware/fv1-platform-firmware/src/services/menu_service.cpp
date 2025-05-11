#include "services/menu_service.h"

MenuService::MenuService(LogicalState& t_lState)
    : m_logicState(t_lState), m_mode(UiMode::kLocked), m_cursor(0), m_first(0),
      m_lastInputTime(0), m_subState(SubState::kSelecting), m_editRow(0),
      m_valueBackup(0) {}

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

    case EventType::kMenuEncoderPressed: {
      ui::MenuItem item = getcurrentMenuItem();

      if (item.m_subMenu) {
        m_cursor = 0;
        m_first = 0;
        m_menuStack.push(item.m_subMenu);
      }
      else if (item.m_onClick) {
        item.m_onClick();
      }
      else if (item.m_onMove) {
        beginEditing();
      }
      break;
    }

    default:
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
  uint8_t visibleCount = getVisibleItemCount();
  int8_t index = static_cast<int8_t>(m_cursor) + t_delta;

  if (index < 0) index += visibleCount;
  if (index >= visibleCount) index -= visibleCount;

  m_cursor = static_cast<uint8_t>(index);

  if (m_cursor < m_first) {
    m_first = m_cursor;
  }
  else if (m_cursor >= m_first + MenuConstants::c_visibleItemsPerPage) {
    m_first = m_cursor - (MenuConstants::c_visibleItemsPerPage - 1);
  }
}

uint8_t MenuService::visibleToRealIndex(uint8_t visibleIndex) const {
  const auto& page = getcurrentMenuPage();
  uint8_t visCount = 0;

  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&m_logicState)) {
      if (visCount == visibleIndex) {
        return i;
      }
      ++visCount;
    }
  }

  return 0;
}

void MenuService::beginEditing() {
  m_editRow = m_first + m_cursor;
  m_subState = SubState::kEditing;
}

void MenuService::endEditing() {
  m_editRow = 0;
  m_subState = SubState::kSelecting;
}

uint8_t MenuService::getVisibleItemCount() const {
  const ui::MenuPage& page = getcurrentMenuPage();
  uint8_t count = 0;

  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&m_logicState)) {
      ++count;
    }
  }

  return count;
}

void MenuService::publishView() {
  const ui::MenuPage& page = getcurrentMenuPage();

  uint8_t visIndex = 0;
  uint8_t sliceCount = 0;

  for (uint8_t i = 0; i < page.m_count; ++i) {
    const ui::MenuItem& item = page.m_items[i];

    if (!item.m_visible(&m_logicState))
      continue;

    if (visIndex < m_first) {
      ++visIndex;
      continue;
    }

    if (sliceCount < MenuConstants::c_visibleItemsPerPage) {
      m_view.m_items[sliceCount] = &item;

      if (visIndex == m_cursor) {
        m_view.m_selected = sliceCount;
      }

      ++sliceCount;
    }

    ++visIndex;
  }

  m_view.m_count = sliceCount;
  m_view.m_header = page.m_header;
  m_view.m_layout = page.m_layout;
  m_subState == SubState::kEditing
    ? m_view.m_editing = true
    : m_view.m_editing = false;
}

void MenuService::init() {
  m_menuStack.push(&ui::LockScreenMenu);
  publishView();
}

void MenuService::handleEvent(const Event& t_event) {
  switch (m_mode) {
    case UiMode::kLocked:
      handleLocked(t_event);
      break;

    case UiMode::kUnlocked:
      handleUnlocked(t_event);
      break;

    default:
      break;
  }
}

void MenuService::update() {
  if (m_mode == UiMode::kLocked) return;

  uint32_t now = 31000; // Dummy value for testing
  if ((now - m_lastInputTime) > MenuConstants::c_menuTimeout) {
    lockUi({EventType::kMenuEncoderMoved, 500, {}});
  }

  // For tests only
  publishView();
}

const ui::MenuPage& MenuService::getcurrentMenuPage() const {
  return *m_menuStack.top();
}

const ui::MenuItem& MenuService::getcurrentMenuItem() const {
  return m_menuStack.top()->m_items[visibleToRealIndex(m_cursor)];
}

const SubState MenuService::getsubState() const {
  return m_subState;
}

const MenuView* MenuService::getMenuView() const {
  return &m_view;
}

bool MenuService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kEncoderEvent;
}
