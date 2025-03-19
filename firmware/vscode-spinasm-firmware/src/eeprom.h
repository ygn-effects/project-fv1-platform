#pragma once

#include <Arduino.h>
#include <Wire.h>

/**
 * @brief Possible outcomes of EEPROM operations.
 */
enum class EEPROMResult {
  Success,             ///< Operation succeeded
  Timeout,             ///< Operation timed out
  WriteError,          ///< Failed to write to EEPROM
  ReadError,           ///< Failed to read from EEPROM
  CommunicationError   ///< I²C communication error
};

/**
 * @brief Manages low-level interactions with an I²C EEPROM device.
 */
class EEPROM {
  private:
    uint8_t m_i2cAddress;

    /**
     * @brief Wait until the EEPROM is ready or timeout occurs.
     *
     * @param timeout Timeout duration in milliseconds (default 10ms).
     * @return true if EEPROM is ready, false if timeout.
     */
    bool waitForReady(uint16_t timeout = 10);

  public:
    /**
     * @brief Construct an EEPROM object.
     *
     * @param address I²C address of the EEPROM device.
     */
    EEPROM(uint8_t address) : m_i2cAddress(address) {}

    /**
     * @brief Initializes the I²C interface.
     */
    void setup();

    /**
     * @brief Checks if the EEPROM is ready for operations.
     *
     * @return true if ready, false otherwise.
     */
    bool isReady();

    /**
     * @brief Writes a single byte to the EEPROM.
     *
     * @param address EEPROM memory address.
     * @param data Byte to write.
     * @param maxRetries Maximum retry attempts on failure.
     * @return EEPROMResult indicating the outcome.
     */
    EEPROMResult writeByte(uint16_t address, uint8_t data, uint8_t maxRetries = 3);

    /**
     * @brief Reads a single byte from the EEPROM.
     *
     * @param address EEPROM memory address.
     * @param data Reference to store the read byte.
     * @param maxRetries Maximum retry attempts on failure.
     * @return EEPROMResult indicating the outcome.
     */
    EEPROMResult readByte(uint16_t address, uint8_t &data, uint8_t maxRetries = 3);

    /**
     * @brief Writes multiple bytes (page) to the EEPROM.
     *
     * @param address Starting EEPROM memory address.
     * @param data Pointer to data buffer to write.
     * @param length Number of bytes to write.
     * @param maxRetries Maximum retry attempts on failure.
     * @return EEPROMResult indicating the outcome.
     */
    EEPROMResult writePage(uint16_t address, const uint8_t *data, size_t length, uint8_t maxRetries = 3);

    /**
     * @brief Reads multiple bytes (page) from the EEPROM.
     *
     * @param address Starting EEPROM memory address.
     * @param data Pointer to buffer to store read data.
     * @param length Number of bytes to read.
     * @param maxRetries Maximum retry attempts on failure.
     * @return EEPROMResult indicating the outcome.
     */
    EEPROMResult readPage(uint16_t address, uint8_t *data, size_t length, uint8_t maxRetries = 3);
};
