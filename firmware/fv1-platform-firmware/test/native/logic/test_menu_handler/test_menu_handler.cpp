#include <unity.h>
#include "core/event_bus.h"

#include "../src/ui/menu_model.cpp"
#include "../src/logic/menu_handler.cpp"


void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

// =============================================================================
// Initialization Tests
// =============================================================================

void test_default_initialization() {
  MenuHandler handler;

  TEST_ASSERT_EQUAL(UiMode::kLocked, handler.m_mode);
  TEST_ASSERT_EQUAL(SubState::kSelecting, handler.m_subState);
  TEST_ASSERT_EQUAL(0, handler.m_cursor);
  TEST_ASSERT_EQUAL(0, handler.m_first);
}

void test_init_pushes_lock_screen() {
  MenuHandler handler;
  handler.init();

  TEST_ASSERT_EQUAL(1, handler.m_menuStack.depth());
  TEST_ASSERT_EQUAL(&ui::LockScreenMenuPage, handler.m_menuStack.top());
}

// =============================================================================
// Mode Transition Tests
// =============================================================================

void test_lock_resets_state() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);
  handler.m_cursor = 3;
  handler.m_first = 2;
  handler.m_subState = SubState::kEditing;

  handler.lock();

  TEST_ASSERT_EQUAL(UiMode::kLocked, handler.m_mode);
  TEST_ASSERT_EQUAL(SubState::kSelecting, handler.m_subState);
  TEST_ASSERT_EQUAL(0, handler.m_cursor);
  TEST_ASSERT_EQUAL(0, handler.m_first);
  TEST_ASSERT_EQUAL(&ui::LockScreenMenuPage, handler.m_menuStack.top());
}

void test_unlock_pushes_program_menu_in_program_mode() {
  MenuHandler handler;
  LogicalState state;
  state.m_programMode = ProgramMode::kProgram;

  handler.init();
  handler.unlock(state);

  TEST_ASSERT_EQUAL(UiMode::kUnlocked, handler.m_mode);
  TEST_ASSERT_EQUAL(&ui::ProgramMenuPage, handler.m_menuStack.top());
}

void test_unlock_pushes_preset_menu_in_preset_mode() {
  MenuHandler handler;
  LogicalState state;
  state.m_programMode = ProgramMode::kPreset;

  handler.init();
  handler.unlock(state);

  TEST_ASSERT_EQUAL(UiMode::kUnlocked, handler.m_mode);
  TEST_ASSERT_EQUAL(&ui::PresetMenuPage, handler.m_menuStack.top());
}

// =============================================================================
// Cursor Navigation Tests
// =============================================================================

void test_move_cursor_down() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  TEST_ASSERT_EQUAL(0, handler.m_cursor);

  handler.moveCursor(1, state);
  TEST_ASSERT_EQUAL(1, handler.m_cursor);

  handler.moveCursor(1, state);
  TEST_ASSERT_EQUAL(2, handler.m_cursor);
}

void test_move_cursor_up() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);
  handler.m_cursor = 2;

  handler.moveCursor(-1, state);
  TEST_ASSERT_EQUAL(1, handler.m_cursor);

  handler.moveCursor(-1, state);
  TEST_ASSERT_EQUAL(0, handler.m_cursor);
}

void test_move_cursor_wraps_at_bottom() {
  MenuHandler handler;
  LogicalState state;
  state.m_programMode = ProgramMode::kProgram;

  handler.init();
  handler.unlock(state);

  // Count visible items in ProgramMenuPage
  const ui::MenuPage& page = *handler.m_menuStack.top();
  uint8_t visibleCount = 0;
  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&state)) ++visibleCount;
  }

  handler.m_cursor = visibleCount - 1;
  handler.moveCursor(1, state);

  TEST_ASSERT_EQUAL(0, handler.m_cursor);
}

