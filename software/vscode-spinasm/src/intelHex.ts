import { BANK_SIZE_BYTES, EEPROM_BLANK_BYTE } from "./fv1Constants";

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
 * caller in Programmer.
 */
export function parseIntelHex(content: string): IntelHexData {
  const lines = content.split(/\r\n|\r|\n/);

  const memoryMap = new Map<number, number>();
  let minAddress = Infinity;
  let maxAddress = 0;
  let lineNo = 0;

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

      const byteCount = parseInt(line.substring(1, 3), 16);
      const address = parseInt(line.substring(3, 7), 16);
      const recordType = parseInt(line.substring(7, 9), 16);

      if (isNaN(byteCount) || isNaN(address) || isNaN(recordType)) {
        throw new Error(`Malformed HEX record at line ${lineNo}: non-hex header`);
      }

      const expectedLength = 11 + 2 * byteCount;
      if (line.length < expectedLength) {
        throw new Error(`Malformed HEX record at line ${lineNo}: expected ${expectedLength} chars for byteCount=${byteCount}, got ${line.length}`);
      }

      const checksum = parseInt(line.slice(-2), 16);
      if (isNaN(checksum)) {
        throw new Error(`Malformed HEX record at line ${lineNo}: non-hex checksum`);
      }

      let calculatedChecksum = byteCount + (address >> 8) + (address & 0xFF) + recordType;

      for (let i = 0; i < byteCount; i++) {
        const byte = parseInt(line.substring(9 + i * 2, 11 + i * 2), 16);
        if (isNaN(byte)) {
          throw new Error(`Malformed HEX record at line ${lineNo}: non-hex data byte at offset ${i}`);
        }
        calculatedChecksum += byte;
      }
      // Intel HEX checksum is two's complement of the LSB of the running sum.
      if (((calculatedChecksum + checksum) & 0xFF) !== 0) {
        throw new Error(`Checksum mismatch at line ${lineNo}`);
      }

      if (recordType === 0x00) { // data
        for (let i = 0; i < byteCount; i++) {
          const byte = parseInt(line.substring(9 + i * 2, 11 + i * 2), 16);
          const absoluteAddress = address + i;
          memoryMap.set(absoluteAddress, byte);

          if (absoluteAddress < minAddress) {
            minAddress = absoluteAddress;
          }

          if (absoluteAddress > maxAddress) {
            maxAddress = absoluteAddress;
          }
        }
      } else if (recordType === 0x01) { // EOF
          break;
      }
  }

  if (minAddress === Infinity) {
    throw new Error("HEX file contained no valid data records.");
  }

  // Bank size is fixed but we tolerate larger inputs in case of off-spec hex.
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
