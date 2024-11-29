#include "programmer.h"

void Programmer::setup() {
  Serial.begin(57600);
  pinMode(m_fv1Pin, OUTPUT);
  digitalWrite(m_fv1Pin, HIGH);
}

void Programmer::receiveData() {
  while (Serial.available()) {
    m_buffer.push(Serial.read());
  }
}

bool Programmer::getMessage(uint8_t* t_data, uint8_t t_count) {
  if (m_buffer.size() >= t_count + 2) {
    uint8_t start = m_buffer.peek(0);
    uint8_t end = m_buffer.peek(t_count + 1);

    if (start == ProgrammerConstants::c_startMarker && end == ProgrammerConstants::c_endMarker) {
      m_buffer.pop();

      for (uint8_t i = 0; i < t_count; i++) {
        t_data[i] = m_buffer.pop();
      }

      m_buffer.pop();
      return true;
    }
    else {
      for (uint8_t i = 0; i < t_count + 2; i++) {
        m_buffer.pop();
      }
    }
  }

  return false;
}

void Programmer::sendMessage(uint8_t* t_data, uint8_t t_count) {
  for (uint8_t i = 0; i < t_count; i++) {
    Serial.write(t_data[i]);
  }
}
