#pragma once

#include <stdint.h>

enum class ItemKind : uint8_t {
  kParam,
  kSubMenu,
  kAction
};

enum class ParamId : uint8_t {
  kNone,
  kProgramName,
  kPot0Value,
  kPot1Value,
  kPot2Value,
  kMixPotValue,
  kTempo,
  kDivValue,
  kExprDirection,
  kExprMappedPot,
  kExprHeel,
  kExprToe
};

struct Menu;

struct MenuItem {
  const char* m_label;
  ItemKind m_kind;
  ParamId m_paramId;
  uint16_t m_value;
  ParamUnit m_unit;
  const Menu* m_subMenu;
  void (*action)();
};

struct Menu {
  const MenuItem* m_items;
  uint8_t m_count;
};
