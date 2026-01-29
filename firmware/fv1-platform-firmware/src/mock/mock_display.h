#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include "periphs/display.h"

struct DrawCmd {
  enum class Type {
    kText,
    kRect,
    kClear,
    kDisplay
  };

  Type type;
  std::string text;
  int16_t x, y;
  bool highlighted;
};

class MockDisplay : public Display {
  public:
    bool m_initialized = false;
    std::vector<DrawCmd> m_log;

    void init() override { m_initialized = true; }

    void clear() override {
      m_log.push_back({DrawCmd::Type::kClear});
    }

    void drawText(int16_t x, int16_t y, const char* text, bool inverted) override {
      m_log.push_back({DrawCmd::Type::kText, text, x, y, inverted});
    }

    void drawRect(int16_t x, int16_t y, int16_t w, int8_t h, bool filled) override {
      m_log.push_back({DrawCmd::Type::kRect, "", x, y, filled});
    }

    void display() override {
      m_log.push_back({DrawCmd::Type::kDisplay});
    }

    bool showsText(const std::string& text) const {
      return std::any_of(m_log.begin(), m_log.end(), [&](const DrawCmd& cmd){
        return cmd.type == DrawCmd::Type::kText && cmd.text == text;
      });
    }

    bool isSelected(const std::string& text) const {
      return std::any_of(m_log.begin(), m_log.end(), [&](const DrawCmd& cmd){
        return cmd.type == DrawCmd::Type::kText && cmd.text == text && cmd.highlighted;
      });
    }

    void reset() { m_log.clear(); }

    int16_t getWidth() const override { return 128; }
    int16_t getHeight() const override { return 64; }
    uint16_t getTextWidth(const char* t) override { return strlen(t) * 6; }
    uint16_t getTextHeight(const char*) override { return 8; }
    uint16_t getLineHeight() override { return 8; }
    void setTextSize(uint8_t) override {}
};