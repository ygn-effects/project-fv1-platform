#pragma once

#include <stdint.h>
#include "periphs/display.h"

class MockDisplay : public Display {
  public:
    bool m_initialized = false;
    bool m_cleared = false;
    bool m_displayed = false;
    uint8_t m_textSize = 1;

    void init() override {
      m_initialized = true;
    }

    void clear() override {
      m_cleared = true;
    }

    void drawRect(int16_t t_x, int16_t t_y, int16_t t_w, int8_t t_h, bool t_filled = true) override {
      (void)t_x; (void)t_y; (void)t_w; (void)t_h; (void)t_filled;
    }

    void drawText(int16_t t_x, int16_t t_y, const char* t_text, bool t_inverted = false) override {
      (void)t_x; (void)t_y; (void)t_text; (void)t_inverted;
    }

    void setTextSize(uint8_t t_size) override {
      m_textSize = t_size;
    }

    void display() override {
      m_displayed = true;
    }

    int16_t getWidth() const override { return 128; }
    int16_t getHeight() const override { return 64; }
    uint16_t getTextWidth(const char* t_text) override { (void)t_text; return 6; }
    uint16_t getTextHeight(const char* t_text) override { (void)t_text; return 8; }
    uint16_t getLineHeight() override { return 10; }

    void reset() {
      m_initialized = false;
      m_cleared = false;
      m_displayed = false;
      m_textSize = 1;
    }
};
