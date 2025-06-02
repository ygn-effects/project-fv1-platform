#include "services/menu_service.h"

MenuService::MenuService(LogicalState& t_lState, Clock& t_clock)
    : m_logicState(t_lState), m_clock(t_clock), m_mode(UiMode::kLocked),
      m_cursor(0), m_first(0), m_lastInputTime(0), m_lastPotMoveTime(0),
      m_potMenuActive(false), m_subState(SubState::kSelecting), m_editRow(0),
      m_valueBackup(0), m_lastTempoChangeTime(0), m_tempoMenuActive(false) {}

void MenuService::handleLocked(const Event& t_event) {
  if (t_event.m_type == EventType::kMenuLockLongPressed) {
    unlockUi(t_event.m_timestamp);
    m_lastInputTime = t_event.m_timestamp;

    publishView();
  }
}

void MenuService::handleUnlocked(const Event& t_event) {
  if (t_event.m_type == EventType::kBypassDisabled) { lockUi(t_event.m_timestamp); publishView(); return; }

  if (m_subState == SubState::kSelecting) {
    switch (t_event.m_type) {
      case EventType::kMenuEncoderMoved:
      case EventType::kMenuEncoderPressed:
      case EventType::kMenuLockLongPressed:
        handleSelecting(t_event);
        break;

      case EventType::kPot0Moved:
      case EventType::kPot1Moved:
      case EventType::kPot2Moved:
      case EventType::kMixPotMoved:
        handlePotsMoving(t_event);
        break;

      case EventType::kTempoChanged:
        handleTempoChange(t_event);
        break;

      default:
        break;
    }
  }
  else {
    handleEditing(t_event);
  }

  publishView();
  m_lastInputTime = t_event.m_timestamp;
}

void MenuService::lockUi(uint32_t t_now) {
  m_mode = UiMode::kLocked;
  m_subState = SubState::kSelecting;

  m_menuStack.clear();
  m_menuStack.push(&ui::LockScreenMenuPage);

  m_cursor = 0;
  m_first = 0;

  EventBus::publish({EventType::kMenuLocked, t_now, {}});
}

void MenuService::unlockUi(uint32_t t_now) {
  m_mode = UiMode::kUnlocked;

  m_menuStack.clear();
  m_logicState.m_programMode == ProgramMode::kProgram
    ? m_menuStack.push(&ui::ProgramMenuPage)
    : m_menuStack.push(&ui::PresetMenuPage);

  EventBus::publish({EventType::kMenuUnlocked, t_now, {}});
}