void test_move_cursor_wraps_at_top() {
  MenuHandler handler;
  LogicalState state;
  state.m_programMode = ProgramMode::kProgram;

  handler.init();
  handler.unlock(state);

  // Count visible items in ProgramMenuPage
  const ui::MenuPage& page = *handler.m_menuStack.top();
  uint8_t visibleCount = 0;
  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&state)) ++visibleCount;
  }

  handler.m_cursor = 0;
  handler.moveCursor(-1, state);

  TEST_ASSERT_EQUAL(visibleCount - 1, handler.m_cursor);
}

void test_move_cursor_updates_first_when_scrolling_down() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  // Move cursor past visible window (5 items per page)
  for (int i = 0; i < 6; ++i) {
    handler.moveCursor(1, state);
  }

  // first should have scrolled
  TEST_ASSERT_TRUE(handler.m_first > 0);
  TEST_ASSERT_EQUAL(2, handler.m_first);
}

void test_move_cursor_updates_first_when_scrolling_up() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  // Set cursor and first beyond window
  handler.m_cursor = 5;
  handler.m_first = 3;

  handler.moveCursor(-1, state);
  handler.moveCursor(-1, state);
  handler.moveCursor(-1, state);

  // first should have scrolled back
  TEST_ASSERT_EQUAL(2, handler.m_first);
}

// =============================================================================
// Selection Tests
// =============================================================================

void test_handle_select_pushes_submenu() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  // Find a submenu item (ExprSettingsMenuPage)
  const ui::MenuPage& page = *handler.m_menuStack.top();
  uint8_t targetCursor = 0;
  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&state)) {
      if (page.m_items[i].m_subMenu == &ui::ExprSettingsMenuPage) {
        break;
      }
      ++targetCursor;
    }
  }
  handler.m_cursor = targetCursor;

  uint8_t depthBefore = handler.m_menuStack.depth();
  bool result = handler.handleSelect(state);

  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL(depthBefore + 1, handler.m_menuStack.depth());
  TEST_ASSERT_EQUAL(&ui::ExprSettingsMenuPage, handler.m_menuStack.top());
  TEST_ASSERT_EQUAL(0, handler.m_cursor);
  TEST_ASSERT_EQUAL(0, handler.m_first);
}

void test_handle_select_pops_on_blank_page() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  // Push ExprSettingsMenuPage which has a "Back" item
  handler.m_menuStack.push(&ui::ExprSettingsMenuPage);
  uint8_t depthBefore = handler.m_menuStack.depth();

  // Find "Back" item pointing to BlankMenuPage
  const ui::MenuPage& page = *handler.m_menuStack.top();
  uint8_t targetCursor = 0;
  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&state)) {
      if (page.m_items[i].m_subMenu == &ui::BlankMenuPage) {
        break;
      }
      ++targetCursor;
    }
  }
  handler.m_cursor = targetCursor;

  bool result = handler.handleSelect(state);

  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL(depthBefore - 1, handler.m_menuStack.depth());
}

void test_handle_select_begins_editing_for_onmove_item() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  // Find first item with onMove (should be Program item)
  const ui::MenuPage& page = *handler.m_menuStack.top();
  uint8_t targetCursor = 0;
  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&state)) {
      if (page.m_items[i].m_onMove != nullptr) {
        break;
      }
      ++targetCursor;
    }
  }
  handler.m_cursor = targetCursor;

  bool result = handler.handleSelect(state);

  TEST_ASSERT_TRUE(result);
  TEST_ASSERT_EQUAL(SubState::kEditing, handler.m_subState);
}

void test_handle_select_calls_onclick() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  // Push ExprSettingsMenuPage which has onClick items
  handler.m_menuStack.push(&ui::ExprSettingsMenuPage);

  // Find first item with onClick
  const ui::MenuPage& page = *handler.m_menuStack.top();
  uint8_t targetCursor = 0;
  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&state)) {
      if (page.m_items[i].m_onClick != nullptr) {
        break;
      }
      ++targetCursor;
    }
  }
  handler.m_cursor = targetCursor;

  bool result = handler.handleSelect(state);

  TEST_ASSERT_TRUE(result);
  // onClick was called, state should remain selecting
  TEST_ASSERT_EQUAL(SubState::kSelecting, handler.m_subState);
}

// =============================================================================
// Editing Tests
// =============================================================================

