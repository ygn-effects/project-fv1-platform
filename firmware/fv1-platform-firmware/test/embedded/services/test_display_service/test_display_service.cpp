#include <Arduino.h>
#include <unity.h>

#include "logic/logical_state.h"
#include "drivers/display_ssd1306.h"
#include "services/display_service.h"

#include "services/display_service.cpp"
#include "ui/menu_model.cpp"

void setUp(void) {

}

void tearDown() {

}

void test_display_service_init() {
  LogicalState state;
  SSD1306Driver driver;

  DisplayService displayService(state, driver);

  ui::MenuView view;

  uint8_t offset = 0;

  for (uint8_t i = 0; i < ui::LockScreenMenuPage.m_count; i++) {
    if (ui::LockScreenMenuPage.m_items[i].m_visible(&state)) {
      view.m_items[offset] = &ui::LockScreenMenuPage.m_items[i];
      offset++;
    }
  }

  view.m_header = ui::LockScreenMenuPage.m_header;
  view.m_count = offset;
  view.m_layout = ui::LockScreenMenuPage.m_layout;
  view.m_editing = false;
  view.m_selected = 0;

  displayService.init();
  Event e;
  e.m_type = EventType::kMenuViewUpdated;
  e.m_timestamp = 0;
  e.m_data.view=&view;

  displayService.handleEvent(e);
}

void setup() {
  delay(2000);

  UNITY_BEGIN();
  RUN_TEST(test_display_service_init);
  delay(5000);
  UNITY_END();
}

void loop() {
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(500);
}
