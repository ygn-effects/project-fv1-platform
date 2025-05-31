#pragma once

#include "logic/logical_state.h"
#include "ui/menu_model.h"
#include "ui/menu_layout.h"

namespace ui {

class MenuRenderer {
  private:
    ui::ListLayout m_listLayout;
    ui::TwoColumnsLayout m_twoColumnsLayout;
    ui::LabelValueLayout m_labelValueLayout;

  public:
    MenuRenderer(Display& t_display)
      : m_listLayout(t_display),
        m_twoColumnsLayout(t_display),
        m_labelValueLayout(t_display) {}

    void init() {
      m_listLayout.init();
      m_twoColumnsLayout.init();
      m_labelValueLayout.init();
    }

    void drawMenu(const ui::MenuView& t_view, const LogicalState& t_lState) {
      switch (t_view.m_layout) {
        case ui::MenuLayout::kList:
          m_listLayout.draw(t_view, t_lState);
          break;

        case ui::MenuLayout::kTwoColumns:
          m_twoColumnsLayout.draw(t_view, t_lState);
          break;

        case ui::MenuLayout::kLabelValue:
          m_labelValueLayout.draw(t_view, t_lState);
          break;

        default:
          break;
      }
    }
};

}
