#pragma once

#include "core/event.h"
#include "utils/circular_buffer.h"

namespace EventBusConstants {
  constexpr uint8_t c_maxEvents = 32;
}

class EventBus {
  private:
    using EventBuffer = CircularBuffer<Event, EventBusConstants::c_maxEvents>;
    inline static EventBuffer m_busBuffer;

  public:
    static bool hasEvent() {
      return ! m_busBuffer.isEmpty();
    }

    static bool publish(const Event& t_event) {
      return m_busBuffer.push(t_event);
    }

    static bool recall(Event& t_event) {
      return m_busBuffer.pop(t_event);
    }

    static void clear() {
      m_busBuffer.clear();
    }
};
