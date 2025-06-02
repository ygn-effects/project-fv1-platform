#include "ui/menu_model.h"

namespace ui {

inline constexpr bool isAlwaysVisible(const LogicalState*) {
  return true;
}

inline constexpr bool visibleIfPresetMode(const LogicalState* t_state) {
  return t_state->m_programMode == ProgramMode::kPreset;
}

inline constexpr bool visibleIfProgramMode(const LogicalState* t_state) {
  return t_state->m_programMode == ProgramMode::kProgram;
}

inline constexpr bool visibleIfDelayEffect(const LogicalState* t_state) {
  return t_state->m_activeProgram->m_isDelayEffect;
}

inline constexpr bool notVisibleIfDelayEffect(const LogicalState* t_state) {
  return ! t_state->m_activeProgram->m_isDelayEffect;
}

inline constexpr bool visibleIfExprActive(const LogicalState* t_state) {
  return t_state->m_exprParams[t_state->m_currentProgram].m_state == ExprState::kActive;
}

inline constexpr bool visibleIfDivEnabled(const LogicalState* t_state) {
  return t_state->m_divState == DivState::kEnabled;
}

inline constexpr const char* labelProgram(const LogicalState*) {
  return "Prog";
}

inline constexpr const char* valueProgram(const LogicalState* t_state) {
  return t_state->m_activeProgram->m_name;
}

inline constexpr const char* labelPresetBank(const LogicalState* t_state) {
  return "Bank";
}

inline constexpr const char* labelPresetBankLocked(const LogicalState* t_state) {
  return "B";
}

inline constexpr const char* labelPreset(const LogicalState* t_state) {
  return "Preset";
}

inline constexpr const char* labelPresetLocked(const LogicalState* t_state) {
  return "P";
}

inline constexpr const char* labelPot0Locked(const LogicalState* t_state) {
  return "P0";
}

inline constexpr const char* labelPot1Locked(const LogicalState* t_state) {
  return "P1";
}

inline constexpr const char* labelPot2Locked(const LogicalState* t_state) {
  return "P2";
}

inline constexpr const char* labelMixPotLocked(const LogicalState* t_state) {
  return "MX";
}

inline constexpr const char* labelPot(const LogicalState* t_state, uint8_t t_potIndex) {
  return t_state->m_activeProgram->m_params[t_potIndex].m_label;
}

inline constexpr const char* labelPot0(const LogicalState* t_state) {
  return labelPot(t_state, 0);
}

inline constexpr const char* labelPot1(const LogicalState* t_state) {
  return labelPot(t_state, 1);
}

inline constexpr const char* labelPot2(const LogicalState* t_state) {
  return labelPot(t_state, 2);
}

inline constexpr const char* labelMixPot(const LogicalState* t_state) {
  return labelPot(t_state, 3);
}

inline constexpr const char* labelTempo(const LogicalState* t_state) {
  return "Tempo";
}

inline constexpr const char* labelTempoLocked(const LogicalState* t_state) {
  return "T";
}

inline constexpr const char* labelExprLocked(const LogicalState*) {
  return "Expr";
}

inline constexpr const char* labelExprSettings(const LogicalState*) {
  return "Expression settings";
}

inline constexpr const char* labelExprState(const LogicalState* t_state) {
  return "State";
}

inline constexpr const char* labelExprMappedPot(const LogicalState* t_state) {
  return "Mapped Pot";
}

inline constexpr const char* labelExprDirection(const LogicalState* t_state) {
  return "Direction";
}

inline constexpr const char* labelExprHeelValue(const LogicalState* t_state) {
  return "Heel value";
}

inline constexpr const char* labelExprToeValue(const LogicalState* t_state) {
  return "Toe value";
}

inline constexpr const char* labelDivValue(const LogicalState* t_state) {
  return "Div";
}

inline constexpr const char* labelDivValueLocked(const LogicalState* t_state) {
  return "D";
}

inline constexpr const char* labelPot0State(const LogicalState* t_state) {
  return "P0 State";
}

inline constexpr const char* labelPot1State(const LogicalState* t_state) {
  return "P1 State";
}

inline constexpr const char* labelPot2State(const LogicalState* t_state) {
  return "P2 State";
}

inline constexpr const char* labelMixPotState(const LogicalState* t_state) {
  return "Mix State";
}

inline constexpr const char* labelPot0MinValue(const LogicalState* t_state) {
  return "P0 Min";
}

inline constexpr const char* labelPot1MinValue(const LogicalState* t_state) {
  return "P1 Min";
}

inline constexpr const char* labelPot2MinValue(const LogicalState* t_state) {
  return "P2 Min";
}

inline constexpr const char* labelMixPotMinValue(const LogicalState* t_state) {
  return "Mix Min";
}

inline constexpr const char* labelPot0MaxValue(const LogicalState* t_state) {
  return "P0 Max";
}

inline constexpr const char* labelPot1MaxValue(const LogicalState* t_state) {
  return "P1 Max";
}

inline constexpr const char* labelPot2MaxValue(const LogicalState* t_state) {
  return "P2 Max";
}

inline constexpr const char* labelMixPotMaxValue(const LogicalState* t_state) {
  return "Mix Max";
}

inline constexpr const char* labelPotSettings(const LogicalState* t_state) {
  return "Pot settings";
}

const char* valuePresetBank(const LogicalState* t_state) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u", t_state->m_currentPresetBank);

