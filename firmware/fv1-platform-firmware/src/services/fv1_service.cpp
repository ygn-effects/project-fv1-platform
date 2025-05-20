#include "services/fv1_service.h"

Fv1Service::Fv1Service(const LogicalState& t_lState, Fv1& t_fv1)
  : m_logicalState(t_lState), m_fv1(t_fv1) {}

void Fv1Service::init() {
  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    m_handler.m_potConfigs[i].m_minLogical = m_logicalState.m_potParams[i]->m_minValue;
    m_handler.m_potConfigs[i].m_maxLogical = m_logicalState.m_potParams[i]->m_maxValue;
  }

  m_fv1.sendProgramChange(m_logicalState.m_currentProgram);

  if (m_logicalState.m_activeProgram->m_isDelayEffect) {
    m_handler.m_tempoConfig.m_minLogical = m_logicalState.m_activeProgram->m_minDelayMs;
    m_handler.m_tempoConfig.m_maxLogical = m_logicalState.m_activeProgram->m_maxDelayMs;
    m_fv1.sendPotValue(Fv1Pot::Pot0, m_handler.mapTempoValue(m_logicalState.m_tempo));
  }
  else {
    m_fv1.sendPotValue(Fv1Pot::Pot0, m_logicalState.m_potParams[0]->m_value);
  }

  m_fv1.sendPotValue(Fv1Pot::Pot1, m_logicalState.m_potParams[1]->m_value);
  m_fv1.sendPotValue(Fv1Pot::Pot2, m_logicalState.m_potParams[2]->m_value);
}

void Fv1Service::handleEvent(const Event& t_event) {
  switch(t_event.m_type) {
    case EventType::kProgramChanged:
    case EventType::kProgramModeChanged:
    case EventType::kPresetBankChanged:
    case EventType::kPresetChanged:
      init();
      break;

    case EventType::kTempoChanged:
      m_fv1.sendPotValue(Fv1Pot::Pot0, m_handler.mapTempoValue(m_logicalState.m_tempo));
      break;

    case EventType::kPot0Moved:
    case EventType::kMenuPot0Moved:
    case EventType::kMidiPot0Moved:
      if (m_logicalState.m_activeProgram->m_isDelayEffect) { return; }
      m_fv1.sendPotValue(Fv1Pot::Pot0, m_logicalState.m_potParams[0]->m_value);
      break;

    case EventType::kPot1Moved:
    case EventType::kMenuPot1Moved:
    case EventType::kMidiPot1Moved:
      m_fv1.sendPotValue(Fv1Pot::Pot1, m_logicalState.m_potParams[1]->m_value);
      break;

    case EventType::kPot2Moved:
    case EventType::kMenuPot2Moved:
    case EventType::kMidiPot2Moved:
      m_fv1.sendPotValue(Fv1Pot::Pot2, m_logicalState.m_potParams[2]->m_value);
      break;

    default:
      break;
  }
}

void Fv1Service::update() {

}

bool Fv1Service::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return (t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kPotEvent)
      || (t_category == EventCategory::kTempoEvent && t_subCategory == EventSubCategory::kTempoChangedEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramChangedEvent)
      || (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuPotEvent)
      || (t_category == EventCategory::kMidiEvent && t_subCategory == EventSubCategory::kMidiPotMovedEvent)
      || t_category == EventCategory::kProgramEvent;
}
