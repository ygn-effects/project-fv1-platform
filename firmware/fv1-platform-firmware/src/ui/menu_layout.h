#pragma once

#include <stdint.h>
#include "logic/logical_state.h"
#include "periphs/display.h"
#include "ui/menu_model.h"

namespace ui {

class BaseLayout {
  protected:
    Display& m_display;

    int16_t m_width;
    int16_t m_height;
    uint16_t m_lineHeight;
    uint8_t m_space;
    uint16_t m_tab;
    uint8_t m_lineMargin;

  public:
    BaseLayout(Display& t_display)
      : m_display(t_display),
        m_width(t_display.getWidth()),
        m_height(t_display.getHeight()),
        m_lineHeight(t_display.getLineHeight()),
        m_space(2),
        m_tab(t_display.getTextWidth(">") + m_space),
        m_lineMargin(1) {}

    virtual void draw(const ui::MenuView& t_view, const LogicalState& t_lState) = 0;
};

class ListLayout : public BaseLayout {
  public:
    ListLayout(Display& t_display)
      : BaseLayout(t_display) {}

    void draw(const ui::MenuView& t_view, const LogicalState& t_lState) override {
      m_display.drawRect(0, 0, m_width, m_lineHeight + m_lineMargin * 2);
      uint16_t headerW = m_display.getTextWidth(t_view.m_header);
      uint16_t headerX = (m_width - headerW) / 2;
      m_display.drawText(headerX, m_lineMargin, t_view.m_header, true);

      for (uint8_t i = 0; i < t_view.m_count; i++) {
        const char* label = t_view.m_items[i]->m_label(&t_lState);
        int16_t y = i * (m_lineHeight + m_lineMargin * 2) + (m_lineHeight + m_lineMargin * 2);
        m_display.drawText(m_tab, y, label);

        if (auto valueF = t_view.m_items[i]->m_value) {
          int16_t x = m_display.getTextWidth(label) + m_tab;
          m_display.drawText(x, y, ":");
          if (t_view.m_editing) {
            m_display.drawText(x + m_tab, y, valueF(&t_lState), true);
          }
          else {
            m_display.drawText(x + m_tab, y, valueF(&t_lState));
          }
        }

        if (t_view.m_selected == i) {
          m_display.drawText(0, y, ">");
        }
      }
    }
};

class TwoColumnsLayout : public BaseLayout {
  public:
    TwoColumnsLayout(Display& t_display)
      : BaseLayout(t_display) {}

    void draw(const ui::MenuView& t_view, const LogicalState& t_lState) override {
      m_display.drawRect(0, 0, m_width, m_lineHeight + m_lineMargin * 2);
      uint16_t headerW = m_display.getTextWidth(t_view.m_header);
      uint16_t headerX = (m_width - headerW) / 2;
      m_display.drawText(headerX, m_lineMargin, t_view.m_header, true);

      int16_t x = m_space;
      int16_t y = (m_lineHeight + m_lineMargin * 2);
      const char* progLabel = t_view.m_items[0]->m_label(&t_lState);
      m_display.drawText(x, y, progLabel);

      if (auto valueF = t_view.m_items[0]->m_value) {
        x = m_display.getTextWidth(progLabel) + m_space;
        m_display.drawText(x, y, ":");
        m_display.drawText(x + m_tab, y, valueF(&t_lState));
      }

      uint16_t halfWidth = m_width / 2;
      uint8_t remaining = t_view.m_count - 1;
      uint8_t rows = (remaining + 1) / 2;

      for (uint8_t i = 0; i < remaining; i++) {
        uint8_t offset = i + 1;
        uint8_t column = i / rows;
        uint8_t row = i % rows;
        x = column * halfWidth;
        y = (row + 1) * (m_lineHeight + m_lineMargin * 2) + (m_lineHeight + m_lineMargin * 2);

        const char* label = t_view.m_items[offset]->m_label(&t_lState);
        m_display.drawText(x, y, label);

        if (auto valueF = t_view.m_items[offset]->m_value) {
          m_display.drawText(column * halfWidth + m_display.getTextWidth(label) + m_space, y, ":");
          m_display.drawText(x + halfWidth - (m_display.getTextWidth(valueF(&t_lState))) - m_tab, y, valueF(&t_lState));
        }
      }
    }
};

class LabelValueLayout : public BaseLayout {
  public:
    LabelValueLayout(Display& t_display)
      : BaseLayout(t_display) {}

    void draw(const ui::MenuView& t_view, const LogicalState& t_lState) override {
      const char* label = t_view.m_items[0]->m_label(&t_lState);
      const char* value = t_view.m_items[0]->m_value(&t_lState);

      m_display.setTextSize(2);
      uint16_t y = m_lineHeight;
      uint16_t x = (m_width - m_display.getTextWidth(label)) / 2;

      m_display.drawText(x, y, label);

      m_display.drawRect(0, m_height / 2, m_width, 1);

      y = m_height - m_display.getTextHeight(value) - m_lineHeight;
      x = (m_width - m_display.getTextWidth(value)) / 2;

      m_display.drawText(x, y, value);

      m_display.setTextSize(1);
    }
};

}
