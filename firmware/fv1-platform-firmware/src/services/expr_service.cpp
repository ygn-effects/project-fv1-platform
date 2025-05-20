#include "services/expr_service.h"

void ExprService::syncHandler() {
  auto& params = m_logicState.m_exprParams[m_logicState.m_currentProgram];

  m_exprHandler.m_state = params.m_state;
  m_exprHandler.m_mappedPot = params.m_mappedPot;
  m_exprHandler.m_direction = params.m_direction;
  m_exprHandler.m_heelValue = params.m_heelValue;
  m_exprHandler.m_toeValue = params.m_toeValue;
}

void ExprService::init() {
  syncHandler();
}

void ExprService::handleEvent(const Event& t_event) {
  if (t_event.m_type == EventType::kProgramChanged || t_event.m_type == EventType::kProgramModeChanged) {
    syncHandler();
    return;
  }

  if (m_exprHandler.m_state != ExprState::kActive) return;

  Event e;
  e.m_timestamp = t_event.m_timestamp;
  e.m_data.value = m_exprHandler.mapAdcValue(t_event.m_data.value);

  switch (m_exprHandler.m_mappedPot) {
    case MappedPot::kPot0:
      e.m_type = EventType::kPot0Moved;
      break;

    case MappedPot::kPot1:
      e.m_type = EventType::kPot1Moved;
      break;

    case MappedPot::kPot2:
      e.m_type = EventType::kPot2Moved;
      break;

    case MappedPot::kMixPot:
      e.m_type = EventType::kMixPotMoved;
      break;

    default:
      e.m_type = EventType::kPot0Moved;
      break;
  }

  EventBus::publish(e);
}

void ExprService::update() {

}

bool ExprService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return (t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kExprEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramChangedEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramModeChangedEvent);
}