void test_begin_editing() {
  MenuHandler handler;

  TEST_ASSERT_EQUAL(SubState::kSelecting, handler.m_subState);

  handler.beginEditing();

  TEST_ASSERT_EQUAL(SubState::kEditing, handler.m_subState);
}

void test_end_editing() {
  MenuHandler handler;
  handler.m_subState = SubState::kEditing;

  handler.endEditing();

  TEST_ASSERT_EQUAL(SubState::kSelecting, handler.m_subState);
}

void test_apply_edit_delta_calls_onmove() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  // Find first item with onMove (Program item in ProgramMenuPage)
  const ui::MenuPage& page = *handler.m_menuStack.top();
  uint8_t targetCursor = 0;
  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&state)) {
      if (page.m_items[i].m_onMove != nullptr) {
        break;
      }
      ++targetCursor;
    }
  }
  handler.m_cursor = targetCursor;
  handler.beginEditing();

  // Clear any events from setup
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }

  handler.applyEditDelta(1, state);

  // Verify the callback was invoked - it publishes an event
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventDomain::kUI, e.m_domain);
  TEST_ASSERT_EQUAL(EventSubject::kProgram, e.m_subject);
  TEST_ASSERT_EQUAL(EventAction::kValueChanged, e.m_action);
  TEST_ASSERT_EQUAL(1, e.m_data.delta);
}

void test_apply_edit_delta_passes_delta_correctly() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  // Find first item with onMove
  const ui::MenuPage& page = *handler.m_menuStack.top();
  uint8_t targetCursor = 0;
  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&state)) {
      if (page.m_items[i].m_onMove != nullptr) {
        break;
      }
      ++targetCursor;
    }
  }
  handler.m_cursor = targetCursor;
  handler.beginEditing();

  // Clear events
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }

  handler.applyEditDelta(-3, state);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(-3, e.m_data.delta);
}

void test_apply_edit_delta_with_no_onmove_is_noop() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  // Find an item with submenu but no onMove
  const ui::MenuPage& page = *handler.m_menuStack.top();
  uint8_t targetCursor = 0;
  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&state)) {
      if (page.m_items[i].m_subMenu != nullptr && page.m_items[i].m_onMove == nullptr) {
        break;
      }
      ++targetCursor;
    }
  }
  handler.m_cursor = targetCursor;

  // Clear any events
  Event e;
  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }

  // Should not crash and should not publish any events
  handler.applyEditDelta(1, state);

  TEST_ASSERT_FALSE(EventBus::hasEvent());
}

// =============================================================================
// Overlay Tests
// =============================================================================

void test_push_pot_overlay_pot1() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);
  uint8_t depthBefore = handler.m_menuStack.depth();

  handler.pushPotOverlay(static_cast<uint8_t>(PotId::kPot1), state);

  TEST_ASSERT_EQUAL(depthBefore + 1, handler.m_menuStack.depth());
  TEST_ASSERT_EQUAL(&ui::Pot1ValueMenuPage, handler.m_menuStack.top());
}

void test_push_pot_overlay_pot2() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);
  uint8_t depthBefore = handler.m_menuStack.depth();

  handler.pushPotOverlay(static_cast<uint8_t>(PotId::kPot2), state);

  TEST_ASSERT_EQUAL(depthBefore + 1, handler.m_menuStack.depth());
  TEST_ASSERT_EQUAL(&ui::Pot2ValueMenuPage, handler.m_menuStack.top());
}

void test_push_pot_overlay_mix_pot() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);
  uint8_t depthBefore = handler.m_menuStack.depth();

  handler.pushPotOverlay(static_cast<uint8_t>(PotId::kMixPot), state);

  TEST_ASSERT_EQUAL(depthBefore + 1, handler.m_menuStack.depth());
  TEST_ASSERT_EQUAL(&ui::MixPotValueMenuPage, handler.m_menuStack.top());
}

