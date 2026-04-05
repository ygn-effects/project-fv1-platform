#include <Arduino.h>
#include <unity.h>

#include "drivers/display_ssd1306.h"

void setUp(void) {

}

void tearDown() {

}

void test_text() {
  SSD1306Driver m_ssd1306;
  m_ssd1306.init();
  m_ssd1306.clear();

  uint16_t lineHeight = m_ssd1306.getLineHeight();
  m_ssd1306.drawText(0, 0, "Test test test");
  m_ssd1306.drawText(0, lineHeight + 1, "Test test test", true);
  m_ssd1306.display();

  TEST_ASSERT_TRUE(true);
}

void test_rect() {
  SSD1306Driver m_ssd1306;
  m_ssd1306.init();
  m_ssd1306.clear();

  uint16_t lineHeight = m_ssd1306.getLineHeight();
  m_ssd1306.drawRect(0, 0, m_ssd1306.getWidth(), lineHeight);
  m_ssd1306.drawRect(0, lineHeight + 1, m_ssd1306.getWidth(), lineHeight, false);
  m_ssd1306.display();

  TEST_ASSERT_TRUE(true);
}

void setup() {
  delay(2000);

  UNITY_BEGIN();
  RUN_TEST(test_text);
  delay(5000);
  RUN_TEST(test_rect);
  delay(5000);
  UNITY_END();
}

void loop() {
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(500);
}
