#include "programmer.h"

void Programmer::setup() {
  Serial.begin(57600);
  pinMode(m_fv1Pin, OUTPUT);
  digitalWrite(m_fv1Pin, HIGH); // Initialize FV-1 pin to known state
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
  // If there is data to be read
  if (Serial.available() > 0) {
    uint32_t startTime = millis();

    while (millis() - startTime < timeout_ms) {
      // Read incoming data and manage potential overflow
      while (Serial.available()) {
        uint8_t byte = Serial.read();
        if (!m_buffer.push(byte)) {
          // Buffer full: discard oldest byte to make room
          m_buffer.pop();
          m_buffer.push(byte);
        }
      }

      if (! m_buffer.isEmpty()) {
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
            // Framing mismatch: discard one byte to realign buffer
            m_buffer.pop();
            return ProgrammerStatus::FramingError;
          }
        }
      }
      else {
        return ProgrammerStatus::NoMessage; // No incoming message
      }
    }

    return ProgrammerStatus::Timeout; // No valid message within timeout
  }

  return ProgrammerStatus::NoMessage; // No incoming message
}

void Programmer::sendMessage(const uint8_t* t_data, uint8_t t_count) {
  // Send data with framing markers for reliable parsing
  Serial.write(ProgrammerConstants::c_startMarker);
  Serial.write(t_data, t_count);
  Serial.write(ProgrammerConstants::c_endMarker);
}
