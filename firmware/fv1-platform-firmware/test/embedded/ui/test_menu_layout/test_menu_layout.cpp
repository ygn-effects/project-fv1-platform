#include <Arduino.h>
#include <unity.h>

#include "logic/logical_state.h"
#include "drivers/display_ssd1306.h"
#include "ui/menu_model.h"
#include "ui/menu_layout.h"

#include "ui/menu_model.cpp"

void setUp(void) {

}

void test_list_layout() {
  LogicalState state;
  SSD1306Driver driver;
  ui::ListLayout layout(driver);
  ui::MenuView view;

  for (uint8_t i = 0; i < ui::MenuConstants::c_visibleItemsPerPage; i++) {
    view.m_items[i] = &ui::ProgramMenuPage.m_items[i];
  }

  view.m_header = "Test";
  view.m_count = ui::MenuConstants::c_visibleItemsPerPage;
  view.m_layout = ui::ProgramMenuPage.m_layout;
  view.m_editing = false;
  view.m_selected = 1;

  driver.init();
  driver.clear();
  layout.draw(view, state);
  driver.display();
}

void test_label_value() {
  LogicalState state;
  SSD1306Driver driver;
  ui::LabelValueLayout layout(driver);
  ui::MenuView view;

  view.m_items[0] = &ui::Pot0ValueMenuPage.m_items[0];

  view.m_header = "Test";
  view.m_count = ui::Pot0ValueMenuPage.m_count;
  view.m_layout = ui::Pot0ValueMenuPage.m_layout;
  view.m_editing = false;
  view.m_selected = 1;

  state.m_potParams[0]->m_value = 1000;

  driver.init();
  driver.clear();
  layout.draw(view, state);
  driver.display();

  delay(5000);

  view.m_items[0] = &ui::Pot1ValueMenuPage.m_items[0];
  driver.clear();
  layout.draw(view, state);
  driver.display();

  delay(5000);

  view.m_items[0] = &ui::Pot2ValueMenuPage.m_items[0];
  driver.clear();
  layout.draw(view, state);
  driver.display();

  delay(5000);

  view.m_items[0] = &ui::MixPotValueMenuPage.m_items[0];
  driver.clear();
  layout.draw(view, state);
  driver.display();
}

void test_two_columns() {
  LogicalState state;
  SSD1306Driver driver;
  ui::TwoColumnsLayout layout(driver);
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

  driver.init();
  driver.clear();
  layout.draw(view, state);
  driver.display();
}

void tearDown() {

}

void setup() {
  delay(2000);

  UNITY_BEGIN();
  RUN_TEST(test_list_layout);
  delay(5000);
  RUN_TEST(test_label_value);
  delay(5000);
  RUN_TEST(test_two_columns);
  delay(5000);
  UNITY_END();
}

void loop() {
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(500);
}
