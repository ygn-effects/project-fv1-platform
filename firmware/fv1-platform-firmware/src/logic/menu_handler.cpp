#include "logic/menu_handler.h"

void MenuHandler::resolveHeader(const ui::MenuPage& t_page, const LogicalState& t_state) {
  if (t_state.m_presetDirty && t_state.m_programMode == ProgramMode::kPreset
      && &t_page == &ui::PresetMenuPage) {
    size_t len = strlen(t_page.m_header);

    if (len + 3 <= sizeof(m_headerBuffer)) {
      memcpy(m_headerBuffer, t_page.m_header, len);
      m_headerBuffer[len] = ' ';
      m_headerBuffer[len + 1] = '*';
      m_headerBuffer[len + 2] = '\0';
    }
    m_view.m_header = m_headerBuffer;
  }
  else {
    m_view.m_header = t_page.m_header;
  }
}

uint8_t MenuHandler::getVisibleItemCount(const LogicalState& t_state) const {
  const ui::MenuPage& page = *m_menuStack.top();
  uint8_t count = 0;

  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&t_state)) {
      ++count;
    }
  }

  return count;
}

uint8_t MenuHandler::visibleToRealIndex(uint8_t t_visibleIndex, const LogicalState& t_state) const {
  const ui::MenuPage& page = *m_menuStack.top();
  uint8_t visCount = 0;

  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&t_state)) {
      if (visCount == t_visibleIndex) {
        return i;
      }
      ++visCount;
    }
  }

  return 0;
}

void MenuHandler::buildListView(const ui::MenuPage& t_page, const LogicalState& t_state) {
  uint8_t visIndex = 0;
  uint8_t sliceCount = 0;

  for (uint8_t i = 0; i < t_page.m_count; ++i) {
    const ui::MenuItem& item = t_page.m_items[i];

    if (!item.m_visible(&t_state))
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
  resolveHeader(t_page, t_state);
  m_view.m_layout = t_page.m_layout;
  m_view.m_editing = (m_subState == SubState::kEditing);
}

void MenuHandler::buildTwoColumnsView(const ui::MenuPage& t_page, const LogicalState& t_state) {
  uint8_t sliceCount = 0;

  for (uint8_t i = 0; i < t_page.m_count; i++) {
    if (t_page.m_count > ui::MenuConstants::c_visibleItemsPerTwoColumns) {
      break;
    }

    const ui::MenuItem& item = t_page.m_items[i];

    if (!item.m_visible(&t_state)) {
      continue;
    }

    m_view.m_items[sliceCount] = &t_page.m_items[i];
    sliceCount++;
  }

  resolveHeader(t_page, t_state);
  m_view.m_count = sliceCount;
  m_view.m_layout = t_page.m_layout;
  m_view.m_selected = 0;
  m_view.m_editing = false;
}

void MenuHandler::buildLabelValueView(const ui::MenuPage& t_page, const LogicalState& t_state) {
  uint8_t sliceCount = 0;

  for (uint8_t i = 0; i < t_page.m_count; i++) {
    const ui::MenuItem& item = t_page.m_items[i];

    if (!item.m_visible(&t_state)) {
      continue;
    }

    m_view.m_items[sliceCount] = &t_page.m_items[i];
    sliceCount++;
  }

  resolveHeader(t_page, t_state);
  m_view.m_count = sliceCount;
  m_view.m_layout = t_page.m_layout;
  m_view.m_selected = 0;
  m_view.m_editing = false;
}

void MenuHandler::init() {
  m_menuStack.push(&ui::LockScreenMenuPage);
}

void MenuHandler::lock() {
  m_mode = UiMode::kLocked;
  m_subState = SubState::kSelecting;

  m_menuStack.clear();
  m_menuStack.push(&ui::LockScreenMenuPage);

  m_cursor = 0;
  m_first = 0;
}

void MenuHandler::unlock(const LogicalState& t_state) {
  m_mode = UiMode::kUnlocked;

  m_menuStack.clear();
  t_state.m_programMode == ProgramMode::kProgram
    ? m_menuStack.push(&ui::ProgramMenuPage)
    : m_menuStack.push(&ui::PresetMenuPage);
}

void MenuHandler::moveCursor(int8_t t_delta, const LogicalState& t_state) {
  uint8_t visibleCount = getVisibleItemCount(t_state);
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

bool MenuHandler::handleSelect(const LogicalState& t_state) {
  const ui::MenuItem& item = m_menuStack.top()->m_items[visibleToRealIndex(m_cursor, t_state)];

  if (item.m_subMenu) {
    m_cursor = 0;
    m_first = 0;

    if (item.m_subMenu == &ui::BlankMenuPage) {
      m_menuStack.pop();
    }
    else {
      m_menuStack.push(item.m_subMenu);
    }
    return true;
  }
  else if (item.m_onClick) {
    item.m_onClick();
    return true;
  }
  else if (item.m_onMove) {
    beginEditing();
    return true;
  }

  return false;
}

void MenuHandler::beginEditing() {
  m_subState = SubState::kEditing;
}

void MenuHandler::endEditing() {
  m_subState = SubState::kSelecting;
}

void MenuHandler::applyEditDelta(int8_t t_delta, const LogicalState& t_state) {
  const ui::MenuItem& item = m_menuStack.top()->m_items[visibleToRealIndex(m_cursor, t_state)];
  if (item.m_onMove) {
    item.m_onMove(t_delta);
  }
}

void MenuHandler::pushPotOverlay(uint8_t t_potId, const LogicalState& t_state) {
  switch (t_potId) {
    case static_cast<uint8_t>(PotId::kPot0):
      if (!t_state.m_activeProgram->m_isDelayEffect) {
        m_menuStack.push(&ui::Pot0ValueMenuPage);
      }
      break;

    case static_cast<uint8_t>(PotId::kPot1):
      m_menuStack.push(&ui::Pot1ValueMenuPage);
      break;

    case static_cast<uint8_t>(PotId::kPot2):
      m_menuStack.push(&ui::Pot2ValueMenuPage);
      break;

    case static_cast<uint8_t>(PotId::kMixPot):
      m_menuStack.push(&ui::MixPotValueMenuPage);
      break;

    default:
      break;
  }
}

void MenuHandler::pushTempoOverlay() {
  m_menuStack.push(&ui::TempoMenuPage);
}

void MenuHandler::pushPresetSaveOverlay() {
  m_menuStack.push(&ui::SavePresetMenuPage);
}

void MenuHandler::popOverlay() {
  m_menuStack.pop();
}

void MenuHandler::buildView(const LogicalState& t_state) {
  const ui::MenuPage& page = *m_menuStack.top();

  switch (page.m_layout) {
    case ui::MenuLayout::kList:
      buildListView(page, t_state);
      break;

    case ui::MenuLayout::kTwoColumns:
      buildTwoColumnsView(page, t_state);
      break;

    case ui::MenuLayout::kLabelValue:
      buildLabelValueView(page, t_state);
      break;
  }
}