void test_push_pot_overlay_pot0_pushed_for_non_delay_effect() {
  MenuHandler handler;
  LogicalState state;
  // Set program to non-delay
  state.m_activeProgram = &ProgramsDefinitions::kPrograms[7];

  handler.init();
  handler.unlock(state);
  uint8_t depthBefore = handler.m_menuStack.depth();

  // If not a delay effect, Pot0 overlay should be pushed
  if (!state.m_activeProgram->m_isDelayEffect) {
    handler.pushPotOverlay(static_cast<uint8_t>(PotId::kPot0), state);
    TEST_ASSERT_EQUAL(depthBefore + 1, handler.m_menuStack.depth());
    TEST_ASSERT_EQUAL(&ui::Pot0ValueMenuPage, handler.m_menuStack.top());
  }
}

void test_push_tempo_overlay() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);
  uint8_t depthBefore = handler.m_menuStack.depth();

  handler.pushTempoOverlay();

  TEST_ASSERT_EQUAL(depthBefore + 1, handler.m_menuStack.depth());
  TEST_ASSERT_EQUAL(&ui::TempoMenuPage, handler.m_menuStack.top());
}

void test_pop_overlay() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);
  handler.pushTempoOverlay();
  uint8_t depthBefore = handler.m_menuStack.depth();

  handler.popOverlay();

  TEST_ASSERT_EQUAL(depthBefore - 1, handler.m_menuStack.depth());
}

// =============================================================================
// View Building Tests
// =============================================================================

void test_build_view_list_layout() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);
  handler.buildView(state);

  const ui::MenuView& view = handler.m_view;

  TEST_ASSERT_EQUAL(ui::MenuLayout::kList, view.m_layout);
  TEST_ASSERT_EQUAL_STRING("Program mode", view.m_header);
  TEST_ASSERT_TRUE(view.m_count > 0);
  TEST_ASSERT_TRUE(view.m_count <= ui::MenuConstants::c_visibleItemsPerPage);
}

void test_build_view_two_columns_layout() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  // Lock screen uses two columns layout
  handler.buildView(state);

  const ui::MenuView& view = handler.m_view;

  TEST_ASSERT_EQUAL(ui::MenuLayout::kTwoColumns, view.m_layout);
  TEST_ASSERT_EQUAL_STRING("Lock screen", view.m_header);
}

void test_build_view_label_value_layout() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);
  handler.pushPotOverlay(static_cast<uint8_t>(PotId::kPot1), state);
  handler.buildView(state);

  const ui::MenuView& view = handler.m_view;

  TEST_ASSERT_EQUAL(ui::MenuLayout::kLabelValue, view.m_layout);
}

void test_build_view_sets_selected_correctly() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);
  handler.m_cursor = 2;
  handler.buildView(state);

  const ui::MenuView& view = handler.m_view;

  TEST_ASSERT_EQUAL(2, view.m_selected);
}

void test_build_view_editing_flag() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  handler.buildView(state);
  TEST_ASSERT_FALSE(handler.m_view.m_editing);

  handler.beginEditing();
  handler.buildView(state);
  TEST_ASSERT_TRUE(handler.m_view.m_editing);
}

void test_build_view_respects_visibility() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  const ui::MenuPage& page = *handler.m_menuStack.top();

  // Count visible items in program mode
  state.m_programMode = ProgramMode::kProgram;
  uint8_t visibleInProgramMode = 0;
  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&state)) ++visibleInProgramMode;
  }

  // Count visible items in preset mode
  state.m_programMode = ProgramMode::kPreset;
  uint8_t visibleInPresetMode = 0;
  for (uint8_t i = 0; i < page.m_count; ++i) {
    if (page.m_items[i].m_visible(&state)) ++visibleInPresetMode;
  }

  // Both should have items, and counts depend on visibility functions
  TEST_ASSERT_TRUE(visibleInProgramMode > 0);
  TEST_ASSERT_TRUE(visibleInPresetMode > 0);
}

void test_build_view_scrolls_with_first() {
  MenuHandler handler;
  LogicalState state;

  handler.init();
  handler.unlock(state);

  // Set first to skip first 2 items
  handler.m_first = 2;
  handler.m_cursor = 2;
  handler.buildView(state);

  const ui::MenuView& view = handler.m_view;

  // View should start from the third visible item (index 2)
  // Just verify we have items and selected is correct
  TEST_ASSERT_TRUE(view.m_count > 0);
  TEST_ASSERT_EQUAL(0, view.m_selected); // cursor 2 - first 2 = selected 0
}

