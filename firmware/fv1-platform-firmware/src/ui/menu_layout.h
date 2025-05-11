#pragma once

#include <stdint.h>
#include "logic/logical_state.h"
#include "services/menu_service.h"
#include "hal/display.h"

enum class MenuLayout {
  kList,
  kTwoColumns
};

class BaseLayout {
  protected:
    DisplayDriver& m_driver;

    int16_t m_width;
    uint16_t m_lineHeight;
    uint8_t m_space;
    uint16_t m_tab;
    uint8_t m_lineMargin;

  public:
    BaseLayout(DisplayDriver& t_driver)
      : m_driver(t_driver),
        m_width(t_driver.getWidth()),
        m_lineHeight(t_driver.getLineHeight()),
        m_space(2),
        m_tab(t_driver.getTextWidth(">") + m_space),
        m_lineMargin(1) {}

    virtual void draw(const MenuView& t_view, const LogicalState& t_lState) = 0;
};

class ListLayout : public BaseLayout {
  public:
    ListLayout(DisplayDriver& t_driver)
      : BaseLayout(t_driver) {}

    void draw(const MenuView& t_view, const LogicalState& t_lState) override {
      m_driver.drawRect(0, 0, m_width, m_lineHeight + m_lineMargin);
      uint16_t headerW = m_driver.getTextWidth(t_view.m_header);
      uint16_t headerX = (m_width - headerW) / 2;
      m_driver.drawText(headerX, m_lineMargin, t_view.m_header, true);

      for (uint8_t i = 0; i < t_view.m_count; i++) {
        const char* label = t_view.m_items[i]->m_label(&t_lState);
        int16_t y = i * (m_lineHeight + m_lineMargin);
        m_driver.drawText(m_tab, y, label);

        if (auto valueF = t_view.m_items[i]->m_value) {
          int16_t x = m_driver.getTextWidth(label);
          m_driver.drawText(x, y, ":");
          if (t_view.m_editing) {
            m_driver.drawText(x + m_tab, y, valueF(&t_lState), true);
          }
          else {
            m_driver.drawText(x + m_tab, y, valueF(&t_lState));
          }
        }

        if (t_view.m_selected == i) {
          m_driver.drawText(0, y, ">");
        }
      }
    }
};

struct TwoColumnsLayout : public BaseLayout {
  void draw(const MenuView& t_view, DisplayDriver& t_driver, const LogicalState& t_lState) {

  }
};

