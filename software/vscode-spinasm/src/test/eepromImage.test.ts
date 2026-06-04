import * as assert from "assert";
import { mergeBankImages } from "../eepromImage";
import { BANK_COUNT, BANK_SIZE_BYTES, EEPROM_SIZE_BYTES } from "../fv1Constants";

/**
 * Builds an asfv1 `-p bank` style output: `(bank+1) * BANK_SIZE_BYTES` bytes,
 * with the bank-N region filled with `marker` and the leading padding (the part
 * the merge must discard) filled with a distinct `pad` byte.
 */
function bankOutput(bank: number, marker: number, pad = 0xaa): Buffer {
  const buf = Buffer.alloc((bank + 1) * BANK_SIZE_BYTES, pad);
  buf.fill(marker, bank * BANK_SIZE_BYTES, (bank + 1) * BANK_SIZE_BYTES);
  return buf;
}

describe("mergeBankImages", () => {
  it("produces a full-size, zero-filled image when no banks are present", () => {
    const img = mergeBankImages([]);
    assert.strictEqual(img.length, EEPROM_SIZE_BYTES);
    assert.ok(img.every(b => b === 0x00));
  });

  it("places bank 0 at offset 0 and leaves the rest zero", () => {
    const img = mergeBankImages([bankOutput(0, 0x11)]);
    assert.strictEqual(img.length, EEPROM_SIZE_BYTES);
    for (let i = 0; i < BANK_SIZE_BYTES; i++) {
      assert.strictEqual(img[i], 0x11);
    }
    assert.strictEqual(img[BANK_SIZE_BYTES], 0x00);
  });

  it("extracts the bank-N slice at the right offset, discarding leading padding", () => {
    const bank = 3;
    const img = mergeBankImages([null, null, null, bankOutput(bank, 0x33, 0xaa)]);

    const start = bank * BANK_SIZE_BYTES;
    const end = (bank + 1) * BANK_SIZE_BYTES;

    for (let i = start; i < end; i++) {
      assert.strictEqual(img[i], 0x33, `bank-${bank} region carries the program`);
    }
    assert.ok(!img.includes(0xaa), "asfv1's leading padding must not appear in the image");
    for (let i = 0; i < start; i++) {
      assert.strictEqual(img[i], 0x00, "earlier banks stay blank");
    }
  });

  it("places every bank at its own offset without overlap", () => {
    const outputs = Array.from({ length: BANK_COUNT }, (_, b) => bankOutput(b, b + 1));
    const img = mergeBankImages(outputs);

    for (let b = 0; b < BANK_COUNT; b++) {
      assert.strictEqual(img[b * BANK_SIZE_BYTES], b + 1, `bank ${b} marker at start of its slot`);
      assert.strictEqual(img[(b + 1) * BANK_SIZE_BYTES - 1], b + 1, `bank ${b} marker at end of its slot`);
    }
  });

  it("throws when a bank output is not the expected length", () => {
    const wrong = Buffer.alloc(2 * BANK_SIZE_BYTES); // bank 0 should be 1 * BANK_SIZE_BYTES
    assert.throws(() => mergeBankImages([wrong]), /Bank 0 compiled to 1024 bytes, expected 512/);
  });
});
