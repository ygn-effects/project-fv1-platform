#include "services/menu_service.h"

MenuService::MenuService(LogicalState& t_lState, Clock& t_clock)
    : m_logicState(t_lState), m_clock(t_clock), m_mode(UiMode::kLocked),
      m_cursor(0), m_first(0), m_lastInputTime(0), m_lastPotMoveTime(0),
      m_potMenuActive(false), m_subState(SubState::kSelecting), m_editRow(0),
      m_valueBackup(0), m_lastTempoChangeTime(0), m_tempoMenuActive(false) {}

void MenuService::publishUIMenuLockedEvent() const {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMenu;
  e.m_action = EventAction::kLocked;

  EventBus::publish(e);
}

void MenuService::publishUIMenuUnlockedEvent() const {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMenu;
  e.m_action = EventAction::kUnlocked;

  EventBus::publish(e);
}

void MenuService::publishUIMenuUpdatedEvent() {
  Event e;
  e.m_domain = EventDomain::kUI;
  e.m_subject = EventSubject::kMenu;
  e.m_action = EventAction::kUpdated;
  e.m_data.ptr = static_cast<void*>(&m_view);

  EventBus::publish(e);
}

void MenuService::handleLocked(const Event& t_event) {
  if (t_event.m_action == EventAction::kLongPressed
      && t_event.matchesId(SwitchId::kMenuLock)) {
    unlockUi(t_event.m_timestamp);
    m_lastInputTime = t_event.m_timestamp;

    publishView();
  }
}

void MenuService::handleUnlocked(const Event& t_event) {
  if (m_subState == SubState::kSelecting) {
    if (t_event.m_domain == EventDomain::kPhysical) {
      if (t_event.m_subject == EventSubject::kEncoder
          && t_event.matchesId(EncoderId::kMenuEncoder)) {
        handleSelecting(t_event);
      }

      else if (t_event.m_subject == EventSubject::kPot
          && t_event.m_action == EventAction::kValueChanged) {
        handlePotsMoving(t_event);
      }

      else if (t_event.m_subject == EventSubject::kSwitch
          && t_event.m_action == EventAction::kLongPressed
          && t_event.matchesId(SwitchId::kMenuLock)) {
        lockUi(t_event.m_timestamp);
      }

      publishView();
      m_lastInputTime = t_event.m_timestamp;

      return;
    }

    if (t_event.m_domain == EventDomain::kLogic) {
      if (t_event.m_subject == EventSubject::kTempo
          && t_event.m_action == EventAction::kValueChanged) {
        handleTempoChange(t_event);
        publishView();
        m_lastInputTime = t_event.m_timestamp;

        return;
      }

      if (t_event.m_subject == EventSubject::kBypass
          && t_event.m_action == EventAction::kToggled) {
        lockUi(t_event.m_timestamp);
        publishView();
        return;
      }
    }
  }
  else {
    if (t_event.m_subject == EventSubject::kEncoder
        && t_event.matchesId(EncoderId::kMenuEncoder)) {
      handleEditing(t_event);
    }
  }
}

void MenuService::lockUi(uint32_t t_now) {
  m_mode = UiMode::kLocked;
  m_subState = SubState::kSelecting;

  m_menuStack.clear();
  m_menuStack.push(&ui::LockScreenMenuPage);

  m_cursor = 0;
  m_first = 0;

  publishUIMenuLockedEvent();
}

void MenuService::unlockUi(uint32_t t_now) {
  m_mode = UiMode::kUnlocked;

  m_menuStack.clear();
  m_logicState.m_programMode == ProgramMode::kProgram
    ? m_menuStack.push(&ui::ProgramMenuPage)
    : m_menuStack.push(&ui::PresetMenuPage);

  publishUIMenuUnlockedEvent();
}

void MenuService::handleSelecting(const Event& t_event) {
  switch (t_event.m_action) {
    case EventAction::kDeltaChanged:
      moveCursor(t_event.m_data.delta);
      break;

    case EventAction::kPressed: {
      ui::MenuItem item = getcurrentMenuItem();

      if (item.m_subMenu) {
        m_cursor = 0;
        m_first = 0;

        if (item.m_subMenu == &ui::BlankMenuPage) {
          m_menuStack.pop();
        }
        else {
          m_menuStack.push(item.m_subMenu);
        }
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
  switch (t_event.m_action) {
    case EventAction::kDeltaChanged:
      getcurrentMenuItem().m_onMove(t_event.m_data.delta);
      break;

    case EventAction::kPressed:
      endEditing();
      break;
  }
}

void MenuService::handlePotsMoving(const Event& t_event) {
  if (m_potMenuActive) {
    m_menuStack.pop();
  }

  if (m_tempoMenuActive) {
    m_tempoMenuActive = false;
    m_menuStack.pop();
  }

  switch (t_event.m_id) {
    case static_cast<uint8_t>(PotId::kPot0):
      if (!m_logicState.m_activeProgram->m_isDelayEffect){
        m_menuStack.push(&ui::Pot0ValueMenuPage);
        m_potMenuActive = true;
      }
      break;

    case static_cast<uint8_t>(PotId::kPot1):
      m_menuStack.push(&ui::Pot1ValueMenuPage);
      m_potMenuActive = true;
      break;

    case static_cast<uint8_t>(PotId::kPot2):
      m_menuStack.push(&ui::Pot2ValueMenuPage);
      m_potMenuActive = true;
      break;

    case static_cast<uint8_t>(PotId::kMixPot):
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

  publishUIMenuUpdatedEvent();
}

void MenuService::init() {
  m_menuStack.push(&ui::LockScreenMenuPage);
  publishView();
}

void MenuService::handleEvent(const Event& t_event) {
  // Weird
  // if (t_event.m_type == EventType::kProgramChanged || t_event.m_type == EventType::kMenuExprStateToggled) { publishView(); return; }

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

bool MenuService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kPhysical) {
    if (t_event.m_subject == EventSubject::kSwitch
        && t_event.m_action == EventAction::kLongPressed
        && t_event.matchesId(SwitchId::kMenuLock)) return true;

    if (t_event.m_subject == EventSubject::kEncoder
        && t_event.matchesId(EncoderId::kMenuEncoder)) return true;

    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kExpr) return true;
  }

  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kTempo
        && t_event.m_action == EventAction::kValueChanged) return true;

    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) return true;

    if (t_event.m_subject == EventSubject::kBypass
        && t_event.m_action == EventAction::kToggled) return true;
  }

  return false;
}
