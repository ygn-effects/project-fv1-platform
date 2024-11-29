#pragma once

#include <Arduino.h>

template<size_t c_bufferSize = 64>
class CircularBuffer {
  private:
    uint8_t buffer[c_bufferSize];
    uint8_t head = 0;
    uint8_t tail = 0;
    bool overflow = false;

  public:
    CircularBuffer() {}

    bool isEmpty() const { return head == tail; }
    bool isFull() const { return (head + 1) % c_bufferSize == tail; }

    bool push(uint8_t data) {
      if (!isFull()) {
        buffer[head] = data;
        head = (head + 1) % c_bufferSize;
        return true;
      } else {
        overflow = true;
        return false;
      }
    }

    uint8_t pop(uint8_t defaultValue = 0) {
      if (!isEmpty()) {
        uint8_t data = buffer[tail];
        tail = (tail + 1) % c_bufferSize;
        return data;
      }
      return defaultValue;
    }

    uint8_t peek(uint8_t index) const {
      return buffer[(tail + index) % c_bufferSize];
    }

    uint8_t size() const {
      return (c_bufferSize + head - tail) % c_bufferSize;
    }

    void clear() {
      head = tail = 0;
      overflow = false;
    }

    bool hasOverflowed() const {
      return overflow;
    }

    void clearOverflowFlag() {
      overflow = false;
    }
};
