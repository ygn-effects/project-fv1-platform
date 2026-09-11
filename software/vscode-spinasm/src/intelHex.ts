import { BANK_COUNT, BANK_SIZE_BYTES, EEPROM_BLANK_BYTE } from "./fv1Constants";

export interface IntelHexData {
  address: number;
  data: Buffer;
}

/**
 * Parses Intel HEX text, validating per-record checksums, and returns the base
 * address plus a contiguous byte buffer (gaps and trailing space filled with
 * EEPROM_BLANK_BYTE, padded up to at least one bank).
 *
 * Pure and dependency-free (no serial transport, no vscode) so the format
 * handling can be unit-tested in isolation. File I/O and logging stay with the
 * caller.
 */
export function parseIntelHex(content: string): IntelHexData {
  const lines = content.split(/\r\n|\r|\n/);

  const memoryMap = new Map<number, number>();
  let minAddress = Infinity;
  let maxAddress = 0;
  let lineNo = 0;
  let foundEof = false;

  for (const line of lines) {
      lineNo++;

      if (line.trim().length === 0) {
        continue;
      }

      if (line[0] !== ':') {
        continue;
      }

      // Intel HEX record: `:LLAAAATT[DD...]CC` — min frame (no data) is 11 chars
      // (`:` + LL(2) + AAAA(4) + TT(2) + CC(2)). Data adds 2*LL between TT and CC.
      if (line.length < 11) {
        throw new Error(`Malformed HEX record at line ${lineNo}: too short (${line.length} chars)`);
      }

      const byteCountText = line.substring(1, 3);
      const addressText = line.substring(3, 7);
      const recordTypeText = line.substring(7, 9);
      if (!isHexField(byteCountText, 2) ||
          !isHexField(addressText, 4) ||
          !isHexField(recordTypeText, 2)) {
        throw new Error(`Malformed HEX record at line ${lineNo}: non-hex header`);
      }

      const byteCount = Number.parseInt(byteCountText, 16);
      const address = Number.parseInt(addressText, 16);
      const recordType = Number.parseInt(recordTypeText, 16);

      const expectedLength = 11 + 2 * byteCount;
      if (line.length !== expectedLength) {
        throw new Error(`Malformed HEX record at line ${lineNo}: expected ${expectedLength} chars for byteCount=${byteCount}, got ${line.length}`);
      }

      const checksumText = line.substring(expectedLength - 2, expectedLength);
      if (!isHexField(checksumText, 2)) {
        throw new Error(`Malformed HEX record at line ${lineNo}: non-hex checksum`);
      }
      const checksum = Number.parseInt(checksumText, 16);

      let calculatedChecksum = byteCount + (address >> 8) + (address & 0xFF) + recordType;
      const dataBytes: number[] = [];

      for (let i = 0; i < byteCount; i++) {
        const byteText = line.substring(9 + i * 2, 11 + i * 2);
        if (!isHexField(byteText, 2)) {
          throw new Error(`Malformed HEX record at line ${lineNo}: non-hex data byte at offset ${i}`);
        }
        const byte = Number.parseInt(byteText, 16);
        dataBytes.push(byte);
        calculatedChecksum += byte;
      }
      // Intel HEX checksum is two's complement of the LSB of the running sum.
      if (((calculatedChecksum + checksum) & 0xFF) !== 0) {
        throw new Error(`Checksum mismatch at line ${lineNo}`);
      }

      if (recordType === 0x00) { // data
        for (let i = 0; i < dataBytes.length; i++) {
          const byte = dataBytes[i];
          const absoluteAddress = address + i;
          memoryMap.set(absoluteAddress, byte);

          if (absoluteAddress < minAddress) {
            minAddress = absoluteAddress;
          }

          if (absoluteAddress > maxAddress) {
            maxAddress = absoluteAddress;
          }
        }
      }
      else if (recordType === 0x01) { // EOF
          if (byteCount !== 0 || address !== 0) {
            throw new Error(`Malformed HEX EOF record at line ${lineNo}`);
          }
          foundEof = true;
          break;
      }
      else {
        throw new Error(`Unsupported HEX record type 0x${recordType.toString(16).padStart(2, "0").toUpperCase()} at line ${lineNo}`);
      }
  }

  if (!foundEof) {
    throw new Error("HEX file is missing an EOF record.");
  }

  if (minAddress === Infinity) {
    throw new Error("HEX file contained no valid data records.");
  }

  // Keep the parser format-focused: short images are blank-filled to one bank,
  // while the upload validator below rejects images that extend past that bank.
  const size = maxAddress - minAddress + 1;
  const bufferSize = Math.max(BANK_SIZE_BYTES, size);
  const buffer = Buffer.alloc(bufferSize, EEPROM_BLANK_BYTE);

  memoryMap.forEach((byte, addr) => {
    buffer[addr - minAddress] = byte;
  });

  return {
    address: minAddress,
    data: buffer
  };
}

/** Ensures a parsed program can only target the bank selected by the user. */
export function validateIntelHexForBank(program: IntelHexData, bank: number): void {
  if (!Number.isInteger(bank) || bank < 0 || bank >= BANK_COUNT) {
    throw new Error(`Invalid bank ${bank}. Expected a bank from 0 to ${BANK_COUNT - 1}.`);
  }

  const expectedAddress = bank * BANK_SIZE_BYTES;
  if (program.address !== expectedAddress) {
    throw new Error(
      `HEX image starts at 0x${program.address.toString(16).toUpperCase()}, ` +
      `but bank ${bank} must start at 0x${expectedAddress.toString(16).toUpperCase()}.`
    );
  }

  if (program.data.length !== BANK_SIZE_BYTES) {
    throw new Error(
      `HEX image for bank ${bank} is ${program.data.length} bytes; ` +
      `expected exactly ${BANK_SIZE_BYTES} bytes.`
    );
  }
}

function isHexField(value: string, length: number): boolean {
  return value.length === length && /^[0-9A-Fa-f]+$/.test(value);
}
