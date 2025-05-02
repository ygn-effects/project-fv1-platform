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
  TEST_ASSERT_EQUAL("Feedback", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Low pass", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=-1}});
  TEST_ASSERT_EQUAL("Feedback", menuService.getcurrentMenuItem().m_label(&logicalState));

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

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Low pass", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuPot2Moved, e.m_type);
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

void test_not_visible() {
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

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("State", menuService.getcurrentMenuItem().m_label(&logicalState));

  TEST_ASSERT_TRUE(menuService.getcurrentMenuItem().m_visible(&logicalState));
  logicalState.m_exprParams[logicalState.m_currentProgram].m_state = ExprState::kActive;

  TEST_ASSERT_TRUE(menuService.getcurrentMenuItem().m_visible(&logicalState));
}

void test_publish() {
  LogicalState logicalState;
  MenuService menuService(logicalState);

  menuService.init();

  menuService.handleEvent({EventType::kMenuEncoderLongPressed, 30000, {}});
  Event e;
  EventBus::recall(e);

  menuService.update();
  const MenuView* view = menuService.getMenuView();

  TEST_ASSERT_EQUAL("Program", view->m_items[0]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Tempo", view->m_items[1]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Feedback", view->m_items[2]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Low pass", view->m_items[3]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Mix", view->m_items[4]->m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 30000, {.delta=5}});
  menuService.update();

  TEST_ASSERT_EQUAL("Tempo", view->m_items[0]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Feedback", view->m_items[1]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Low pass", view->m_items[2]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Mix", view->m_items[3]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Expression settings", view->m_items[4]->m_label(&logicalState));
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
  RUN_TEST(test_not_visible);
  RUN_TEST(test_publish);
  UNITY_END();
}