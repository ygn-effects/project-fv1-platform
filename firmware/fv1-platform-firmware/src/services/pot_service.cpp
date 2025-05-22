#include "services/pot_service.h"

void PotService::syncHandler(uint8_t t_potIndex) {
  m_handler.m_state[t_potIndex] = m_logicalState.m_potParams[m_logicalState.m_currentProgram][t_potIndex].m_state;
  m_handler.m_minValue[t_potIndex] = m_logicalState.m_potParams[m_logicalState.m_currentProgram][t_potIndex].m_minValue;
  m_handler.m_maxValue[t_potIndex] = m_logicalState.m_potParams[m_logicalState.m_currentProgram][t_potIndex].m_maxValue;
}

void PotService::publishSavePotEvent(uint8_t t_potIndex) {
  Event e;
  e.m_type = EventType::kSavePot;
  e.m_timestamp = 0; /*millis()*/
  e.m_data.value = t_potIndex;

  EventBus::publish(e);
}

void PotService::handlePhysicalEvent(const Event& t_event) {
  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  uint8_t potIndex = static_cast<uint8_t>(t_event.m_type) - static_cast<uint8_t>(EventType::kPot0Moved);
  params[potIndex].m_value = m_handler.mapAdcValue(t_event.m_data.value, potIndex);
}

void PotService::handleMenuEvent(const Event& t_event) {
  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  uint8_t potIndex = static_cast<uint8_t>(t_event.m_type) - static_cast<uint8_t>(EventType::kMenuPot0Moved);
  params[potIndex].m_value = m_handler.mapMenuValue(params[potIndex].m_value, t_event.m_data.delta, potIndex);
}

void PotService::handleMidiEvent(const Event& t_event) {
  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  uint8_t potIndex = static_cast<uint8_t>(t_event.m_type) - static_cast<uint8_t>(EventType::kMidiPot0Moved);
  params[potIndex].m_value = m_handler.mapMidiValue(t_event.m_data.value, potIndex);
}

void PotService::handleExprEvent(const Event& t_event) {
  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  uint8_t potIndex = static_cast<uint8_t>(t_event.m_type) - static_cast<uint8_t>(EventType::kExprPot0Moved);
  params[potIndex].m_value = t_event.m_data.value;
}

void PotService::handleMenuPotStateToggleEvent(const Event& t_event) {
  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  uint8_t potIndex = static_cast<uint8_t>(t_event.m_type) - static_cast<uint8_t>(EventType::kMenuPot0StateToggled);
  params[potIndex].m_state = m_handler.togglePotState(potIndex);

  publishSavePotEvent(potIndex);
}

void PotService::handleMenuPotMinValueMove(const Event& t_event) {
  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  uint8_t potIndex = static_cast<uint8_t>(t_event.m_type) - static_cast<uint8_t>(EventType::kMenuPot0MinValueMoved);
  params[potIndex].m_minValue = m_handler.changePotMinValue(t_event.m_data.delta, potIndex);

  publishSavePotEvent(potIndex);
}

void PotService::handleMenuPotMaxValueMove(const Event& t_event) {
  auto& params = m_logicalState.m_potParams[m_logicalState.m_currentProgram];
  uint8_t potIndex = static_cast<uint8_t>(t_event.m_type) - static_cast<uint8_t>(EventType::kMenuPot0MaxValueMoved);
  params[potIndex].m_maxValue = m_handler.changePotMaxValue(t_event.m_data.delta, potIndex);

  publishSavePotEvent(potIndex);
}

void PotService::init() {
  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    syncHandler(i);
  }
}

void PotService::handleEvent(const Event& t_event) {
  if (t_event.m_type == EventType::kProgramChanged) { init(); return; }

  if (EventToSubCategory(t_event.m_type) == EventSubCategory::kMenuPotSettingsEvent) {
    switch (t_event.m_type) {

      case EventType::kMenuPot0StateToggled:
      case EventType::kMenuPot1StateToggled:
      case EventType::kMenuPot2StateToggled:
      case EventType::kMenuMixPotStateToggled:
        handleMenuPotStateToggleEvent(t_event);
        break;

      case EventType::kMenuPot0MinValueMoved:
      case EventType::kMenuPot1MinValueMoved:
      case EventType::kMenuPot2MinValueMoved:
      case EventType::kMenuMixPotMinValueMoved:
        handleMenuPotMinValueMove(t_event);
        break;

      case EventType::kMenuPot0MaxValueMoved:
      case EventType::kMenuPot1MaxValueMoved:
      case EventType::kMenuPot2MaxValueMoved:
      case EventType::kMenuMixPotMaxValueMoved:
        handleMenuPotMaxValueMove(t_event);
        break;

      default:
        break;
    }

    return;
  }

  switch (EventToSubCategory(t_event.m_type)) {
    case EventSubCategory::kPotEvent:
      handlePhysicalEvent(t_event);
      break;

    case EventSubCategory::kMenuPotEvent:
      handleMenuEvent(t_event);
      break;

    case EventSubCategory::kMidiPotMovedEvent:
      handleMidiEvent(t_event);
      break;

    case EventSubCategory::kExprPotEvent:
      handleExprEvent(t_event);
      break;

    default:
      break;
  }
}

void PotService::update() {

}

bool PotService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return (t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kPotEvent)
      || (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuPotEvent)
      || (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuPotSettingsEvent)
      || (t_category == EventCategory::kMidiEvent && t_subCategory == EventSubCategory::kMidiPotMovedEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramChangedEvent);
}
