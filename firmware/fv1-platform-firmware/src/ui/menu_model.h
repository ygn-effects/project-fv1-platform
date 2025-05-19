#pragma once

#include <stdint.h>
#include <stdio.h>
#include "core/event.h"
#include "core/event_bus.h"
#include "core/service.h"
#include "logic/logical_state.h"

namespace ui {

namespace MenuConstants {
  static constexpr uint32_t c_menuTimeout = 30000u;
  static constexpr uint32_t c_potMenuTimeout = 500u;
  static constexpr uint8_t c_visibleItemsPerPage = 5;
  static constexpr uint8_t c_visibleItemsPerTwoColumns = 2 * MenuConstants::c_visibleItemsPerPage;
}

struct MenuPage;

enum class MenuLayout {
  kList,
  kTwoColumns,
  kLabelValue
};

using VisibleFn = bool (*)(const LogicalState*);
using LabelFn = const char* (*)(const LogicalState*);
using ValueFn = const char* (*)(const LogicalState*);
using MoveFn = void (*)(int8_t);
using ClickFn = void (*)();

struct MenuItem {
  LabelFn m_label;
  VisibleFn m_visible;
  ValueFn m_value;
  MoveFn m_onMove;
  ClickFn m_onClick;
  const MenuPage* m_subMenu;
};

struct MenuPage {
  const char* m_header;
  const MenuItem* m_items;
  uint8_t m_count;
  MenuLayout m_layout;
};

struct MenuView {
  const char* m_header;
  const MenuItem* m_items[ui::MenuConstants::c_visibleItemsPerTwoColumns];
  uint8_t m_count;
  uint8_t m_selected;
  MenuLayout m_layout;
  bool m_editing;
};

constexpr bool isAlwaysVisible(const LogicalState*);
constexpr bool visibleIfPresetMode(const LogicalState* t_state);
constexpr bool visibleIfProgramMode(const LogicalState* t_state);
constexpr bool visibleIfDelayEffect(const LogicalState* t_state);
constexpr bool notVisibleIfDelayEffect(const LogicalState* t_state);
constexpr bool visibleIfExprActive(const LogicalState* t_state);

constexpr const char* labelProgram(const LogicalState*);
constexpr const char* labelPresetBank(const LogicalState* t_state);
constexpr const char* labelPresetBankLocked(const LogicalState* t_state);
constexpr const char* labelPreset(const LogicalState* t_state);
constexpr const char* labelPresetLocked(const LogicalState* t_state);
constexpr const char* labelPot0Locked(const LogicalState* t_state);
constexpr const char* labelPot1Locked(const LogicalState* t_state);
constexpr const char* labelPot2Locked(const LogicalState* t_state);
constexpr const char* labelMixPotLocked(const LogicalState* t_state);
constexpr const char* labelPot(const LogicalState* t_state, uint8_t t_potIndex);
constexpr const char* labelPot0(const LogicalState* t_state);
constexpr const char* labelPot1(const LogicalState* t_state);
constexpr const char* labelPot2(const LogicalState* t_state);
constexpr const char* labelMixPot(const LogicalState* t_state);
constexpr const char* labelTempo(const LogicalState* t_state);
constexpr const char* labelTempoLocked(const LogicalState* t_state);
constexpr const char* labelExprLocked(const LogicalState*);
constexpr const char* labelExprSettings(const LogicalState*);
constexpr const char* labelExprState(const LogicalState* t_state);
constexpr const char* labelExprMappedPot(const LogicalState* t_state);
constexpr const char* labelExprDirection(const LogicalState* t_state);
constexpr const char* labelExprHeelValue(const LogicalState* t_state);
constexpr const char* labelExprToeValue(const LogicalState* t_state);

constexpr const char* valueProgram(const LogicalState* t_state);
const char* valuePresetBank(const LogicalState* t_state);
const char* valuePreset(const LogicalState* t_state);
void valuePot(const LogicalState* t_state, uint8_t t_potIndex, char* buffer, size_t bufferSize);
const char* valuePot0(const LogicalState* t_state);
const char* valuePot1(const LogicalState* t_state);
const char* valuePot2(const LogicalState* t_state);
const char* valueMixPot(const LogicalState* t_state);
const char* valueTempo(const LogicalState* t_state);
constexpr const char* valueExprState(const LogicalState* t_state);
constexpr const char* valueExprMappedPot(const LogicalState* t_state);
constexpr const char* valueExprDirection(const LogicalState* t_state);
const char* valueExprHeelValue(const LogicalState* t_state);
const char* valueExprToeValue(const LogicalState* t_state);

void onMoveProgram(int8_t t_delta);
void onMovePot0(int8_t t_delta);
void onMovePot1(int8_t t_delta);
void onMovePot2(int8_t t_delta);
void onMoveMixPot(int8_t t_delta);
void onMoveTempo(int8_t t_delta);
void onMoveExprMappedPot(int8_t t_delta);
void onMoveExprHeelValue(int8_t t_delta);
void onMoveExprToeValue(int8_t t_delta);

void onClickExprState();
void onClickExprDirection();

extern const MenuPage LockScreenMenuPage;
extern const MenuPage ProgramMenuPage;
extern const MenuPage PresetMenuPage;
extern const MenuPage ExprSettingsMenuPage;
extern const MenuPage Pot0ValueMenuPage;
extern const MenuPage Pot1ValueMenuPage;
extern const MenuPage Pot2ValueMenuPage;
extern const MenuPage MixPotValueMenuPage;

} // namespace ui
