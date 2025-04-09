#pragma once

#include <Arduino.h>

/// EEPROM SPI Instructions
constexpr uint8_t EEPROM_READ = B00000011;
constexpr uint8_t EEPROM_WRITE = B00000010;
constexpr uint8_t EEPROM_WREN = B00000110;
constexpr uint8_t EEPROM_WRDI = B00000100;
constexpr uint8_t EEPROM_RDSR = B00000101;
constexpr uint8_t EEPROM_WRSR = B00000001;

/**
 * @brief Interface for a serial EEPROM (M95256), supports read/write of various data types.
 */
class Eeprom {
  private:
    uint8_t m_csPin;  // CS pin #

    /// @brief Setup the SPI bus and select the chip
    void select();

    /// @brief Deselect the chip and close the SPI bus
    void deselect();

    /// @brief Select the chip, send the WREN instruction, and deselect
    void enableWrite();

    /// @brief Selects the memory address to write or read
    /// @param t_address Address to select
    void sendAddress(uint16_t t_address);

    /// @brief Check the status of the WIP (Write in Progress) bit
    /// @return true if a read/write is still in progress, false otherwise
    bool isWip();

  public:
    /// @brief Construct a new Eeprom object
    /// @param t_cspin CS pin #
    Eeprom(uint8_t t_cspin) :
      m_csPin(t_cspin) { };

    /// @brief Return the status register
    /// @return uint8_t Status register value
    uint8_t readStatusRegister();

    /// @brief Reset the status register to its initial value `b00000010`
    void writeStatusRegister();

    /// @brief Setup the micro controller pins and start the SPI bus
    void setup();

    /// @brief Read an 8-bit value from the selected memory address
    /// @param t_address Memory address to read from
    /// @return uint8_t 8-bit value read from the EEPROM
    uint8_t readInt8(uint16_t t_address);

    /// @brief Read an 8-bit value from the selected memory address into a pointer
    /// @param t_address Memory address to read from
    /// @param t_data Pointer to store the 8-bit value
    void readInt8(uint16_t t_address, uint8_t* t_data);

    /// @brief Write an 8-bit value to the selected memory address
    /// @param t_address Memory address to write to
    /// @param t_data 8-bit value to write
    void writeInt8(uint16_t t_address, uint8_t t_data);

    /// @brief Read a 16-bit value from the selected memory address
    /// @param t_address Memory address to read from
    /// @return uint16_t 16-bit value read from the EEPROM
    uint16_t readInt16(uint16_t t_address);

    /// @brief Write a 16-bit value to the selected memory address
    /// @param t_address Memory address to write to
    /// @param t_data 16-bit value to write
    void writeInt16(uint16_t t_address, uint16_t t_data);

    /// @brief Read an array of 8-bit integers starting at the selected memory address
    /// @param t_address Memory address to read from
    /// @param t_data Pointer to store the array of data
    /// @param t_length Length of the data array
    void readArray(uint16_t t_address, uint8_t* t_data, uint8_t t_length);

    /// @brief Write an array of 8-bit integers starting at the selected memory address
    /// @param t_address Memory address to write to
    /// @param t_data Pointer to the array of data to write
    /// @param t_length Length of the data array
    void writeArray(uint16_t t_address, uint8_t* t_data, uint8_t t_length);
};

