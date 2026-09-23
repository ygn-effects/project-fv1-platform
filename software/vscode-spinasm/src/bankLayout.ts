import * as path from "path";
import { BANK_COUNT } from "./fv1Constants";

/**
 * On-disk layout of an FV-1 project, relative to its root folder:
 *
 *   bank_<N>/<any>.spn     program source for bank N
 *   output/bank_<N>.hex    compiled program for bank N
 *   output/output.bin      combined EEPROM image
 *
 * Single source of truth for these names, so scanning, output paths and the
 * file-watcher lookups can't drift apart. Pure so it can be unit-tested.
 */

export const OUTPUT_FOLDER_NAME = "output";
export const COMBINED_IMAGE_NAME = "output.bin";

export function bankFolderName(bank: number): string {
  return `bank_${bank}`;
}

export function bankOutputName(bank: number): string {
  return `bank_${bank}.hex`;
}

/**
 * Bank whose folder directly contains `filePath` (`<root>/bank_N/file`), or -1.
 * Only the path below `root` is considered, so a `bank_N` folder above the
 * project root doesn't count.
 */
export function bankOfSourceFile(root: string, filePath: string): number {
  const parts = relativeParts(root, filePath);
  if (parts.length !== 2) {
    return -1;
  }
  return bankFromName(parts[0], /^bank_(\d+)$/i);
}

/** Bank that `filePath` is the compiled output of (`<root>/output/bank_N.hex`), or -1. */
export function bankOfOutputFile(root: string, filePath: string): number {
  const parts = relativeParts(root, filePath);
  if (parts.length !== 2 || parts[0] !== OUTPUT_FOLDER_NAME) {
    return -1;
  }
  return bankFromName(parts[1], /^bank_(\d+)\.hex$/i);
}

function relativeParts(root: string, filePath: string): string[] {
  const relative = path.relative(root, filePath);
  if (!relative || relative.startsWith("..") || path.isAbsolute(relative)) {
    return [];
  }
  return relative.split(/[\\/]/);
}

function bankFromName(name: string, pattern: RegExp): number {
  const match = pattern.exec(name);
  if (!match) {
    return -1;
  }
  const bank = parseInt(match[1], 10);
  return bank < BANK_COUNT ? bank : -1;
}
