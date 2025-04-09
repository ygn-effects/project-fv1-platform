#include "encoder.h"
#include "utils/logging.h"

void Encoder::setup() {
  pinMode(m_encoderPinA, INPUT);
  pinMode(m_encoderPinB, INPUT);

  // Enable pull-up resistors
  digitalWrite(m_encoderPinA, HIGH);
  digitalWrite(m_encoderPinB, HIGH);
}

uint8_t Encoder::readState() {
  m_lastEncoderState = (digitalRead(m_encoderPinB) << 1) | digitalRead(m_encoderPinA);
  m_encoderState = c_encoderStates[m_encoderState & 0xF][m_lastEncoderState];
  return m_encoderState & (INCREMENT | DECREMENT);  // Return only increment/decrement flags
}

bool Encoder::poll() {
  uint8_t state = readState();

  if (state == DECREMENT) {
    m_counter--;
    m_movedLeft = true;
    m_movedRight = false;

    if (m_counter == 255 || m_counter < m_minCounterValue) {
      m_counter = m_maxCounterValue;
    }

    LOG_DEBUG("Encoder pins %d/%d decremented : %d", m_encoderPinA, m_encoderPinB, m_counter);

    return true;

  } else if (state == INCREMENT) {
    m_counter++;
    m_movedLeft = false;
    m_movedRight = true;

    if (m_counter > m_maxCounterValue) {
      m_counter = m_minCounterValue;
    }

    LOG_DEBUG("Encoder pins %d/%d incremented : %d", m_encoderPinA, m_encoderPinB, m_counter);

    return true;
  }

  return false;
}

uint8_t Encoder::getCounter() const {
  return m_counter;
}

void Encoder::setCounter(uint8_t counter) {
  m_counter = counter;
}

uint8_t Encoder::getMinValue() const {
  return m_minCounterValue;
}

uint8_t Encoder::getMaxValue() const {
  return m_maxCounterValue;
}

void Encoder::setMinValue(uint8_t value) {
  m_minCounterValue = value;
}

void Encoder::setMaxValue(uint8_t value) {
  m_maxCounterValue = value;
}

uint8_t Encoder::getState() const {
  return m_encoderState;
}

bool Encoder::isMovedRight() const {
  return m_movedRight;
}

bool Encoder::isMovedLeft() const {
  return m_movedLeft;
}