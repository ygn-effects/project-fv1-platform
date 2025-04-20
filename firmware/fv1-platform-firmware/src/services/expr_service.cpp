#include "services/expr_service.h"

void ExprService::syncHandler() {
  auto& params = m_logicState.m_exprParams[m_logicState.m_currentProgram];

  m_exprHandler.setState(params.m_state);
  m_exprHandler.setMappedPot(params.m_mappedPot);
  m_exprHandler.setDirection(params.m_direction);
  m_exprHandler.setHeelToeValues(params.m_heelValue, params.m_toeValue);
}

void ExprService::init() {
  syncHandler();
}

void ExprService::handleEvent(const Event& t_event) {
  if (t_event.m_type == EventType::kProgramChanged) { syncHandler(); return; }
  if (t_event.m_type != EventType::kExprMoved) return;
  if (m_exprHandler.getState() != ExprState::kActive) return;

  Event e;
  e.m_timestamp = t_event.m_timestamp;
  e.m_data.value = m_exprHandler.mapAdcValue(t_event.m_data.value);

  switch (m_exprHandler.getMappedPot()) {
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
