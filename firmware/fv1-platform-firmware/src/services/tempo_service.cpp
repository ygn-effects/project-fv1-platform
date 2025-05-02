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

    case EventType::kMenuTempoChanged:
      m_interval = Utils::clamp<uint16_t>(m_interval + t_event.m_data.delta, m_minInterval, m_maxInterval);
      m_logicState.m_tempo = m_interval;
      m_source = TempoSource::kMenu;

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

bool TempoService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kPotEvent
      || t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuTempoEvent
      || t_category == EventCategory::kTempoEvent && t_subCategory == EventSubCategory::kTapIntervalEvent
      || t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramChangedEvent;
}
