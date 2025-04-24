#include "services/tempo_service.h"

TempoService::TempoService(LogicalState& t_lState) :
    m_logicState(t_lState), m_interval(0), m_source(TempoSource::kPot),
    m_minInterval(0), m_maxInterval(0) {}

void TempoService::publishTempoEvent(const Event& t_event) const {
  EventBus::publish({EventType::kTempoChanged, t_event.m_timestamp, {.value = m_interval}});
}

void TempoService::init() {
  m_interval = m_logicState.m_tempo;
  m_minInterval = m_logicState.m_activeProgram->m_minDelayMs;
  m_maxInterval = m_logicState.m_activeProgram->m_maxDelayMs;
}

void TempoService::handleEvent(const Event& t_event) {
  if (t_event.m_type != EventType::kTapIntervalChanged
      && t_event.m_type != EventType::kPot0Moved
      && t_event.m_type != EventType::kProgramChanged) return;

  switch (t_event.m_type) {
    case EventType::kTapIntervalChanged:
      m_interval = Utils::clamp<uint16_t>(t_event.m_data.value, m_minInterval, m_maxInterval);
      m_source = TempoSource::kTap;

      m_logicState.m_tempo = m_interval;
      publishTempoEvent(t_event);
      break;

    case EventType::kPot0Moved:
      m_interval = Utils::mapValue<uint16_t>(t_event.m_data.value, 0, 1023, m_minInterval, m_maxInterval);
      m_source = TempoSource::kPot;

      m_logicState.m_tempo = m_interval;
      publishTempoEvent(t_event);
      break;

    case EventType::kProgramChanged:
      if (m_logicState.m_activeProgram->m_isDelayEffect) {
        m_minInterval = m_logicState.m_activeProgram->m_minDelayMs;
        m_maxInterval = m_logicState.m_activeProgram->m_maxDelayMs;
        m_interval = Utils::clamp<uint16_t>(m_interval, m_minInterval, m_maxInterval);

        m_logicState.m_tempo = m_interval;
        publishTempoEvent(t_event);
      }
      break;

    default:
      break;
  }
}

void TempoService::update() {

}
