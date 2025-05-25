#include "drivers/encoder.h"

namespace hal {

uint8_t EncoderDriver::readState() const {
  return (digitalRead(m_pinA) ? 2 : 0) | (digitalRead(m_pinB) ? 1 : 0);
}

EncoderDriver::EncoderDriver(uint8_t t_pinA, uint8_t t_pinB, EncoderId t_id)
  : m_pinA(t_pinA), m_pinB(t_pinB), m_prevState(0), m_accumulator(0), m_encoderId(t_id) {}

void EncoderDriver::init() {
  pinMode(m_pinA, INPUT_PULLUP);
  pinMode(m_pinB, INPUT_PULLUP);
  m_prevState = readState();
}

size_t EncoderDriver::poll(Event* t_outEvents, size_t t_maxEvents) {
  size_t eventCount = 0;
  uint8_t state = readState();

  if (state != m_prevState) {
    uint8_t index = (m_prevState << 2) | state;
    int8_t delta = c_lookupTable[index];
    m_accumulator += delta;
    m_prevState = state;

    while (m_accumulator >= c_stepsPerDetent && eventCount < t_maxEvents) {
      m_accumulator -= c_stepsPerDetent;

      Event e;
      e.m_type = EventType::kEncoderMoved;
      e.m_timestamp = millis();
      e.m_data.delta = +1;
      e.m_data.id = static_cast<uint8_t>(m_encoderId);
      t_outEvents[eventCount++] = e;
    }

    while (m_accumulator <= -c_stepsPerDetent && eventCount < t_maxEvents) {
      m_accumulator += c_stepsPerDetent;

      Event e;
      e.m_type = EventType::kEncoderMoved;
      e.m_timestamp = millis();
      e.m_data.delta = -1;
      e.m_data.id = static_cast<uint8_t>(m_encoderId);
      t_outEvents[eventCount++] = e;
    }
  }

  return eventCount;
}

}