  return buffer;
}

const char* valuePreset(const LogicalState* t_state) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u", t_state->m_currentPreset);

  return buffer;
}

void valuePot(const LogicalState* t_state, uint8_t t_potIndex, char* buffer, size_t bufferSize) {
  const ProgramParameter& param = t_state->m_activeProgram->m_params[t_potIndex];

  switch (param.m_unit) {
    case ParamUnit::kHz:
      snprintf(buffer, bufferSize, "%u Hz", t_state->m_potParams[t_state->m_currentProgram][t_potIndex].m_value);
      break;

    case ParamUnit::kMs:
      snprintf(buffer, bufferSize, "%u ms", t_state->m_potParams[t_state->m_currentProgram][t_potIndex].m_value);
      break;

    case ParamUnit::kPercent:
      snprintf(buffer, bufferSize, "%u %%", t_state->m_potParams[t_state->m_currentProgram][t_potIndex].m_value);
      break;

    case ParamUnit::kNone:
    case ParamUnit::kRaw:
      snprintf(buffer, bufferSize, "%u", t_state->m_potParams[t_state->m_currentProgram][t_potIndex].m_value);
      break;

    default:
      snprintf(buffer, bufferSize, "ERR");
      break;
  }
}

const char* valuePot0(const LogicalState* t_state) {
  static char valuePot0[8];
  valuePot(t_state, 0, valuePot0, sizeof(valuePot0));

  return valuePot0;
}

const char* valuePot1(const LogicalState* t_state) {
  static char valuePot1[8];
  valuePot(t_state, 1, valuePot1, sizeof(valuePot1));

  return valuePot1;
}

const char* valuePot2(const LogicalState* t_state) {
  static char valuePot2[8];
  valuePot(t_state, 2, valuePot2, sizeof(valuePot2));

  return valuePot2;
}

const char* valueMixPot(const LogicalState* t_state) {
  static char valueMixPot[8];
  snprintf(valueMixPot, sizeof(valueMixPot), "%u ms", t_state->m_potParams[t_state->m_currentProgram][3].m_maxValue);

  return valueMixPot;
}

const char* valueTempo(const LogicalState* t_state) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u ms", t_state->m_tempo);

  return buffer;
}

inline constexpr const char* valueExprState(const LogicalState* t_state) {
  return t_state->m_exprParams[t_state->m_currentProgram].m_state == ExprState::kActive
    ? "ON"
    : "OFF";
}

inline constexpr const char* valueExprMappedPot(const LogicalState* t_state) {
  switch (t_state->m_exprParams[t_state->m_currentProgram].m_mappedPot) {
    case MappedPot::kPot0:
      return "Pot0";

    case MappedPot::kPot1:
      return "Pot1";

    case MappedPot::kPot2:
      return "Pot2";

    case MappedPot::kMixPot:
      return "MixPot";

    default:
      return "Potx";
  }
}

inline constexpr const char* valueExprDirection(const LogicalState* t_state) {
  return t_state->m_exprParams[t_state->m_currentProgram].m_direction == Direction::kNormal
    ? "Normal"
    : "Reversed";
}

const char* valueExprHeelValue(const LogicalState* t_state) {
  static char buffer[6];
  snprintf(buffer, sizeof(buffer), "%u", t_state->m_exprParams[t_state->m_currentProgram].m_heelValue);

  return buffer;
}

const char* valueExprToeValue(const LogicalState* t_state) {
  static char buffer[6];
  snprintf(buffer, sizeof(buffer), "%u", t_state->m_exprParams[t_state->m_currentProgram].m_toeValue);

  return buffer;
}

