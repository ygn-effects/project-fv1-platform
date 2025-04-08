#pragma once

#include <Arduino.h>

/**
 * @brief Simple circular buffer implementation for fixed-size byte storage.
 *
 * Provides FIFO (First-In-First-Out) functionality with efficient
 * wrap-around behavior and overflow detection.
 *
 * @tparam c_bufferSize Size of the buffer; defaults to 64 bytes.
 */
template<size_t c_bufferSize = 64>
class CircularBuffer {
private:
  uint8_t buffer[c_bufferSize];  ///< Internal storage buffer
  uint8_t head = 0;              ///< Index to next write position
  uint8_t tail = 0;              ///< Index to next read position
  bool overflow = false;         ///< Overflow flag indicating data loss

public:
  /**
   * @brief Construct a new Circular Buffer object.
   */
  CircularBuffer() {}

  /**
   * @brief Checks if the buffer is empty.
   *
   * @return true Buffer is empty.
   * @return false Buffer has data.
   */
  bool isEmpty() const { return head == tail; }

  /**
   * @brief Checks if the buffer is full.
   *
   * @return true Buffer is full.
   * @return false Buffer has room for data.
   */
  bool isFull() const { return (head + 1) % c_bufferSize == tail; }

  /**
   * @brief Adds a byte to the buffer.
   *
   * @param data Byte to add.
   * @return true Data added successfully.
   * @return false Buffer was full; data not added, overflow set.
   */
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

  /**
   * @brief Removes and returns the oldest byte from the buffer.
   *
   * @param defaultValue Default value to return if buffer is empty.
   * @return uint8_t Retrieved byte or defaultValue if empty.
   */
  uint8_t pop(uint8_t defaultValue = 0) {
    if (!isEmpty()) {
      uint8_t data = buffer[tail];
      tail = (tail + 1) % c_bufferSize;
      return data;
    }
    return defaultValue;
  }

  /**
   * @brief Reads a byte at the specified index without removing it.
   *
   * @param index Index relative to current tail position.
   * @return uint8_t The byte at specified position.
   */
  uint8_t peek(uint8_t index) const {
    return buffer[(tail + index) % c_bufferSize];
  }

  /**
   * @brief Returns the current number of bytes stored in the buffer.
   *
   * @return uint8_t Number of stored bytes.
   */
  uint8_t size() const {
    return (c_bufferSize + head - tail) % c_bufferSize;
  }

  /**
   * @brief Clears all data from the buffer.
   */
  void clear() {
    head = tail = 0;
    overflow = false;
  }

  /**
   * @brief Checks if buffer overflow has occurred.
   *
   * @return true Buffer overflow occurred.
   * @return false No overflow detected.
   */
  bool hasOverflowed() const {
    return overflow;
  }

  /**
   * @brief Resets the overflow flag without clearing buffer content.
   */
  void clearOverflowFlag() {
    overflow = false;
  }
};
