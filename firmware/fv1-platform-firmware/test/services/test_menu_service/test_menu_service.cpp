#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/menu_service.h"
#include "ui/menu_model.h"

#include "../src/services/menu_service.cpp"

void setUp() {
  Event event;

  while (EventBus::hasEvent()) {
    EventBus::recall(event);
  }
}

void tearDown() {

}

void test_unlock_lock() {
  LogicalState logicalState;
  MenuService menuService(logicalState);

  menuService.init();

  TEST_ASSERT_EQUAL("Lock screen", menuService.getcurrentMenuPage().m_header);

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 100, {}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event event;
  EventBus::recall(event);

  TEST_ASSERT_EQUAL(EventType::kMenuUnlocked, event.m_type);
  TEST_ASSERT_EQUAL("Program mode", menuService.getcurrentMenuPage().m_header);

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 100, {}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(event);

  TEST_ASSERT_EQUAL(EventType::kMenuLocked, event.m_type);
  TEST_ASSERT_EQUAL("Lock screen", menuService.getcurrentMenuPage().m_header);
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
  TEST_ASSERT_EQUAL("Program mode", menuService.getcurrentMenuPage().m_header);

  menuService.update();
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuLocked, e.m_type);
  TEST_ASSERT_EQUAL("Lock screen", menuService.getcurrentMenuPage().m_header);
}

void test_scroll() {
  LogicalState logicalState;
  MenuService menuService(logicalState);

  menuService.init();

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 500, {}});
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL("Program", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Tempo", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Delay time", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Feedback", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=-1}});
  TEST_ASSERT_EQUAL("Delay time", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=-1}});
  TEST_ASSERT_EQUAL("Tempo", menuService.getcurrentMenuItem().m_label(&logicalState));
}

void test_wrap_around() {
  LogicalState logicalState;
  MenuService menuService(logicalState);

  menuService.init();

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 500, {}});
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL("Program", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=-1}});
  TEST_ASSERT_EQUAL("Expression settings", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Program", menuService.getcurrentMenuItem().m_label(&logicalState));
}

void test_edit_begin_end() {
  LogicalState logicalState;
  MenuService menuService(logicalState);

  menuService.init();

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 500, {}});
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL("Program", menuService.getcurrentMenuItem().m_label(&logicalState));
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {.delta=1}});
  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {.delta=1}});
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());
}

void test_edit_begin_move_end() {
  LogicalState logicalState;
  MenuService menuService(logicalState);

  menuService.init();

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 500, {}});
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL("Program", menuService.getcurrentMenuItem().m_label(&logicalState));
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuProgramChanged, e.m_type);
  TEST_ASSERT_EQUAL(1, e.m_data.delta);

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=-1}});
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuProgramChanged, e.m_type);
  TEST_ASSERT_EQUAL(-1, e.m_data.delta);

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Tempo", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuTempoChanged, e.m_type);
  TEST_ASSERT_EQUAL(1, e.m_data.delta);

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Delay time", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuPot0Moved, e.m_type);
  TEST_ASSERT_EQUAL(1, e.m_data.delta);

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Feedback", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuPot1Moved, e.m_type);
  TEST_ASSERT_EQUAL(1, e.m_data.delta);

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());
}

void test_sub_menu() {
  LogicalState logicalState;
  MenuService menuService(logicalState);

  menuService.init();

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 500, {}});
  Event e;
  EventBus::recall(e);

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=-1}});
  TEST_ASSERT_EQUAL("Expression settings", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});

  TEST_ASSERT_EQUAL("Expression settings", menuService.getcurrentMenuPage().m_header);
  TEST_ASSERT_EQUAL("State", menuService.getcurrentMenuItem().m_label(&logicalState));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_unlock_lock);
  RUN_TEST(test_menu_lock_timeout);
  RUN_TEST(test_scroll);
  RUN_TEST(test_wrap_around);
  RUN_TEST(test_edit_begin_end);
  RUN_TEST(test_edit_begin_move_end);
  RUN_TEST(test_sub_menu);
  UNITY_END();
}