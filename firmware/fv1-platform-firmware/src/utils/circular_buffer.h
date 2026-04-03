// util/CircularBuffer.h
#pragma once

#include <stddef.h>

/**
* @brief Simple fixed-size circular buffer template.
*
* Provides FIFO behavior with optional overflow detection.
* @tparam T Type of elements to store
* @tparam Capacity Number of elements (must be power-of-two or arbitrary)
*/
template<typename T, size_t Capacity>
class CircularBuffer {
  private:
    T buffer_[Capacity];
    size_t head_{0};
    size_t tail_{0};
    bool full_{false};
    bool overflowed_{false};

    static constexpr size_t increment(size_t idx) {
      return (idx + 1) % Capacity;
    }

    void advanceHead() {
      if (full_) {
        // Overwriting oldest
        tail_ = increment(tail_);
        overflowed_ = true;
      }
      head_ = increment(head_);
      full_ = (head_ == tail_);
    }

  public:
    CircularBuffer() = default;

    /**
    * @return true if buffer is empty
    */
    bool isEmpty() const {
      return head_ == tail_ && !full_;
    }

    /**
    * @return true if buffer is full (next push will overflow)
    */
    bool isFull() const {
      return full_;
    }

    /**
    * Push an element into the buffer.
    * @param item The item to add
    * @return true if added, false if buffer was full (item dropped)
    */
    bool push(const T& item) {
      buffer_[head_] = item;
      advanceHead();
      return !overflowed_;
    }

    /**
    * Pop the oldest element.
    * @param out Reference to store popped item; left unchanged if empty.
    * @return true if an item was popped, false if buffer was empty.
    */
    bool pop(T& out) {
      if (isEmpty()) return false;
      out = buffer_[tail_];
      full_ = false;
      tail_ = increment(tail_);
      return true;
    }

    /**
    * Peek at the element at position index (0 = oldest).
    * Does not modify buffer state.
    * @param index 0-based index
    * @return pointer to element or nullptr if out of range
    */
    const T* peek(size_t index) const {
      if (isEmpty() || index >= size()) return nullptr;
      return &buffer_[(tail_ + index) % Capacity];
    }

    /**
    * Current number of stored elements
    */
    size_t size() const {
      if (full_) return Capacity;
      if (head_ >= tail_) return head_ - tail_;
      return Capacity + head_ - tail_;
    }

    /**
    * Reset buffer to empty state (drops data)
    */
    void clear() {
      head_ = tail_ = 0;
      full_ = false;
      overflowed_ = false;
    }

    /**
    * Did an overflow occur since last clear?
    */
    bool hasOverflowed() const {
      return overflowed_;
    }
};
