#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "services/menu_service.h"
#include "ui/menu_model.h"
#include "mock/mock_clock.h"

#include "../src/services/menu_service.cpp"
#include "../src/ui/menu_model.cpp"

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
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event event;
  EventBus::recall(event);

  TEST_ASSERT_EQUAL("Lock screen", menuService.getcurrentMenuPage().m_header);

  menuService.handleEvent({EventType::kMenuLockLongPressed, 100, {}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(event);

  TEST_ASSERT_EQUAL(EventType::kMenuUnlocked, event.m_type);
  TEST_ASSERT_EQUAL("Program mode", menuService.getcurrentMenuPage().m_header);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(event);

  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, event.m_type);

  menuService.handleEvent({EventType::kMenuLockLongPressed, 100, {}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(event);

  TEST_ASSERT_EQUAL(EventType::kMenuLocked, event.m_type);
  TEST_ASSERT_EQUAL("Lock screen", menuService.getcurrentMenuPage().m_header);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(event);

  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, event.m_type);
}

void test_menu_lock_timeout() {
  LogicalState logicalState;
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  menuService.handleEvent({EventType::kMenuLockLongPressed, 500, {}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuUnlocked, e.m_type);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);
  TEST_ASSERT_EQUAL("Program mode", menuService.getcurrentMenuPage().m_header);

  clock.setClock(31000);
  menuService.update();
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuLocked, e.m_type);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);
  TEST_ASSERT_EQUAL("Lock screen", menuService.getcurrentMenuPage().m_header);
}

void test_scroll() {
  LogicalState logicalState;
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();

  menuService.handleEvent({EventType::kMenuLockLongPressed, 500, {}});
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL("Prog", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Tempo", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Feedback", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Tone LPF", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=-1}});
  TEST_ASSERT_EQUAL("Feedback", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=-1}});
  TEST_ASSERT_EQUAL("Tempo", menuService.getcurrentMenuItem().m_label(&logicalState));
}

void test_wrap_around() {
  LogicalState logicalState;
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();

  menuService.handleEvent({EventType::kMenuLockLongPressed, 500, {}});
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL("Prog", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=-1}});
  TEST_ASSERT_EQUAL("Expression settings", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Prog", menuService.getcurrentMenuItem().m_label(&logicalState));
}

void test_edit_begin_end() {
  LogicalState logicalState;
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();

  menuService.handleEvent({EventType::kMenuLockLongPressed, 500, {}});
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL("Prog", menuService.getcurrentMenuItem().m_label(&logicalState));
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {.delta=1}});
  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {.delta=1}});
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());
}

void test_edit_begin_move_end() {
  LogicalState logicalState;
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();
  Event e;
  EventBus::recall(e);

  menuService.handleEvent({EventType::kMenuLockLongPressed, 500, {}});

  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }

  TEST_ASSERT_EQUAL("Prog", menuService.getcurrentMenuItem().m_label(&logicalState));
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kMenuProgramChanged, e.m_type);
  TEST_ASSERT_EQUAL(1, e.m_data.delta);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=-1}});
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuProgramChanged, e.m_type);
  TEST_ASSERT_EQUAL(-1, e.m_data.delta);

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Tempo", menuService.getcurrentMenuItem().m_label(&logicalState));

  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuTempoChanged, e.m_type);
  TEST_ASSERT_EQUAL(1, e.m_data.delta);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Feedback", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuPot1Moved, e.m_type);
  TEST_ASSERT_EQUAL(1, e.m_data.delta);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_EQUAL("Tone LPF", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kEditing, menuService.getsubState());

  while (EventBus::hasEvent()) {
    EventBus::recall(e);
  }

  menuService.handleEvent({EventType::kMenuEncoderMoved, 100, {.delta=1}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuPot2Moved, e.m_type);
  TEST_ASSERT_EQUAL(1, e.m_data.delta);

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);
  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);

  menuService.handleEvent({EventType::kMenuEncoderPressed, 100, {}});
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());
}

void test_sub_menu() {
  LogicalState logicalState;
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();

  menuService.handleEvent({EventType::kMenuLockLongPressed, 500, {}});
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
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();

  menuService.handleEvent({EventType::kMenuLockLongPressed, 500, {}});
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

void test_publish_list_menu() {
  LogicalState logicalState;
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);

  menuService.handleEvent({EventType::kMenuLockLongPressed, 30000, {}});
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuUnlocked, e.m_type);
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);

  const ui::MenuView& view = *e.m_data.view;

  TEST_ASSERT_EQUAL("Prog", view.m_items[0]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Tempo", view.m_items[1]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Feedback", view.m_items[2]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Tone LPF", view.m_items[3]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Mix", view.m_items[4]->m_label(&logicalState));

  menuService.handleEvent({EventType::kMenuEncoderMoved, 30000, {.delta=5}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);

  const ui::MenuView& view2 = *e.m_data.view;

  TEST_ASSERT_EQUAL("Tempo", view2.m_items[0]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Feedback", view2.m_items[1]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Tone LPF", view2.m_items[2]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Mix", view2.m_items[3]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Expression settings", view2.m_items[4]->m_label(&logicalState));
}

