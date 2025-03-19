#pragma once

#include <Arduino.h>
#include "programmer.h"
#include "eeprom.h"

/**
 * @brief Defines standard lengths for messages exchanged over serial communication.
 */
namespace MessageLength {
  constexpr uint8_t c_orderMessageLength = 1;   ///< Length of command/order messages
  constexpr uint8_t c_addressMessageLength = 2; ///< Length of address field messages
  constexpr uint8_t c_dataMessageLength = 32;   ///< Length of data payload in messages
}

/**
 * @brief Enumerates the possible states of the Hardware state machine.
 */
enum SystemState {
  kReceivingMessage,       ///< Awaiting message from programmer
  kProcessingMessage,      ///< Processing received message
  kProcessingWriteMessage, ///< Processing EEPROM write operation
  kProcessingReadMessage   ///< Processing EEPROM read operation
};

/**
 * @brief Enumerates messages exchanged between hardware and the VSCode module.
 */
enum class Message : uint8_t {
  kNone,               ///< Undefined or no message
  kRuThere,            ///< "Are you there?" message
  kRuReady,            ///< "Are you ready?" EEPROM readiness check
  kRead,               ///< EEPROM read command
  kWrite,              ///< EEPROM write command
  kEnd,                ///< End of transaction or session
  kNok,                ///< Negative acknowledgment (general failure)
  kOk,                 ///< Positive acknowledgment (success)
  kTimeout,            ///< Timeout occurred
  kWriteError,         ///< EEPROM write operation failed
  kReadError,          ///< EEPROM read operation failed
  kCommunicationError, ///< Generic communication error
  kFramingError        ///< Serial framing error detected
};

/**
 * @brief Maintains context information during EEPROM operations.
 */
struct OperationContext {
  uint16_t address = 0; ///< Current EEPROM address for operation
  uint8_t buffer[MessageLength::c_dataMessageLength] = {0}; ///< Data buffer for EEPROM operations
  size_t bufferLength = 0; ///< Actual length of data in buffer

  /**
   * @brief Resets the operation context to default state.
   */
  void reset() {
    address = 0;
    bufferLength = 0;
    memset(buffer, 0, sizeof(buffer));
  }
};

/**
 * @brief High-level controller class managing communication between VSCode and EEPROM hardware.
 *
 * Provides structured processing of incoming serial messages, state management,
 * and error handling in communication and EEPROM operations.
 */
class Hardware {
  private:
    SystemState m_systemState = SystemState::kReceivingMessage; ///< Current state machine status
    Message m_currentMessage = Message::kNone;                  ///< Last validated message received
    OperationContext m_context;                                 ///< Operational context data

    /**
     * @brief Retrieves and validates messages from the Programmer.
     *
     * @param t_data Buffer to store retrieved message.
     * @param t_count Expected message length.
     * @return ProgrammerStatus indicating success or type of error.
     */
    ProgrammerStatus getProgrammerMessage(uint8_t* t_data, uint8_t t_count);

    /**
     * @brief Performs an EEPROM page read operation and handles errors internally.
     *
     * @param t_address Starting address for read operation.
     * @param t_buffer Buffer to store read data.
     * @param t_length Number of bytes to read.
     * @return EEPROMResult indicating success or type of error.
     */
    EEPROMResult readEepromPage(uint16_t t_address, uint8_t* t_buffer, size_t t_length);

    /**
     * @brief Performs an EEPROM page write operation and handles errors internally.
     *
     * @param t_address Starting address for write operation.
     * @param t_buffer Data buffer to write to EEPROM.
     * @param t_length Number of bytes to write.
     * @return EEPROMResult indicating success or type of error.
     */
    EEPROMResult writeEepromPage(uint16_t t_address, uint8_t* t_buffer, size_t t_length);

    /**
     * @brief Validates received message against allowed message types.
     *
     * @param t_value Raw byte value of received message.
     * @return Validated Message enum value or Message::kNone if invalid.
     */
    Message validateMessage(uint8_t t_value);

    /**
     * @brief Sends a control message back to the VSCode module.
     *
     * @param t_order Message to send.
     */
    void sendOrder(Message t_order);

    /**
     * @brief Handles transition between system states.
     *
     * @param t_state New state to transition to.
     */
    void transitionToState(SystemState t_state);

    /**
     * @brief Processes incoming serial messages from the programmer.
     */
    void processReceivingMessage();

    /**
     * @brief Processes and delegates validated incoming messages.
     */
    void processProcessingMessage();

    /**
     * @brief Responds to "Are you there?" command.
     */
    void processRuThereMessage();

    /**
     * @brief Responds to "Are you ready?" EEPROM status inquiry.
     */
    void processRuReadyMessage();

    /**
     * @brief Handles sequential EEPROM read operations.
     */
    void processReadMessage();

    /**
     * @brief Handles sequential EEPROM write operations.
     */
    void processWriteMessage();

    /**
     * @brief Handles session end message.
     */
    void processEndMessage();

  public:
    /**
     * @brief Initializes the Hardware class and underlying components.
     */
    void setup();

    /**
     * @brief Polls and processes incoming serial data.
     */
    void poll();

    /**
     * @brief Processes the current operation based on internal state.
     */
    void process();
};
