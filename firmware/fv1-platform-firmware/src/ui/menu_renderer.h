#pragma once

#include "logic/logical_state.h"
#include "ui/menu_model.h"
#include "ui/menu_layout.h"

namespace ui {

class MenuRenderer {
  private:
    ui::ListLayout m_listLayout;
    ui::TwoColumnsLayout m_twoColumnsLayout;

  public:
    MenuRenderer(DisplayDriver& t_driver)
      : m_listLayout(t_driver),
        m_twoColumnsLayout(t_driver) {}

    void drawMenu(const ui::MenuView& t_view, const LogicalState& t_lState) {
      switch (t_view.m_layout) {
        case ui::MenuLayout::kList:
          m_listLayout.draw(t_view, t_lState);
          break;

        case ui::MenuLayout::kTwoColumns:
          m_twoColumnsLayout.draw(t_view, t_lState);
          break;

        default:
          break;
      }
  }
};

}
