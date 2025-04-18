#include "services/bypass_service.h"

void BypassService::init() {

}

void BypassService::handleEvent(const Event& t_event) {
  if (t_event.m_type != EventType::kBypassPressed) return;

  m_logicalState.m_bypassState = m_logicalState.m_bypassState == BypassState::kActive
                                  ? BypassState::kBypassed
                                  : BypassState::kActive;

  // Switch bypass relay
  // Switch bypass LED

  Event bypassEvent;
  bypassEvent.m_timestamp = t_event.m_timestamp;
  bypassEvent.m_type = m_logicalState.m_bypassState == BypassState::kActive
                        ? EventType::kBypassEnabled
                        : EventType::kBypassDisabled;

  EventBus::publish(bypassEvent);
}

void BypassService::update() {

}
