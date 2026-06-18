#include "programmer.h"

void Programmer::setup() {
  Serial.begin(57600);
  pinMode(m_fv1ResetPin, OUTPUT);
  digitalWrite(m_fv1ResetPin, HIGH); // Initialize FV-1 pin to known state
}

void Programmer::resetFv1() {
  digitalWrite(m_fv1ResetPin, LOW);
  delay(50);
  digitalWrite(m_fv1ResetPin, HIGH);
}

void Programmer::receiveData() {
  // Read incoming data and manage potential overflow
  while (Serial.available()) {
    uint8_t byte = Serial.read();

    if (!m_buffer.push(byte)) {
      // Buffer full: discard oldest byte to make room
      m_buffer.pop();
      m_buffer.push(byte);
    }
  }
}

ProgrammerStatus Programmer::getMessage(uint8_t* t_data, uint8_t t_count, uint16_t timeout_ms) {
  receiveData();

  // No new or previously buffered data: no message pending
  if (m_buffer.isEmpty()) {
    return ProgrammerStatus::NoMessage;
  }

  uint32_t startTime = millis();

  while (millis() - startTime < timeout_ms) {
    receiveData();

    // Check if buffer contains a complete framed message
    if (m_buffer.size() >= t_count + 2) {
      uint8_t startMarker = m_buffer.peek(0);
      uint8_t endMarker = m_buffer.peek(t_count + 1);

      if (startMarker == ProgrammerConstants::c_startMarker &&
          endMarker == ProgrammerConstants::c_endMarker) {
        m_buffer.pop();  // Remove start marker

        // Extract the message content
        for (uint8_t i = 0; i < t_count; i++) {
          t_data[i] = m_buffer.pop();
        }

        m_buffer.pop();  // Remove end marker

        return ProgrammerStatus::Success;
      }
      else {
        // Framing mismatch: discard the offending byte, then resync to the next start marker
        m_buffer.pop();
        while (!m_buffer.isEmpty() && m_buffer.peek(0) != ProgrammerConstants::c_startMarker) {
          m_buffer.pop();
        }
        return ProgrammerStatus::FramingError;
      }
    }
  }

  return ProgrammerStatus::Timeout; // No complete message within timeout
}

void Programmer::sendMessage(const uint8_t* t_data, uint8_t t_count) {
  // Send data with framing markers for reliable parsing
  Serial.write(ProgrammerConstants::c_startMarker);
  Serial.write(t_data, t_count);
  Serial.write(ProgrammerConstants::c_endMarker);
}
