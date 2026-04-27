#include "drivers/expr_driver.h"

namespace hal {

void ExprDriver::init() {
  m_detectPin.init();
  m_pot.init();

  m_state = m_detectPin.read();
}

size_t ExprDriver::poll(Event* t_outEvents, size_t t_maxEvents) {
  size_t eventCount = 0;
  uint32_t now = millis();

  if (m_firstPoll && eventCount < t_maxEvents) {
    Event& e = t_outEvents[eventCount++];
    e.m_domain = EventDomain::kDriver;
    e.m_subject = EventSubject::kExpr;
    e.m_action = m_state
      ? EventAction::kConnected
      : EventAction::kDisconnected;
    e.m_timestamp = now;

    m_firstPoll = false;
  }

  bool switchEvent = m_detectPin.read();

  if (m_isDebouncing) {
    if (now - m_stateMs > m_debounceTime) {
      m_state = switchEvent;

      if (eventCount < t_maxEvents) {
        Event& e = t_outEvents[eventCount++];
        e.m_domain = EventDomain::kDriver;
        e.m_subject = EventSubject::kExpr;
        e.m_action = m_state
          ? EventAction::kConnected
          : EventAction::kDisconnected;
        e.m_timestamp = now;

        m_isDebouncing = false;
      }
    }
  }
  else {
    if (switchEvent != m_state) {
      m_isDebouncing = true;
      m_stateMs = now;
    }
  }

  if (m_state && eventCount < t_maxEvents) {
    Event tempEvents[4];
    size_t maxFetch = (t_maxEvents - eventCount > 4)
      ? 4
      : (t_maxEvents - eventCount);

    size_t potCount = m_pot.poll(tempEvents, maxFetch);

    for (size_t i = 0; i < potCount; i++) {
      Event& original = tempEvents[i];
      Event& translated = t_outEvents[eventCount++];

      translated = original;

      translated.m_subject = EventSubject::kExpr;
      translated.m_id = 0;
    }
  }

  return eventCount;
}

}
