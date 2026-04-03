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
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kExpr;
  e.m_action = EventAction::kSave;
  e.m_timestamp = t_event.m_timestamp; // millis()

  EventBus::publish(e);
}

void ExprService::init() {
  syncHandler();
}

void ExprService::handleEvent(const Event& t_event) {
  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) {
      syncHandler();
      return;
    }
  }

  if (t_event.m_domain == EventDomain::kUI
      && t_event.m_subject == EventSubject::kExpr) {
    auto& params = m_logicState.m_exprParams[m_logicState.m_currentProgram];

    switch (t_event.m_id) {
      case static_cast<uint8_t>(ExprParam::kState):
        params.m_state = m_exprHandler.toggleExprState();
        break;

      case static_cast<uint8_t>(ExprParam::kMappedPot):
        params.m_mappedPot = m_exprHandler.changeMappedPot(t_event.m_data.delta);
        break;

      case static_cast<uint8_t>(ExprParam::kDirection):
        params.m_direction = m_exprHandler.toggleDirection();
        break;

      case static_cast<uint8_t>(ExprParam::kHeel):
        params.m_heelValue = m_exprHandler.changeHeelValue(t_event.m_data.delta);
        break;

      case static_cast<uint8_t>(ExprParam::kToe):
        params.m_toeValue = m_exprHandler.changeToeValue(t_event.m_data.delta);
        break;

      default:
        break;
    }

    publishSaveExprEvent(t_event);
    return;
  }

  if (t_event.m_domain == EventDomain::kPhysical) {
    if (m_exprHandler.m_state != ExprState::kActive) return;

    if (t_event.m_subject == EventSubject::kExpr
        && t_event.m_action == EventAction::kValueChanged) {
      Event e;
      e.m_domain = EventDomain::kLogic;
      e.m_subject = EventSubject::kExpr;
      e.m_action = EventAction::kValueChanged;
      e.m_timestamp = t_event.m_timestamp;
      e.m_data.value = m_exprHandler.mapAdcValue(t_event.m_data.value);

      switch (m_exprHandler.m_mappedPot) {
        case MappedPot::kPot0:
          e.m_id = static_cast<uint8_t>(PotId::kPot0);
          break;

        case MappedPot::kPot1:
          e.m_id = static_cast<uint8_t>(PotId::kPot1);
          break;

        case MappedPot::kPot2:
          e.m_id = static_cast<uint8_t>(PotId::kPot2);
          break;

        case MappedPot::kMixPot:
          e.m_id = static_cast<uint8_t>(PotId::kMixPot);
          break;

        default:
          break;
      }

      EventBus::publish(e);
    }
  }
}

void ExprService::update() {

}

bool ExprService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgram
      && t_event.m_action == EventAction::kValueChanged) return true;
  }

  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kExpr) return true;
  }

  if (t_event.m_domain == EventDomain::kPhysical) {
    if (t_event.m_subject == EventSubject::kExpr) return true;
  }

  return false;
}
