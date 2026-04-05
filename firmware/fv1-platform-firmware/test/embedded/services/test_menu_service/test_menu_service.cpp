#include <unity.h>
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "drivers/display_ssd1306.h"
#include "services/display_service.h"
#include "services/menu_service.h"
#include "ui/menu_model.h"
#include "drivers/clock_driver.h"

#include "services/display_service.cpp"
#include "services/menu_service.cpp"
#include "ui/menu_model.cpp"

void setUp(void) {

}

void tearDown() {

}

void test_menu_service_init() {
  LogicalState logicalState;
  SSD1306Driver driver;
  hal::ArduinoClock clock;

  MenuService menuService(logicalState, clock);
  DisplayService displayService(logicalState, driver);

  menuService.init();
  displayService.init();

  TEST_ASSERT_TRUE(EventBus::hasEvent());
  Event e;
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);
  displayService.handleEvent(e);

  delay(2000);

  menuService.handleEvent({EventType::kMenuLockLongPressed, 100, {}});
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuUnlocked, e.m_type);
  TEST_ASSERT_TRUE(EventBus::hasEvent());
  EventBus::recall(e);

  TEST_ASSERT_EQUAL(EventType::kMenuViewUpdated, e.m_type);
  displayService.handleEvent(e);
}

void setup() {
  delay(2000);

  UNITY_BEGIN();
  RUN_TEST(test_menu_service_init);
  delay(5000);
  UNITY_END();
}

void loop() {
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(500);
}
