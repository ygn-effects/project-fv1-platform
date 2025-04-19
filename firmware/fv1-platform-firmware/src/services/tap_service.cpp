#include "services/tap_service.h"

void TapService::publishTempoEvent(const Event& t_event) const {
  Event e;
  e.m_type = EventType::kTempoChanged;
  e.m_timestamp = t_event.m_timestamp;
  e.m_data.value = m_tapHandler.getDivState() == DivState::kDisabled
                    ? m_tapHandler.getInterval()
                    : m_tapHandler.getDivInterval();

  EventBus::publish(e);
}

void TapService::init() {

}

void TapService::handleEvent(const Event& t_event) {
  if (t_event.m_type != EventType::kTapPressed && t_event.m_type != EventType::kTapLongPressed) return;
  else if (t_event.m_type == EventType::kTapPressed) {
    m_tapHandler.registerTap(t_event.m_timestamp);

    if (m_tapHandler.getIsNewIntervalSet()) {
      m_logicalState.m_interval = m_tapHandler.getInterval();

      publishTempoEvent(t_event);
    }

    m_logicalState.m_tapState = m_tapHandler.getTapState();
  }
  else if (t_event.m_type == EventType::kTapLongPressed && m_logicalState.m_tapState == TapState::kEnabled) {
    m_tapHandler.setNextDivValue();

    m_logicalState.m_divState = m_tapHandler.getDivState();
    m_logicalState.m_divValue = m_tapHandler.getDivValue();
    m_logicalState.m_divInterval = m_tapHandler.getDivInterval();

    publishTempoEvent(t_event);
  }
}

void TapService::update() {

}
