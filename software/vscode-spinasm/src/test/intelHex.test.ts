import * as assert from "assert";
import { parseIntelHex } from "../intelHex";
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
