#include "drivers/encoder.h"

namespace hal {

uint8_t EncoderDriver::readState() {
  return (m_gpioA.read() ? 2 : 0) | (m_gpioB.read() ? 1 : 0);
}

EncoderDriver::EncoderDriver(DigitalGpioDriver t_gpioA, DigitalGpioDriver t_gpioB, EncoderId t_id)
  : m_gpioA(t_gpioA), m_gpioB(t_gpioB), m_prevState(0), m_accumulator(0), m_encoderId(t_id) {}

void EncoderDriver::init() {
  m_gpioA.init();
  m_gpioB.init();
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
