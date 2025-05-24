#pragma once

#include <vector>
#include "periphs/pollable.h"

class MockPollable : public Pollable {
  private:
    std::vector<Event> m_events;
    size_t m_callCount = 0;
    bool m_initialized = false;

  public:
    void init() override {
      m_initialized = true;
    }

    void setEvents(std::vector<Event> t_events) {
      m_events = t_events;
    }

    void addEvent(const Event& t_event) {
      m_events.push_back(t_event);
    }

    size_t poll(Event* t_outEvents, size_t t_maxEvents) override {
      if (! m_initialized) return 0;

      size_t emitted = 0;

      for (; emitted < t_maxEvents && m_callCount < m_events.size(); emitted++, m_callCount++) {
        t_outEvents[emitted] = m_events[m_callCount];
      }

      return emitted;
    }
};
