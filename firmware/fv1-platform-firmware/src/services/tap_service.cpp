#include "services/tap_service.h"

void TapService::publishTapIntervalEvent(const Event& t_event) const {
  Event e;
  e.m_type = EventType::kTapIntervalChanged;
  e.m_timestamp = t_event.m_timestamp;
  e.m_data.value = m_tapHandler.getDivState() == DivState::kDisabled
                    ? m_tapHandler.getInterval()
                    : m_tapHandler.getDivInterval();

  EventBus::publish(e);
}

void TapService::init() {
  m_tapHandler.setTapState(m_logicalState.m_tapState);
  m_tapHandler.setDivState(m_logicalState.m_divState);
  m_tapHandler.setDivValue(m_logicalState.m_divValue);
  m_tapHandler.setInterval(m_logicalState.m_interval);
  m_tapHandler.setDivInterval(m_logicalState.m_divInterval);
}

void TapService::handleEvent(const Event& t_event) {
  if (t_event.m_type == EventType::kProgramChanged) {
    if (! m_logicalState.m_activeProgram->m_isDelayEffect || ! m_logicalState.m_activeProgram->m_supportsTap) {
      m_logicalState.m_tapState = TapState::kDisabled;
      m_logicalState.m_divState = DivState::kDisabled;
      m_logicalState.m_divValue = DivValue::kQuarter;
      init();

      return;
    }
  }

  if (! m_logicalState.m_activeProgram->m_supportsTap) return;

  if (t_event.m_type != EventType::kTapPressed && t_event.m_type != EventType::kTapLongPressed) return;
  else if (t_event.m_type == EventType::kTapPressed) {
    m_tapHandler.registerTap(t_event.m_timestamp);

    if (m_tapHandler.getIsNewIntervalSet()) {
      m_logicalState.m_interval = m_tapHandler.getInterval();

      publishTapIntervalEvent(t_event);
    }

    m_logicalState.m_tapState = m_tapHandler.getTapState();
  }
  else if (t_event.m_type == EventType::kTapLongPressed && m_logicalState.m_tapState == TapState::kEnabled) {
    m_tapHandler.setNextDivValue();

    m_logicalState.m_divState = m_tapHandler.getDivState();
    m_logicalState.m_divValue = m_tapHandler.getDivValue();
    m_logicalState.m_divInterval = m_tapHandler.getDivInterval();

    publishTapIntervalEvent(t_event);
  }
}

void TapService::update() {

}