inline constexpr const char* valueDivValue(const LogicalState* t_state) {
  switch (t_state->m_divValue) {
    case DivValue::kEight:
      return "/2";

    case DivValue::kSixteenth:
      return "/4";

    case DivValue::kDottedEight:
      return "*3/4";

    case DivValue::kEightTriplet:
      return "/3";

    default:
      return "/1";
  }
}

inline constexpr const char* valuePotState(const LogicalState* t_state, uint8_t t_potIndex) {
  return t_state->m_potParams[t_state->m_currentProgram][t_potIndex].m_state == PotState::kActive
    ? "ON"
    : "OFF";
}

inline constexpr const char* valuePot0State(const LogicalState* t_state) {
  return valuePotState(t_state, 0);
}

inline constexpr const char* valuePot1State(const LogicalState* t_state) {
  return valuePotState(t_state, 1);
}

inline constexpr const char* valuePot2State(const LogicalState* t_state) {
  return valuePotState(t_state, 2);
}

inline constexpr const char* valueMixPotState(const LogicalState* t_state) {
  return valuePotState(t_state, 3);
}

const char* valuePot0MinValue(const LogicalState* t_state) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u", t_state->m_potParams[t_state->m_currentProgram][0].m_minValue);

  return buffer;
}

const char* valuePot1MinValue(const LogicalState* t_state) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u", t_state->m_potParams[t_state->m_currentProgram][1].m_minValue);

  return buffer;
}

const char* valuePot2MinValue(const LogicalState* t_state) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u", t_state->m_potParams[t_state->m_currentProgram][2].m_minValue);

  return buffer;
}

const char* valueMixPotMinValue(const LogicalState* t_state) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u", t_state->m_potParams[t_state->m_currentProgram][3].m_minValue);

  return buffer;
}

const char* valuePot0MaxValue(const LogicalState* t_state) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u", t_state->m_potParams[t_state->m_currentProgram][0].m_maxValue);

  return buffer;
}

const char* valuePot1MaxValue(const LogicalState* t_state) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u", t_state->m_potParams[t_state->m_currentProgram][1].m_maxValue);

  return buffer;
}

const char* valuePot2MaxValue(const LogicalState* t_state) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u", t_state->m_potParams[t_state->m_currentProgram][2].m_maxValue);

  return buffer;
}

const char* valueMixPotMaxValue(const LogicalState* t_state) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u", t_state->m_potParams[t_state->m_currentProgram][3].m_maxValue);

  return buffer;
}

void onMoveProgram(int8_t t_delta) {
  Event e;
  e.m_type = EventType::kMenuProgramChanged;
  e.m_timestamp = 0; /*millis()*/
  e.m_data.delta = t_delta;
  EventBus::publish(e);
}

void onMovePot0(int8_t t_delta) {
  Event e;
  e.m_type = EventType::kMenuPot0Moved;
  e.m_timestamp = 0; /*millis()*/
  e.m_data.delta = t_delta;
  EventBus::publish(e);
}

void onMovePot1(int8_t t_delta) {
  Event e;
  e.m_type = EventType::kMenuPot1Moved;
  e.m_timestamp = 0; /*millis()*/
  e.m_data.delta = t_delta;
  EventBus::publish(e);
}

void onMovePot2(int8_t t_delta) {
  Event e;
  e.m_type = EventType::kMenuPot2Moved;
  e.m_timestamp = 0; /*millis()*/
  e.m_data.delta = t_delta;
  EventBus::publish(e);
}

void onMoveMixPot(int8_t t_delta) {
  Event e;
  e.m_type = EventType::kMenuMixPotMoved;
  e.m_timestamp = 0; /*millis()*/
  e.m_data.delta = t_delta;
  EventBus::publish(e);
}

void onMoveTempo(int8_t t_delta) {
  Event e;
  e.m_type = EventType::kMenuTempoChanged;
  e.m_timestamp = 0; /*millis()*/
  e.m_data.delta = t_delta;
  EventBus::publish(e);
}

void onMoveExprMappedPot(int8_t t_delta) {
  Event e;
  e.m_type = EventType::kMenuExprMappedPotMoved;
  e.m_timestamp = 0; /*millis()*/
  e.m_data.delta = t_delta;
  EventBus::publish(e);
}

void onMoveExprHeelValue(int8_t t_delta) {
  Event e;
  e.m_type = EventType::kMenuExprHeelValueMoved;
  e.m_timestamp = 0; /*millis()*/
  e.m_data.delta = t_delta;
  EventBus::publish(e);
}

