import { validateIntelHexForBank } from "./intelHex";
import type { IntelHexData } from "./intelHex";

/** Narrow EEPROM surface used by the upload path and its no-write tests. */
export interface ProgramMemory {
  writeProgram(address: number, program: Buffer): Promise<void>;
  readProgram(address: number): Promise<Buffer>;
}

/** Writes one validated bank and verifies it by reading it back. */
export async function writeAndVerifyBankProgram(
  programmer: ProgramMemory,
  bank: number,
  program: IntelHexData
): Promise<void> {
  // Keep this check directly adjacent to the destructive operation even though
  // performUpload also validates before opening the serial port.
  validateIntelHexForBank(program, bank);

  await programmer.writeProgram(program.address, program.data);
  const programRead = await programmer.readProgram(program.address);

  if (Buffer.compare(program.data, programRead) !== 0) {
    throw new Error("Data verification failed.");
  }
}