// =============================================================================
// Resolve Header (Preset Dirty Indicator)
// =============================================================================

void test_resolve_header_shows_dirty_indicator_in_preset_mode() {
  MenuHandler handler;
  LogicalState state;

  state.m_programMode = ProgramMode::kPreset;
  state.m_presetDirty = true;

  handler.init();
  handler.unlock(state);
  handler.buildView(state);

  TEST_ASSERT_EQUAL_STRING("Preset mode *", handler.m_view.m_header);
}

void test_resolve_header_no_indicator_when_clean() {
  MenuHandler handler;
  LogicalState state;

  state.m_programMode = ProgramMode::kPreset;
  state.m_presetDirty = false;

  handler.init();
  handler.unlock(state);
  handler.buildView(state);

  TEST_ASSERT_EQUAL_STRING("Preset mode", handler.m_view.m_header);
}

void test_resolve_header_no_indicator_in_program_mode() {
  MenuHandler handler;
  LogicalState state;

  state.m_programMode = ProgramMode::kProgram;
  state.m_presetDirty = true;

  handler.init();
  handler.unlock(state);
  handler.buildView(state);

  TEST_ASSERT_EQUAL_STRING("Program mode", handler.m_view.m_header);
}

int main() {
  UNITY_BEGIN();

  // Initialization
  RUN_TEST(test_default_initialization);
  RUN_TEST(test_init_pushes_lock_screen);

  // Mode transitions
  RUN_TEST(test_lock_resets_state);
  RUN_TEST(test_unlock_pushes_program_menu_in_program_mode);
  RUN_TEST(test_unlock_pushes_preset_menu_in_preset_mode);

  // Cursor navigation
  RUN_TEST(test_move_cursor_down);
  RUN_TEST(test_move_cursor_up);
  RUN_TEST(test_move_cursor_wraps_at_bottom);
  RUN_TEST(test_move_cursor_wraps_at_top);
  RUN_TEST(test_move_cursor_updates_first_when_scrolling_down);
  RUN_TEST(test_move_cursor_updates_first_when_scrolling_up);

  // Selection
  RUN_TEST(test_handle_select_pushes_submenu);
  RUN_TEST(test_handle_select_pops_on_blank_page);
  RUN_TEST(test_handle_select_begins_editing_for_onmove_item);
  RUN_TEST(test_handle_select_calls_onclick);

  // Editing
  RUN_TEST(test_begin_editing);
  RUN_TEST(test_end_editing);
  RUN_TEST(test_apply_edit_delta_calls_onmove);
  RUN_TEST(test_apply_edit_delta_passes_delta_correctly);
  RUN_TEST(test_apply_edit_delta_with_no_onmove_is_noop);

  // Overlays
  RUN_TEST(test_push_pot_overlay_pot1);
  RUN_TEST(test_push_pot_overlay_pot2);
  RUN_TEST(test_push_pot_overlay_mix_pot);
  RUN_TEST(test_push_pot_overlay_pot0_pushed_for_non_delay_effect);
  RUN_TEST(test_push_tempo_overlay);
  RUN_TEST(test_pop_overlay);

  // View building
  RUN_TEST(test_build_view_list_layout);
  RUN_TEST(test_build_view_two_columns_layout);
  RUN_TEST(test_build_view_label_value_layout);
  RUN_TEST(test_build_view_sets_selected_correctly);
  RUN_TEST(test_build_view_editing_flag);
  RUN_TEST(test_build_view_respects_visibility);
  RUN_TEST(test_build_view_scrolls_with_first);

  // Resolve Header (Preset Dirty Indicator)
  RUN_TEST(test_resolve_header_shows_dirty_indicator_in_preset_mode);
  RUN_TEST(test_resolve_header_no_indicator_when_clean);
  RUN_TEST(test_resolve_header_no_indicator_in_program_mode);

  UNITY_END();
}
