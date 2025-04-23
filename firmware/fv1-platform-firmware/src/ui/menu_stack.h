#pragma once

#include <stddef.h>
#include <stdint.h>

template<size_t N>
class MenuStack {
  private:
    const Menu* m_items[N];
    uint8_t m_top;

  public:
    MenuStack() : m_top(0) {}

    bool push(const Menu* t_menu) {
      if (m_top >= N) return false;

      m_items[m_top++] = t_menu;
      return true;
    }

    void pop() {
      if (m_top) --m_top;
    }

    const Menu* top() const {
      return m_top ? m_items[m_top - 1] : nullptr;
    }

    uint8_t depth() const {
      return m_top;
    }
};
