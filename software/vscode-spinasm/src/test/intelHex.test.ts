import * as assert from "assert";
import { parseIntelHex, validateIntelHexForBank } from "../intelHex";
import { BANK_SIZE_BYTES, EEPROM_BLANK_BYTE } from "../fv1Constants";

/** Builds a valid Intel HEX record with a correct checksum. */
function record(address: number, type: number, data: number[]): string {
  const header = [data.length, (address >> 8) & 0xff, address & 0xff, type];
  const bytes = [...header, ...data];
  const sum = bytes.reduce((a, b) => a + b, 0);
  const checksum = (0x100 - (sum & 0xff)) & 0xff;
  const hex = (n: number) => n.toString(16).padStart(2, "0").toUpperCase();
  return ":" + [...bytes, checksum].map(hex).join("");
}

const EOF = ":00000001FF";

describe("parseIntelHex", () => {
  it("parses a single data record at address 0 and pads to a bank", () => {
    const hex = [record(0, 0, [0xde, 0xad, 0xbe, 0xef]), EOF].join("\n");
    const { address, data } = parseIntelHex(hex);

    assert.strictEqual(address, 0);
    assert.strictEqual(data.length, BANK_SIZE_BYTES);
    assert.deepStrictEqual([...data.subarray(0, 4)], [0xde, 0xad, 0xbe, 0xef]);
    assert.strictEqual(data[4], EEPROM_BLANK_BYTE, "trailing space is blank-filled");
  });

  it("uses the minimum data address as the base", () => {
    const { address, data } = parseIntelHex([record(0x10, 0, [1, 2]), EOF].join("\n"));
    assert.strictEqual(address, 0x10);
    assert.deepStrictEqual([...data.subarray(0, 2)], [1, 2]);
  });

  it("merges multiple data records (and accepts CRLF)", () => {
    const hex = [record(0, 0, [0xaa, 0xbb]), record(2, 0, [0xcc, 0xdd]), EOF].join("\r\n");
    const { data } = parseIntelHex(hex);
    assert.deepStrictEqual([...data.subarray(0, 4)], [0xaa, 0xbb, 0xcc, 0xdd]);
  });

  it("extracts every data byte at the right offset (substr→substring regression)", () => {
    const payload = Array.from({ length: 16 }, (_, i) => (i * 0x11) & 0xff);
    const { data } = parseIntelHex([record(0, 0, payload), EOF].join("\n"));
    assert.deepStrictEqual([...data.subarray(0, 16)], payload);
  });

  it("throws on a checksum mismatch", () => {
    const corrupted = record(0, 0, [0x01, 0x02]).slice(0, -2) + "00";
    assert.throws(() => parseIntelHex([corrupted, EOF].join("\n")), /Checksum mismatch/);
  });

  it("throws on a too-short record", () => {
    assert.throws(() => parseIntelHex(":0102\n"), /too short/);
  });

  it("throws on non-hex header bytes", () => {
    assert.throws(() => parseIntelHex(":ZZ0000004C\n"), /non-hex header/);
  });

  it("rejects a partially hexadecimal data byte", () => {
    const valid = record(0, 0, [0x10]);
    const malformed = valid.substring(0, 9) + "1G" + valid.substring(11);
    assert.throws(
      () => parseIntelHex([malformed, EOF].join("\n")),
      /non-hex data byte at offset 0/
    );
  });

  it("rejects trailing characters after the checksum", () => {
    const malformed = record(0, 0, [0x10]) + "00";
    assert.throws(
      () => parseIntelHex([malformed, EOF].join("\n")),
      /expected 13 chars.*got 15/
    );
  });

  it("rejects unsupported extended-address records", () => {
    const extendedLinearAddress = record(0, 0x04, [0x00, 0x00]);
    assert.throws(
      () => parseIntelHex([extendedLinearAddress, record(0, 0, [0x10]), EOF].join("\n")),
      /Unsupported HEX record type 0x04/
    );
  });

  it("requires a valid EOF record", () => {
    assert.throws(
      () => parseIntelHex(record(0, 0, [0x10])),
      /missing an EOF record/
    );
    assert.throws(
      () => parseIntelHex([record(0, 0, [0x10]), record(1, 0x01, [])].join("\n")),
      /Malformed HEX EOF record/
    );
  });

  it("throws when there are no data records", () => {
    assert.throws(() => parseIntelHex(EOF + "\n"), /no valid data records/);
  });

  it("ignores blank and non-colon lines", () => {
    const hex = ["", "; a note", record(0, 0, [0x42]), "", EOF].join("\n");
    assert.strictEqual(parseIntelHex(hex).data[0], 0x42);
  });

  it("stops at the EOF record", () => {
    const hex = [record(0, 0, [0x11]), EOF, record(0, 0, [0x22])].join("\n");
    assert.strictEqual(parseIntelHex(hex).data[0], 0x11);
  });
});

describe("validateIntelHexForBank", () => {
  it("accepts an image at the selected bank base address", () => {
    const bank = 3;
    const program = parseIntelHex([
      record(bank * BANK_SIZE_BYTES, 0, [0x42]),
      EOF
    ].join("\n"));

    assert.doesNotThrow(() => validateIntelHexForBank(program, bank));
  });

  it("rejects output for a different bank", () => {
    const program = parseIntelHex([record(0, 0, [0x42]), EOF].join("\n"));

    assert.throws(
      () => validateIntelHexForBank(program, 3),
      /bank 3 must start at 0x600/
    );
  });

  it("rejects an image that extends beyond one bank", () => {
    const program = parseIntelHex([
      record(0, 0, [0x42]),
      record(BANK_SIZE_BYTES, 0, [0x43]),
      EOF
    ].join("\n"));

    assert.strictEqual(program.data.length, BANK_SIZE_BYTES + 1);
    assert.throws(
      () => validateIntelHexForBank(program, 0),
      /513 bytes.*expected exactly 512 bytes/
    );
  });
});
