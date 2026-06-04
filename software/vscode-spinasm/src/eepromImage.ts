import { BANK_COUNT, BANK_SIZE_BYTES, EEPROM_SIZE_BYTES } from "./fv1Constants";

/**
 * Merges per-bank `asfv1 -p N` outputs into a single EEPROM image.
 *
 * asfv1 emits `(N+1) * BANK_SIZE_BYTES` for `-p N`: banks 0..(N-1) zero-padded,
 * the assembled program at offset `N * BANK_SIZE_BYTES`. We take just the bank-N
 * slice from each present bank; absent banks (null/undefined) stay `0x00`, which
 * matches asfv1's own padding convention.
 *
 * Pure and dependency-free so the offset math can be unit-tested without a
 * compiler or filesystem. Throws if a bank's output isn't the expected length.
 *
 * @param bankOutputs indexed by bank; null/undefined for banks with no program.
 */
export function mergeBankImages(bankOutputs: ReadonlyArray<Buffer | null | undefined>): Buffer {
  const eeprom = Buffer.alloc(EEPROM_SIZE_BYTES, 0x00);

  for (let bank = 0; bank < BANK_COUNT; bank++) {
    const bytes = bankOutputs[bank];
    if (!bytes) {
      continue;
    }

    const expected = (bank + 1) * BANK_SIZE_BYTES;
    if (bytes.length !== expected) {
      throw new Error(`Bank ${bank} compiled to ${bytes.length} bytes, expected ${expected}`);
    }

    bytes.copy(eeprom, bank * BANK_SIZE_BYTES, bank * BANK_SIZE_BYTES, expected);
  }

  return eeprom;
}