void MenuService::handleSelecting(const Event& t_event) {
  if (t_event.m_type == EventType::kMenuLockLongPressed) {
    lockUi(t_event.m_timestamp);
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

void MenuService::handlePotsMoving(const Event& t_event) {
  if (m_potMenuActive) { m_menuStack.pop(); }
  if (m_tempoMenuActive) { m_tempoMenuActive = false; m_menuStack.pop(); }

  switch (t_event.m_type) {
    case EventType::kPot0Moved:
      if (!m_logicState.m_activeProgram->m_isDelayEffect){
        m_menuStack.push(&ui::Pot0ValueMenuPage);
        m_potMenuActive = true;
      }
      break;

    case EventType::kPot1Moved:
      m_menuStack.push(&ui::Pot1ValueMenuPage);
      m_potMenuActive = true;
      break;

    case EventType::kPot2Moved:
      m_menuStack.push(&ui::Pot2ValueMenuPage);
      m_potMenuActive = true;
      break;

    case EventType::kMixPotMoved:
      m_menuStack.push(&ui::MixPotValueMenuPage);
      m_potMenuActive = true;
      break;

    default:
      break;
  }

  m_lastPotMoveTime = t_event.m_timestamp;
}

void MenuService::handleTempoChange(const Event& t_event) {
  if (m_potMenuActive ) { m_potMenuActive = false; m_menuStack.pop(); }

  if (!m_tempoMenuActive) {
    m_menuStack.push(&ui::TempoMenuPage);
  }

  m_tempoMenuActive = true;
  m_lastTempoChangeTime = t_event.m_timestamp;
  m_lastInputTime = t_event.m_timestamp;
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
  else if (m_cursor >= m_first + ui::MenuConstants::c_visibleItemsPerPage) {
    m_first = m_cursor - (ui::MenuConstants::c_visibleItemsPerPage - 1);
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

  if (page.m_layout == ui::MenuLayout::kList) {
    uint8_t visIndex = 0;
    uint8_t sliceCount = 0;

    for (uint8_t i = 0; i < page.m_count; ++i) {
      const ui::MenuItem& item = page.m_items[i];

      if (! item.m_visible(&m_logicState))
        continue;

      if (visIndex < m_first) {
        ++visIndex;
        continue;
      }

      if (sliceCount < ui::MenuConstants::c_visibleItemsPerPage) {
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
  else if (page.m_layout == ui::MenuLayout::kTwoColumns) {
    uint8_t sliceCount = 0;

    for (uint8_t i = 0; i < page.m_count; i++) {
      if (page.m_count > ui::MenuConstants::c_visibleItemsPerTwoColumns) {
        break;
      }

      const ui::MenuItem& item = page.m_items[i];

      if (! item.m_visible(&m_logicState)) {
        continue;
      }

      m_view.m_items[sliceCount] = &page.m_items[i];
      sliceCount++;
    }

    m_view.m_header = page.m_header;
    m_view.m_count = sliceCount;
    m_view.m_layout = page.m_layout;
    m_view.m_selected = 0;
    m_view.m_editing = false;
  }
  else if (page.m_layout == ui::MenuLayout::kLabelValue) {
    uint8_t sliceCount = 0;

    for (uint8_t i = 0; i < page.m_count; i++) {
      const ui::MenuItem& item = page.m_items[i];

      if (! item.m_visible(&m_logicState)) {
        continue;
      }

      m_view.m_items[sliceCount] = &page.m_items[i];
      sliceCount++;
    }

    m_view.m_header = page.m_header;
    m_view.m_count = sliceCount;
    m_view.m_layout = page.m_layout;
    m_view.m_selected = 0;
    m_view.m_editing = false;
  }
  else {

  }

  Event e;
  e.m_type = EventType::kMenuViewUpdated;
  e.m_timestamp = m_clock.now();
  e.m_data.view = &m_view;
  EventBus::publish(e);
}

void MenuService::init() {
  m_menuStack.push(&ui::LockScreenMenuPage);
  publishView();
}

void MenuService::handleEvent(const Event& t_event) {
  if (t_event.m_type == EventType::kProgramChanged) { publishView(); return; }

  switch (m_mode) {
    case UiMode::kLocked:
      handleLocked(t_event);
      return;

    case UiMode::kUnlocked:
      handleUnlocked(t_event);
      return;

    default:
      break;
  }
}

void MenuService::update() {
  if (m_mode == UiMode::kLocked) return;

  uint32_t now = m_clock.now();
  if ((now - m_lastInputTime) > ui::MenuConstants::c_menuTimeout) {
    lockUi(now);
    publishView();
  }

  if (m_potMenuActive) {
    if ((now - m_lastPotMoveTime) > ui::MenuConstants::c_potMenuTimeout) {
      m_potMenuActive = false;

      m_menuStack.pop();
      publishView();
    }
  }

  if (m_tempoMenuActive) {
    if ((now - m_lastTempoChangeTime) > ui::MenuConstants::c_tempoMenuTimeout) {
      m_tempoMenuActive = false;

      m_menuStack.pop();
      publishView();
    }
  }
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

const ui::MenuView* MenuService::getMenuView() const {
  return &m_view;
}

bool MenuService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return (t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kEncoderEvent)
      || (t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kLockEvent)
      || (t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kPotEvent)
      || (t_category == EventCategory::kTempoEvent && t_subCategory == EventSubCategory::kTempoChangedEvent)
      || (t_category == EventCategory::kBypassEvent && t_subCategory == EventSubCategory::kBypassDisabledEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramChangedEvent);
}
