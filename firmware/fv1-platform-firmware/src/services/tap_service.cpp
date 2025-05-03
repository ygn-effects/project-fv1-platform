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

  if (t_event.m_type == EventType::kTapPressed || t_event.m_type == EventType::kMidiTapPressed && t_event.m_data.value == MidiCCValues::c_tapShortPress) {
    m_tapHandler.registerTap(t_event.m_timestamp);

    if (m_tapHandler.getIsNewIntervalSet()) {
      m_logicalState.m_interval = m_tapHandler.getInterval();

      publishTapIntervalEvent(t_event);
    }

    m_logicalState.m_tapState = m_tapHandler.getTapState();
  }
  else if (t_event.m_type == EventType::kTapLongPressed && m_logicalState.m_tapState == TapState::kEnabled
        || t_event.m_type == EventType::kMidiTapPressed && t_event.m_data.value == MidiCCValues::c_tapLongPress && m_logicalState.m_tapState == TapState::kEnabled) {
    m_tapHandler.setNextDivValue();

    m_logicalState.m_divState = m_tapHandler.getDivState();
    m_logicalState.m_divValue = m_tapHandler.getDivValue();
    m_logicalState.m_divInterval = m_tapHandler.getDivInterval();

    publishTapIntervalEvent(t_event);
  }
  else if (t_event.m_type == EventType::kPot0Moved && m_logicalState.m_tapState == TapState::kEnabled
        || t_event.m_type == EventType::kMenuTempoChanged && m_logicalState.m_tapState == TapState::kEnabled) {
    m_logicalState.m_tapState = TapState::kDisabled;
    m_logicalState.m_divState = DivState::kDisabled;
    m_logicalState.m_divValue = DivValue::kQuarter;

    init();
  }
}

void TapService::update() {

}

bool TapService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kTapEvent
      || t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kPotEvent
      || t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuTempoEvent
      || t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramChangedEvent
      || t_category == EventCategory::kMidiEvent && t_subCategory == EventSubCategory::kMidiTapPressedEvent;
}
