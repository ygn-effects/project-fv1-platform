#pragma once

#include <stdint.h>

enum class ItemKind : uint8_t {
  kParam,
  kBubMenu,
  kAction
};

struct Menu;

struct MenuItem {
  const char* m_label;
  ItemKind m_kind;
  uint8_t m_paramId;
  const Menu* m_subMenu;
  void (*action)();
};

struct Menu {
  const MenuItem* m_items;
  uint8_t m_count;
};