void test_pot_menu() {
  LogicalState logicalState;
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();

  menuService.handleEvent({EventType::kMenuLockLongPressed, 500, {}});
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL("Prog", menuService.getcurrentMenuItem().m_label(&logicalState));
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  menuService.handleEvent({EventType::kPot0Moved, 1000, {}});
  TEST_ASSERT_EQUAL("P0 Setting", menuService.getcurrentMenuPage().m_header);
  TEST_ASSERT_EQUAL("Tempo", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kPot1Moved, 2000, {}});
  TEST_ASSERT_EQUAL("P1 Setting", menuService.getcurrentMenuPage().m_header);

  menuService.handleEvent({EventType::kPot2Moved, 2000, {}});
  TEST_ASSERT_EQUAL("P2 Setting", menuService.getcurrentMenuPage().m_header);

  menuService.handleEvent({EventType::kMixPotMoved, 2000, {}});
  TEST_ASSERT_EQUAL("Mix Setting", menuService.getcurrentMenuPage().m_header);
}

void test_pot_menu_timeout() {
  LogicalState logicalState;
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();

  menuService.handleEvent({EventType::kMenuLockLongPressed, 23000, {}});
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL("Prog", menuService.getcurrentMenuItem().m_label(&logicalState));
  TEST_ASSERT_EQUAL(SubState::kSelecting, menuService.getsubState());

  menuService.handleEvent({EventType::kPot0Moved, 24000, {}});
  TEST_ASSERT_EQUAL("P0 Setting", menuService.getcurrentMenuPage().m_header);
  TEST_ASSERT_EQUAL("Tempo", menuService.getcurrentMenuItem().m_label(&logicalState));

  clock.setClock(31000);
  menuService.update();

  TEST_ASSERT_EQUAL("Prog", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.handleEvent({EventType::kPot0Moved, 30800, {}});
  TEST_ASSERT_EQUAL("P0 Setting", menuService.getcurrentMenuPage().m_header);
  TEST_ASSERT_EQUAL("Tempo", menuService.getcurrentMenuItem().m_label(&logicalState));

  menuService.update();

  TEST_ASSERT_EQUAL("P0 Setting", menuService.getcurrentMenuPage().m_header);
}

void test_lock_screen() {
  LogicalState logicalState;
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();

  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL("Lock screen", menuService.getcurrentMenuPage().m_header);
  TEST_ASSERT_EQUAL("Prog", menuService.getcurrentMenuPage().m_items[0].m_label(&logicalState));
  TEST_ASSERT_EQUAL("B", menuService.getcurrentMenuPage().m_items[1].m_label(&logicalState));
  TEST_ASSERT_EQUAL("P", menuService.getcurrentMenuPage().m_items[2].m_label(&logicalState));
  TEST_ASSERT_EQUAL("T", menuService.getcurrentMenuPage().m_items[3].m_label(&logicalState));
  TEST_ASSERT_EQUAL("P0", menuService.getcurrentMenuPage().m_items[4].m_label(&logicalState));
  TEST_ASSERT_EQUAL("P1", menuService.getcurrentMenuPage().m_items[5].m_label(&logicalState));
  TEST_ASSERT_EQUAL("P2", menuService.getcurrentMenuPage().m_items[6].m_label(&logicalState));
  TEST_ASSERT_EQUAL("Mix", menuService.getcurrentMenuPage().m_items[7].m_label(&logicalState));
  TEST_ASSERT_EQUAL("Expr", menuService.getcurrentMenuPage().m_items[8].m_label(&logicalState));
}

void test_publish_lock_screen() {
  LogicalState logicalState;
  MockedClock clock;
  MenuService menuService(logicalState, clock);

  menuService.init();
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);

  const ui::MenuView& view = *e.m_data.view;

  TEST_ASSERT_EQUAL("Prog", view.m_items[0]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("T", view.m_items[1]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("P1", view.m_items[2]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("P2", view.m_items[3]->m_label(&logicalState));
  TEST_ASSERT_EQUAL("Mix", view.m_items[4]->m_label(&logicalState));
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
  RUN_TEST(test_publish_list_menu);
  RUN_TEST(test_pot_menu);
  RUN_TEST(test_pot_menu_timeout);
  RUN_TEST(test_lock_screen);
  RUN_TEST(test_publish_lock_screen);
  UNITY_END();
}