/**
 * Hardware constants for the Spin Semiconductor FV-1 DSP and its companion
 * 24LC32A-compatible EEPROM. Single source of truth so display strings,
 * resource-limit checks, and EEPROM I/O can't drift apart.
 */

/** Number of program slots stored in an FV-1 EEPROM. */
export const BANK_COUNT = 8;

/** FV-1 opcodes are 32 bits wide. */
export const INSTRUCTION_SIZE_BYTES = 4;

/** Maximum instructions per program slot. */
export const INSTRUCTION_LIMIT = 128;

/** Size of one bank in bytes (INSTRUCTION_LIMIT × INSTRUCTION_SIZE_BYTES). */
export const BANK_SIZE_BYTES = INSTRUCTION_LIMIT * INSTRUCTION_SIZE_BYTES;

/** Full EEPROM image size, all banks concatenated. */
export const EEPROM_SIZE_BYTES = BANK_COUNT * BANK_SIZE_BYTES;

/** Number of user-addressable registers (REG0..REG31). */
export const REGISTER_COUNT = 32;

/** Sample-addressable delay memory length. */
export const MEMORY_SAMPLES = 32768;

/** Highest valid memory index (MEMORY_SAMPLES - 1). */
export const MEMORY_MAX_INDEX = MEMORY_SAMPLES - 1;

/** EEPROM write/read block size in bytes (one page). */
export const EEPROM_BLOCK_SIZE_BYTES = 32;

/** Default byte value of an erased EEPROM cell. */
export const EEPROM_BLANK_BYTE = 0xFF;
