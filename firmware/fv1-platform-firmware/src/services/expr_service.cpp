#include "services/expr_service.h"

void ExprService::syncHandler() {
  auto& params = m_logicState.m_exprParams[m_logicState.m_currentProgram];

  m_exprHandler.m_state = params.m_state;
  m_exprHandler.m_mappedPot = params.m_mappedPot;
  m_exprHandler.m_direction = params.m_direction;
  m_exprHandler.m_heelValue = params.m_heelValue;
  m_exprHandler.m_toeValue = params.m_toeValue;
}

void ExprService::publishSaveExprEvent(const Event& t_event) {
  EventBus::publish({EventType::kSaveExpr, t_event.m_timestamp /*millis*/, {}});
}

void ExprService::init() {
  syncHandler();
}

void ExprService::handleEvent(const Event& t_event) {
  if (t_event.m_type == EventType::kProgramChanged) {
    syncHandler();
    return;
  }

  if (EventToSubCategory(t_event.m_type) == EventSubCategory::kMenuExprEvent) {
    auto& params = m_logicState.m_exprParams[m_logicState.m_currentProgram];

    switch (t_event.m_type) {
      case EventType::kMenuExprStateToggled:
        params.m_state = m_exprHandler.toggleExprState();
        break;

      case EventType::kMenuExprMappedPotMoved:
        params.m_mappedPot = m_exprHandler.changeMappedPot(t_event.m_data.delta);
        break;

      case EventType::kMenuExprDirectionToggled:
        params.m_direction = m_exprHandler.toggleDirection();
        break;

      case EventType::kMenuExprHeelValueMoved:
        params.m_heelValue = m_exprHandler.changeHeelValue(t_event.m_data.delta);
        break;

      case EventType::kMenuExprToeValueMoved:
        params.m_toeValue = m_exprHandler.changeToeValue(t_event.m_data.delta);
        break;
    }

    publishSaveExprEvent(t_event);
    return;
  }

  if (m_exprHandler.m_state != ExprState::kActive) return;

  if (t_event.m_type == EventType::kExprMoved) {
    Event e;
    e.m_timestamp = t_event.m_timestamp;
    e.m_data.value = m_exprHandler.mapAdcValue(t_event.m_data.value);

    switch (m_exprHandler.m_mappedPot) {
      case MappedPot::kPot0:
        e.m_type = EventType::kExprPot0Moved;
        break;

      case MappedPot::kPot1:
        e.m_type = EventType::kExprPot1Moved;
        break;

      case MappedPot::kPot2:
        e.m_type = EventType::kExprPot2Moved;
        break;

      case MappedPot::kMixPot:
        e.m_type = EventType::kExprMixPotMoved;
        break;

      default:
        break;
    }

    EventBus::publish(e);
  }
}

void ExprService::update() {

}

bool ExprService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return (t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kExprEvent)
      || (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuExprEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramChangedEvent);
}
