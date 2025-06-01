#include "services/tempo_service.h"

TempoService::TempoService(LogicalState& t_lState, Adjustable& t_led, Clock& t_clock) :
    m_logicState(t_lState), m_tempoLed(t_led), m_clock(t_clock) {}

void TempoService::syncHandler() {
  m_handler.m_interval = m_logicState.m_tempo;
  m_handler.m_minInterval = m_logicState.m_activeProgram->m_minDelayMs;
  m_handler.m_maxInterval = m_logicState.m_activeProgram->m_maxDelayMs;
}

void TempoService::publishTempoEvent(uint16_t t_interval) const {
  Event e;
  e.m_type = EventType::kTempoChanged;
  e.m_timestamp = 0; /*millis*/
  e.m_data.value = t_interval;
  EventBus::publish(e);
}

void TempoService::publishSaveTempoEvent(uint16_t t_interval) const {
  Event e;
  e.m_type = EventType::kSaveTempo;
  e.m_timestamp = 0; /*millis*/
  e.m_data.value = t_interval;
  EventBus::publish(e);
}

void TempoService::init() {
  syncHandler();
  m_tempoLed.init();
}

void TempoService::handleEvent(const Event& t_event) {
  if (! m_logicState.m_activeProgram->m_isDelayEffect) { return; }

  switch (t_event.m_type) {
    case EventType::kTapIntervalChanged:
      m_handler.m_source = TempoSource::kTap;
      m_logicState.m_tempo = m_handler.mapInterval(t_event.m_data.value);

      publishTempoEvent(m_logicState.m_tempo);
      publishSaveTempoEvent(m_logicState.m_tempo);
      break;

    case EventType::kPot0Moved:
      m_handler.m_source = TempoSource::kPot;
      m_logicState.m_tempo = m_handler.mapInterval(t_event.m_data.value);

      publishTempoEvent(m_logicState.m_tempo);
      publishSaveTempoEvent(m_logicState.m_tempo);
      break;

    case EventType::kMenuTempoChanged:
      m_handler.m_source = TempoSource::kMenu;
      m_logicState.m_tempo = m_handler.mapInterval(t_event.m_data.delta);

      publishTempoEvent(m_logicState.m_tempo);
      publishSaveTempoEvent(m_logicState.m_tempo);
      break;

    case EventType::kProgramChanged:
      if (m_logicState.m_activeProgram->m_isDelayEffect) {
        syncHandler();
        m_handler.m_source = TempoSource::kTap;
        m_logicState.m_tempo = m_handler.mapInterval(m_handler.m_interval);

        publishTempoEvent(m_logicState.m_tempo);
        publishSaveTempoEvent(m_logicState.m_tempo);
      }
      else {
        m_tempoLed.off();
      }
      break;

    default:
      break;
  }
}

void TempoService::update() {
  if (m_logicState.m_activeProgram->m_isDelayEffect && m_logicState.m_tempo > 0) {
    m_tempoLed.setValue(m_handler.calculateTempoLedValue(m_clock.now()));
  }
}

bool TempoService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return (t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kPotEvent)
      || (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuTempoEvent)
      || (t_category == EventCategory::kTempoEvent && t_subCategory == EventSubCategory::kTapIntervalEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramChangedEvent);
}
