import * as assert from "assert";
import { BANK_SIZE_BYTES } from "../fv1Constants";
import type { IntelHexData } from "../intelHex";
import { writeAndVerifyBankProgram } from "../programUpload";
import type { ProgramMemory } from "../programUpload";

class RecordingProgramMemory implements ProgramMemory {
  public writeCalls = 0;
  public readCalls = 0;

  public async writeProgram(_address: number, _program: Buffer): Promise<void> {
    this.writeCalls++;
  }

  public async readProgram(_address: number): Promise<Buffer> {
    this.readCalls++;
    return Buffer.alloc(BANK_SIZE_BYTES);
  }
}

describe("writeAndVerifyBankProgram", () => {
  const invalidPrograms: Array<{ name: string; bank: number; program: IntelHexData }> = [
    {
      name: "wrong-bank output",
      bank: 3,
      program: { address: 0, data: Buffer.alloc(BANK_SIZE_BYTES) }
    },
    {
      name: "oversized output",
      bank: 0,
      program: { address: 0, data: Buffer.alloc(BANK_SIZE_BYTES + 1) }
    }
  ];

  for (const { name, bank, program } of invalidPrograms) {
    it(`issues no EEPROM operations for ${name}`, async () => {
      const memory = new RecordingProgramMemory();

      await assert.rejects(writeAndVerifyBankProgram(memory, bank, program));

      assert.strictEqual(memory.writeCalls, 0);
      assert.strictEqual(memory.readCalls, 0);
    });
  }

  it("writes and reads back a valid bank image", async () => {
    const memory = new RecordingProgramMemory();
    const program = { address: 0, data: Buffer.alloc(BANK_SIZE_BYTES) };

    await writeAndVerifyBankProgram(memory, 0, program);

    assert.strictEqual(memory.writeCalls, 1);
    assert.strictEqual(memory.readCalls, 1);
  });

  it("retains read-back verification", async () => {
    const memory = new RecordingProgramMemory();
    const program = { address: 0, data: Buffer.alloc(BANK_SIZE_BYTES, 0x42) };

    await assert.rejects(
      writeAndVerifyBankProgram(memory, 0, program),
      /Data verification failed/
    );

    assert.strictEqual(memory.writeCalls, 1);
    assert.strictEqual(memory.readCalls, 1);
  });
});
