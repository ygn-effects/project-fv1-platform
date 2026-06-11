#pragma once

#include <Arduino.h>
#include "circular_buffer.h"

/**
 * @brief Constants for message framing in serial communication.
 */
namespace ProgrammerConstants {
  constexpr uint8_t c_startMarker = 30; ///< ASCII RS (Record Separator)
  constexpr uint8_t c_endMarker = 31;   ///< ASCII US (Unit Separator)
};

/**
 * @brief Result status of serial communication operations.
 */
enum class ProgrammerStatus {
  Success,         ///< Message received successfully
  NoMessage,       ///< Message received successfully
  Timeout,         ///< Timeout occurred waiting for message
  FramingError,    ///< Message framing error detected
  BufferOverflow   ///< Buffer overflow occurred during reception
};

/**
 * @brief Handles serial communication with framing for robust message exchange.
 */
class Programmer {
  private:
    uint8_t m_fv1ResetPin;            ///< GPIO pin controlling the FV-1 reset line
    CircularBuffer<64> m_buffer;      ///< Circular buffer for UART data handling

  public:
    /**
     * @brief Construct a Programmer object.
     *
     * @param t_pin GPIO pin used for FV-1 reset control.
     */
    Programmer(uint8_t t_pin) : m_fv1ResetPin(t_pin) {}

    /**
     * @brief Initializes serial communication and GPIO.
     */
    void setup();

    /**
     * @brief Resets the FV-1 by pulsing its reset line LOW then HIGH.
     *
     * Blocks for 50 ms while the line is held LOW.
     */
    void resetFv1();

    /**
     * @brief Receives available UART data into the buffer.
     */
    void receiveData();

    /**
     * @brief Retrieves a complete message from the serial buffer.
     *
     * @param t_data Buffer to store the received message.
     * @param t_count Expected message size (excluding framing markers).
     * @param timeout_ms Maximum wait time in milliseconds.
     * @return ProgrammerStatus indicating success or error type.
     */
    ProgrammerStatus getMessage(uint8_t* t_data, uint8_t t_count, uint16_t timeout_ms = 100);

    /**
     * @brief Sends a message over serial with proper framing.
     *
     * @param t_data Buffer containing data to send.
     * @param t_count Number of bytes to send.
     */
    void sendMessage(const uint8_t* t_data, uint8_t t_count);
};
