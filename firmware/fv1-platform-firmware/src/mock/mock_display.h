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

    bool hasClearCmd() const {
      return m_log.front().type == DrawCmd::Type::kClear;
    }

    bool hasDisplayCmd() const {
      return m_log.back().type == DrawCmd::Type::kDisplay;
    }

    bool showsText(const std::string& text) const {
      return std::any_of(m_log.begin(), m_log.end(), [&](const DrawCmd& cmd){
        return cmd.type == DrawCmd::Type::kText && cmd.text == text;
      });
    }

    bool showsLabelValue(const std::string& t_label, const std::string& t_value) const {
      const std::vector<DrawCmd> target = {
        {DrawCmd::Type::kText, t_label, 0, 0, false},
        {DrawCmd::Type::kText, ":", 0, 0, false},
        {DrawCmd::Type::kText, t_value, 0, 0, false}
      };

      auto it = std::search(
        m_log.begin(), m_log.end(),
        target.begin(), target.end(),

        [](const DrawCmd& a, const DrawCmd& b) {
          return a.type == b.type && a.text == b.text;
        }
      );

      return it != m_log.end();
    }

    bool isHighlighted(const std::string& text) const {
      return std::any_of(m_log.begin(), m_log.end(), [&](const DrawCmd& cmd){
        return cmd.type == DrawCmd::Type::kText && cmd.text == text && cmd.highlighted;
      });
    }

    bool isCursorPointingTo(const std::string& t_label) const {
      auto labelIt = std::find_if(m_log.begin(), m_log.end(), [&](const DrawCmd& cmd){
        return cmd.type == DrawCmd::Type::kText && cmd.text == t_label;
      });

      if (labelIt == m_log.end()) return false;

      return std::any_of(m_log.begin(), m_log.end(), [&](const DrawCmd& cursor){
        if (cursor.type != DrawCmd::Type::kText || cursor.text != ">") return false;

        if (cursor.y != labelIt->y) return false;
        int distance = labelIt->x - cursor.x;

        return (distance >= 5 && distance <= 15);
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