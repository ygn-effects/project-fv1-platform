#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/menu_service.h"

#include "../src/services/menu_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_menu_basic() {
  LogicalState logicalState;
  MenuService menuService(logicalState);

  menuService.init();

  TEST_ASSERT_EQUAL("<empty>", menuService.currentMenu()->m_items[0].m_label);
}

void test_menu_unlock_lock() {
  LogicalState logicalState;
  MenuService menuService(logicalState);

  menuService.init();

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 500, {}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuUnlocked, e.m_type);
  UiMode mode = menuService.getUiMode();
  TEST_ASSERT_EQUAL(UiMode::kProgramEdit, mode);

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 1000, {}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuLocked, e.m_type);
  mode = menuService.getUiMode();
  TEST_ASSERT_EQUAL(UiMode::kLocked, mode);
}

void test_menu_lock_timeout() {
  LogicalState logicalState;
  MenuService menuService(logicalState);

  menuService.init();

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 500, {}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuUnlocked, e.m_type);
  UiMode mode = menuService.getUiMode();
  TEST_ASSERT_EQUAL(UiMode::kProgramEdit, mode);

  menuService.update();
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuLocked, e.m_type);
  mode = menuService.getUiMode();
  TEST_ASSERT_EQUAL(UiMode::kLocked, mode);
}

void test_move_cursor() {
  LogicalState logicalState;
  MenuService menuService(logicalState);

  menuService.init();

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 500, {}});
  Event e;
  EventBus::recall(e);

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta = 1}});
  TEST_ASSERT_EQUAL(1, menuService.getCursor());

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta = -1}});
  TEST_ASSERT_EQUAL(0, menuService.getCursor());
}

void test_cursor_wrap_around() {
  LogicalState logicalState;
  MenuService menuService(logicalState);

  menuService.init();

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 500, {}});
  Event e;
  EventBus::recall(e);

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta = -1}});
  TEST_ASSERT_EQUAL(2, menuService.getCursor());

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta = 1}});
  TEST_ASSERT_EQUAL(0, menuService.getCursor());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_menu_basic);
  RUN_TEST(test_menu_unlock_lock);
  RUN_TEST(test_menu_lock_timeout);
  RUN_TEST(test_move_cursor);
  RUN_TEST(test_cursor_wrap_around);
  UNITY_END();
}