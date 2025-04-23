#pragma once

#include <stdint.h>

enum class ItemKind : uint8_t {
  kParam,
  kSubMenu,
  kAction
};

enum class  ParamKind : uint8_t {
  kProgramName,
  kPot0Value,
  kPot1Value,
  kPot2Value,
  kMixPotValue,
  kTempo
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
