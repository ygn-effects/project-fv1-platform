#pragma once

#include <stdint.h>
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"
#include "Wire.h"
#include "periphs/display.h"

class SSD1306Driver : public Display {
  private:
    Adafruit_SSD1306 m_ssd1306;

  public:
    SSD1306Driver() :
      m_ssd1306(128, 64, &Wire, -1) {}

    void init() override {
      m_ssd1306.begin(SSD1306_SWITCHCAPVCC, 0x3C);
      m_ssd1306.setTextSize(1);
      m_ssd1306.setTextColor(SSD1306_WHITE);
    }

    void clear() override {
      m_ssd1306.clearDisplay();
    }

    void drawRect(int16_t t_x, int16_t t_y, int16_t t_w, int8_t t_h, bool t_filled = true) override {
      if (t_filled) {
        m_ssd1306.fillRect(t_x, t_y, t_w, t_h, SSD1306_WHITE);
      }
      else {
        m_ssd1306.drawRect(t_x, t_y, t_w, t_h, SSD1306_WHITE);
      }
    }

    void drawText(int16_t t_x, int16_t t_y, const char* t_text, bool t_inverted = false) {
      m_ssd1306.setCursor(t_x, t_y);

      if (t_inverted) {
        m_ssd1306.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        m_ssd1306.print(t_text);
        m_ssd1306.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
      }
      else {
        m_ssd1306.print(t_text);
      }
    }

    void setTextSize(uint8_t t_size) override {
      m_ssd1306.setTextSize(t_size);
    }

    void display() override {
      m_ssd1306.display();
    }

    int16_t getWidth() const override { return static_cast<int16_t>(128); }
    int16_t getHeight() const override { return static_cast<int16_t>(64); }

    uint16_t getTextWidth(const char* t_text) {
      int16_t x1, y1;
      uint16_t width, height;
      m_ssd1306.getTextBounds(t_text, 0, 0, &x1, &y1, &width, &height);

      return width;
    }

    uint16_t getTextHeight(const char* t_text) {
      int16_t x1, y1;
      uint16_t textWidth, height;
      m_ssd1306.getTextBounds(t_text, 0, 0, &x1, &y1, &textWidth, &height);

      return height;
    }

    uint16_t getLineHeight() { return getTextHeight("A"); }
};