void onMoveExprToeValue(int8_t t_delta) {
  Event e;
  e.m_type = EventType::kMenuExprToeValueMoved;
  e.m_timestamp = 0; /*millis()*/
  e.m_data.delta = t_delta;
  EventBus::publish(e);
}

void onMoveDivValue(int8_t t_delta) {
  Event e;
  e.m_type = EventType::kMenuDivValueMoved;
  e.m_timestamp = 0; /*millis()*/
  e.m_data.delta = t_delta;
  EventBus::publish(e);
}

void onClickExprState() {
  EventBus::publish({EventType::kMenuExprStateToggled, 0 /*millis()*/, {}});
}

void onClickExprDirection() {
  EventBus::publish({EventType::kMenuExprDirectionToggled, 0 /*millis()*/, {}});
}

constexpr MenuItem lockScreenMenuItems[] = {
  { labelProgram, isAlwaysVisible, valueProgram, nullptr, nullptr, nullptr },
  { labelPresetBankLocked, visibleIfPresetMode, valuePresetBank, nullptr, nullptr, nullptr },
  { labelPresetLocked, visibleIfPresetMode, valuePreset, nullptr, nullptr, nullptr },
  { labelTempoLocked, visibleIfDelayEffect, valueTempo, nullptr, nullptr, nullptr },
  { labelDivValueLocked, visibleIfDivEnabled, valueDivValue, nullptr, nullptr, nullptr },
  { labelPot0Locked, notVisibleIfDelayEffect, valuePot0, nullptr, nullptr, nullptr },
  { labelPot1Locked, isAlwaysVisible, valuePot1, nullptr, nullptr, nullptr },
  { labelPot2Locked, isAlwaysVisible, valuePot2, nullptr, nullptr, nullptr },
  { labelMixPot, isAlwaysVisible, valueMixPot, nullptr, nullptr, nullptr },
  { labelExprLocked, visibleIfExprActive, valueExprMappedPot, nullptr, nullptr, nullptr }
};

constexpr MenuPage LockScreenMenuPage = {
  "Lock screen",
  lockScreenMenuItems,
  sizeof(lockScreenMenuItems) / sizeof(lockScreenMenuItems[0]),
  ui::MenuLayout::kTwoColumns
};

constexpr MenuItem ProgramMenuItems[] = {
  { labelProgram, isAlwaysVisible, valueProgram, onMoveProgram, nullptr, nullptr },
  { labelTempo, visibleIfDelayEffect, valueTempo, onMoveTempo, nullptr, nullptr },
  { labelDivValue, visibleIfDivEnabled, valueDivValue, onMoveDivValue, nullptr, nullptr },
  { labelPot0, notVisibleIfDelayEffect, valuePot0, onMovePot0, nullptr, nullptr },
  { labelPot1, isAlwaysVisible, valuePot1, onMovePot1, nullptr, nullptr },
  { labelPot2, isAlwaysVisible, valuePot2, onMovePot2, nullptr, nullptr },
  { labelMixPot, isAlwaysVisible, valueMixPot, onMoveMixPot, nullptr, nullptr },
  { labelExprSettings, isAlwaysVisible, nullptr, nullptr, nullptr, &ExprSettingsMenuPage },
  { labelPotSettings, isAlwaysVisible, nullptr, nullptr, nullptr, &PotSettingsMenuPage }
};

constexpr MenuPage ProgramMenuPage = {
  "Program mode",
  ProgramMenuItems,
  sizeof(ProgramMenuItems) / sizeof(ProgramMenuItems[0]),
  ui::MenuLayout::kList
};

constexpr MenuItem PresetMenuItems[] = {

};

constexpr MenuPage PresetMenuPage = {
  "Preset mode",
  PresetMenuItems,
  0
};

constexpr MenuItem ExprSettingsMenuItems[] = {
  { labelExprState, isAlwaysVisible, valueExprState, nullptr, onClickExprState, nullptr },
  { labelExprMappedPot, visibleIfExprActive, valueExprMappedPot, onMoveExprMappedPot, nullptr, nullptr },
  { labelExprDirection, visibleIfExprActive, valueExprDirection, nullptr, onClickExprDirection, nullptr },
  { labelExprHeelValue, visibleIfExprActive, valueExprHeelValue, onMoveExprHeelValue, nullptr, nullptr },
  { labelExprToeValue, visibleIfExprActive, valueExprToeValue, onMoveExprToeValue, nullptr, nullptr }
};

