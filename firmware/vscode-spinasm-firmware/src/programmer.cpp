#include "programmer.h"

void Programmer::setup() {
  Serial.begin(57600);
  pinMode(m_fv1Pin, OUTPUT);
  digitalWrite(m_fv1Pin, HIGH);
}

void Programmer::receiveData() {
  while (Serial.available()) {
    if (!m_buffer.push(Serial.read())) {
      // Buffer overflow, discard oldest data
      m_buffer.pop();
      m_buffer.push(Serial.read());
    }
  }
}

ProgrammerStatus Programmer::getMessage(uint8_t* t_data, uint8_t t_count, uint16_t timeout_ms) {
  uint32_t startTime = millis();

  while (millis() - startTime < timeout_ms) {
    receiveData();

    if (m_buffer.size() >= t_count + 2) {
      uint8_t startMarker = m_buffer.peek(0);
      uint8_t endMarker = m_buffer.peek(t_count + 1);

      if (startMarker == ProgrammerConstants::c_startMarker &&
          endMarker == ProgrammerConstants::c_endMarker) {
        m_buffer.pop();  // Remove start marker

        for (uint8_t i = 0; i < t_count; i++) {
          t_data[i] = m_buffer.pop();
        }

        m_buffer.pop();  // Remove end marker

        return ProgrammerStatus::Success;
      }
      else {
        // Framing error, discard until possible next start marker
        m_buffer.pop();
        return ProgrammerStatus::FramingError;
      }
    }
  }

  return ProgrammerStatus::Timeout;
}

void Programmer::sendMessage(const uint8_t* t_data, uint8_t t_count) {
  Serial.write(ProgrammerConstants::c_startMarker);
  Serial.write(t_data, t_count);
  Serial.write(ProgrammerConstants::c_endMarker);
}
