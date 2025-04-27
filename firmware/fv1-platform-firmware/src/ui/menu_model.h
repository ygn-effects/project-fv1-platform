#pragma once

#include <stdint.h>
#include <stdio.h>
#include "core/event.h"
#include "core/event_bus.h"
#include "core/service.h"
#include "logic/logical_state.h"

namespace ui {

struct MenuPage;

using VisibleFn = bool (*)(const LogicalState*);
using LabelFn = const char* (*)(const LogicalState*);
using ValueFn = const char* (*)(const LogicalState*);
using MoveFn = void (*)(int8_t);
using clickFn = void (*)();

struct MenuItem {
  LabelFn m_label;
  VisibleFn m_visible;
  ValueFn m_value;
  MoveFn m_onMove;
  clickFn m_onClick;
  const MenuPage* m_subMenu;
};

struct MenuPage {
  const char* m_header;
  const MenuItem* m_items;
  uint8_t m_count;
};

constexpr bool isAlwaysVisible(const LogicalState*) {
  return true;
}

constexpr const char* labelProgram(const LogicalState*) {
  return "Program";
}

constexpr const char* valueProgram(const LogicalState* t_state) {
  return t_state->m_activeProgram->m_name;
}

constexpr const char* labelPot(const LogicalState* t_state, uint8_t t_potIndex) {
  return t_state->m_activeProgram->m_params[t_potIndex].m_label;
}

constexpr const char* labelPot0(const LogicalState* t_state) {
  return labelPot(t_state, 0);
}

constexpr const char* labelPot1(const LogicalState* t_state) {
  return labelPot(t_state, 1);
}

constexpr const char* labelPot2(const LogicalState* t_state) {
  return labelPot(t_state, 2);
}

constexpr const char* labelMixPot(const LogicalState* t_state) {
  return labelPot(t_state, 3);
}

void valuePot(const LogicalState* t_state, uint8_t t_potIndex, char* buffer, size_t bufferSize) {
  const ProgramParameter& param = t_state->m_activeProgram->m_params[t_potIndex];

  switch (param.m_unit) {
    case ParamUnit::kHz:
      snprintf(buffer, bufferSize, "%u Hz", t_state->m_potParams[t_potIndex].m_value);
      break;

    case ParamUnit::kMs:
      snprintf(buffer, bufferSize, "%u ms", t_state->m_potParams[t_potIndex].m_value);
      break;

    case ParamUnit::kPercent:
      snprintf(buffer, bufferSize, "%u %%", t_state->m_potParams[t_potIndex].m_value);
      break;

    case ParamUnit::kNone:
    case ParamUnit::kRaw:
      snprintf(buffer, bufferSize, "%u", t_state->m_potParams[t_potIndex].m_value);
      break;

    default:
      snprintf(buffer, bufferSize, "ERR");
      break;
  }
}

const char* valuePot0(const LogicalState* t_state) {
  static char potLabel0[8];
  valuePot(t_state, 0, potLabel0, size_t(potLabel0));

  return potLabel0;
}

const char* valuePot1(const LogicalState* t_state) {
  static char potLabel1[8];
  valuePot(t_state, 1, potLabel1, size_t(potLabel1));

  return potLabel1;
}

const char* valuePot2(const LogicalState* t_state) {
  static char potLabel2[8];
  valuePot(t_state, 2, potLabel2, size_t(potLabel2));

  return potLabel2;
}

const char* valueMixPot(const LogicalState* t_state) {
  static char mixPotLabel[8];
  valuePot(t_state, 3, mixPotLabel, size_t(mixPotLabel));

  return mixPotLabel;
}

constexpr const char* labelTempo(const LogicalState* t_state) {
  return "Tempo";
}

const char* valueTempo(const LogicalState* t_state) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u ms", t_state->m_tempo);

  return buffer;
}

constexpr MenuItem lockScreenMenuItems[] = {

};

constexpr MenuPage LockScreenMenu = {
  "Lock screen",
  lockScreenMenuItems,
  0
};

constexpr MenuItem ProgramMenuItems[] = {
  { labelProgram, isAlwaysVisible, valueProgram, nullptr, nullptr, nullptr },
  { labelTempo, [](const LogicalState* t_s) { return t_s->m_activeProgram->m_isDelayEffect; }, valueTempo, nullptr, nullptr, nullptr },
  { labelPot0, [](const LogicalState* t_s) { return ! t_s->m_activeProgram->m_isDelayEffect; }, valuePot0, nullptr, nullptr, nullptr },
  { labelPot1, isAlwaysVisible, valuePot1, nullptr, nullptr, nullptr },
  { labelPot2, isAlwaysVisible, valuePot2, nullptr, nullptr, nullptr },
  { labelMixPot, isAlwaysVisible, valueMixPot, nullptr, nullptr, nullptr }
};

constexpr MenuPage ProgramMenuPage = {
  "Program mode",
  ProgramMenuItems,
  sizeof(ProgramMenuItems) / sizeof(ProgramMenuItems[0])
};

constexpr MenuItem presetMenuItems[] = {

};

constexpr MenuPage PresetMenuPage = {
  "Preset mode",
  presetMenuItems,
  0
};

} // namespace ui
