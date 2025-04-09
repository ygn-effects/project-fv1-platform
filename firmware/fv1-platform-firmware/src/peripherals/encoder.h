#pragma once

#include <Arduino.h>

/// @brief Drive an encoder, it has 2 pins and a counter with
/// a minimum and maximum value. Its state can be polled and
/// the current value of the counter retrieved
class Encoder {
  private:
    uint8_t m_encoderPinA;      // Encoder CLK pin #
    uint8_t m_encoderPinB;      // Encoder DT pin #
    uint8_t m_minCounterValue;
    uint8_t m_maxCounterValue;
    uint8_t m_counter = 0;
    uint8_t m_encoderState = 0;
    uint8_t m_lastEncoderState = 0;

    bool m_movedRight = false;
    bool m_movedLeft = false;

    /// @brief Encoder state transition table
    const uint8_t c_encoderStates[7][4] = {
      {0x0, 0x2, 0x4, 0x0},
      {0x3, 0x0, 0x1, 0x0 | Encoder::DECREMENT},
      {0x3, 0x2, 0x0, 0x0},
      {0x3, 0x2, 0x1, 0x0},
      {0x6, 0x0, 0x4, 0x0},
      {0x6, 0x5, 0x0, 0x0 | Encoder::INCREMENT},
      {0x6, 0x5, 0x4, 0x0},
    };

    /// @brief Reads the current state of the encoder
    /// @return uint8_t Encoder state (increment/decrement)
    uint8_t readState();

  public:
    /// @brief Constant for increment state flag
    static constexpr uint8_t INCREMENT = 0x20;

    /// @brief Constant for decrement state flag
    static constexpr uint8_t DECREMENT = 0x10;

    /// @brief Construct a new Encoder object
    /// @param t_pinA Encoder CLK pin #
    /// @param t_pinB Encoder DT pin #
    /// @param t_minValue
    /// @param t_maxValue
    Encoder(
      uint8_t t_pinA,
      uint8_t t_pinB,
      uint8_t t_minValue = 0,
      uint8_t t_maxValue = 1
    ) :
      m_encoderPinA(t_pinA),
      m_encoderPinB(t_pinB),
      m_minCounterValue(t_minValue),
      m_maxCounterValue(t_maxValue) { };

    /// @brief Setup the micro controller pins
    void setup();

    /// @brief Monitor the encoder for turns and manage the counter
    /// @return true if counter updated, false otherwise
    bool poll();

    /// @brief Get the current counter value
    /// @return uint8_t Counter value
    uint8_t getCounter() const;

    /// @brief Set the counter value
    /// @param t_counter New counter value
    void setCounter(uint8_t t_counter);

    /// @brief Get the minimum counter value
    /// @return uint8_t Minimum counter value
    uint8_t getMinValue() const;

    /// @brief Get the maximum counter value
    /// @return uint8_t Maximum counter value
    uint8_t getMaxValue() const;

    /// @brief Set the minimum counter value
    /// @param t_value New minimum counter value
    void setMinValue(uint8_t t_value);

    /// @brief Set the maximum counter value
    /// @param t_value New maximum counter value
    void setMaxValue(uint8_t t_value);

    /// @brief Get the current encoder state
    /// @return uint8_t Encoder state (INCREMENT/DECREMENT)
    uint8_t getState() const;

    /// @brief Check if the encoder was last moved to the right
    /// @return bool true if last moved to the right
    bool isMovedRight() const;

    /// @brief Check if the encoder was last moved to the left
    /// @return bool true if last moved to the left
    bool isMovedLeft() const;
};