constexpr MenuPage ExprSettingsMenuPage = {
  "Expression settings",
  ExprSettingsMenuItems,
  sizeof(ExprSettingsMenuItems) / sizeof(ExprSettingsMenuItems[0]),
  ui::MenuLayout::kList
};

constexpr MenuItem Pot0ValueMenuItems[] = {
  { labelTempo, visibleIfDelayEffect, valueTempo, onMoveTempo, nullptr, nullptr },
  { labelPot0, notVisibleIfDelayEffect, valuePot0, onMovePot0, nullptr, nullptr }
};

constexpr MenuPage Pot0ValueMenuPage = {
  "P0 Setting",
  Pot0ValueMenuItems,
  sizeof(Pot0ValueMenuItems) / sizeof(Pot0ValueMenuItems[0]),
  ui::MenuLayout::kLabelValue
};

constexpr MenuItem Pot1ValueMenuItems[] = {
  { labelPot1, isAlwaysVisible, valuePot1, onMovePot1, nullptr, nullptr }
};

constexpr MenuPage Pot1ValueMenuPage = {
  "P1 Setting",
  Pot1ValueMenuItems,
  sizeof(Pot1ValueMenuItems) / sizeof(Pot1ValueMenuItems[0]),
  ui::MenuLayout::kLabelValue
};

constexpr MenuItem Pot2ValueMenuItems[] = {
  { labelPot2, isAlwaysVisible, valuePot2, onMovePot2, nullptr, nullptr }
};

constexpr MenuPage Pot2ValueMenuPage = {
  "P2 Setting",
  Pot2ValueMenuItems,
  sizeof(Pot2ValueMenuItems) / sizeof(Pot2ValueMenuItems[0]),
  ui::MenuLayout::kLabelValue
};

constexpr MenuItem MixPotValueMenuItems[] = {
  { labelMixPot, isAlwaysVisible, valueMixPot, onMoveMixPot, nullptr, nullptr }
};

constexpr MenuPage MixPotValueMenuPage = {
  "Mix Setting",
  MixPotValueMenuItems,
  sizeof(MixPotValueMenuItems) / sizeof(MixPotValueMenuItems[0]),
  ui::MenuLayout::kLabelValue
};

constexpr MenuItem TempoMenuItems[] = {
  { labelTempo, isAlwaysVisible, valueTempo, onMoveTempo, nullptr, nullptr }
};

constexpr MenuPage TempoMenuPage = {
  "Tempo",
  TempoMenuItems,
  sizeof(TempoMenuItems) / sizeof(TempoMenuItems[0]),
  ui::MenuLayout::kLabelValue
};

constexpr MenuItem PotSettingsMenuItems[] = {
  { labelPot0State, isAlwaysVisible, valuePot0State, nullptr, nullptr, nullptr },
  { labelPot0MinValue, isAlwaysVisible, valuePot0MinValue, nullptr, nullptr, nullptr },
  { labelPot0MaxValue, isAlwaysVisible, valuePot0MaxValue, nullptr, nullptr, nullptr },
  { labelPot1State, isAlwaysVisible, valuePot1State, nullptr, nullptr, nullptr },
  { labelPot1MinValue, isAlwaysVisible, valuePot1MinValue, nullptr, nullptr, nullptr },
  { labelPot1MaxValue, isAlwaysVisible, valuePot1MaxValue, nullptr, nullptr, nullptr },
  { labelPot2State, isAlwaysVisible, valuePot1State, nullptr, nullptr, nullptr },
  { labelPot2MinValue, isAlwaysVisible, valuePot1MinValue, nullptr, nullptr, nullptr },
  { labelPot2MaxValue, isAlwaysVisible, valuePot1MaxValue, nullptr, nullptr, nullptr },
  { labelMixPotState, isAlwaysVisible, valueMixPotState, nullptr, nullptr, nullptr },
  { labelMixPotMinValue, isAlwaysVisible, valueMixPotMinValue, nullptr, nullptr, nullptr },
  { labelMixPotMaxValue, isAlwaysVisible, valueMixPotMaxValue, nullptr, nullptr, nullptr }
};

constexpr MenuPage PotSettingsMenuPage = {
  "Pot settings",
  PotSettingsMenuItems,
  sizeof(PotSettingsMenuItems) / sizeof(PotSettingsMenuItems[0]),
  ui::MenuLayout::kList
};

}
