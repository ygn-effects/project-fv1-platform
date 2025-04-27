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

  TEST_ASSERT_EQUAL("Program", menuService.getcurrentMenuPage().m_items[0].m_label(&logicalState));

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

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_unlock_lock);
  RUN_TEST(test_menu_lock_timeout);
  RUN_TEST(test_scroll);
  UNITY_END();
}