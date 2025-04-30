#include "services/bypass_service.h"

void BypassService::init() {

}

void BypassService::handleEvent(const Event& t_event) {
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

bool BypassService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kBypassEvent;
}
