import * as assert from "assert";
import * as path from "path";
import { bankOfOutputFile, bankOfSourceFile } from "../bankLayout";

const root = path.resolve("/projects/pedal");
const at = (...parts: string[]) => path.join(root, ...parts);

describe("bankOfSourceFile", () => {
  it("returns the bank of a file directly inside a bank folder", () => {
    assert.strictEqual(bankOfSourceFile(root, at("bank_3", "chorus.spn")), 3);
  });

  it("ignores a bank_N folder above the project root", () => {
    const nestedRoot = path.resolve("/projects/bank_2/pedal");
    assert.strictEqual(bankOfSourceFile(nestedRoot, path.join(nestedRoot, "bank_0", "chorus.spn")), 0);
    assert.strictEqual(bankOfSourceFile(nestedRoot, path.join(nestedRoot, "notes.spn")), -1);
  });

  it("ignores files in subfolders of a bank and files outside the root", () => {
    assert.strictEqual(bankOfSourceFile(root, at("bank_1", "old", "chorus.spn")), -1);
    assert.strictEqual(bankOfSourceFile(root, path.resolve("/elsewhere/bank_1/chorus.spn")), -1);
  });

  it("rejects bank numbers past the last bank", () => {
    assert.strictEqual(bankOfSourceFile(root, at("bank_7", "a.spn")), 7);
    assert.strictEqual(bankOfSourceFile(root, at("bank_8", "a.spn")), -1);
  });
});

describe("bankOfOutputFile", () => {
  it("returns the bank of a compiled output", () => {
    assert.strictEqual(bankOfOutputFile(root, at("output", "bank_5.hex")), 5);
  });

  it("ignores other files and output folders elsewhere", () => {
    assert.strictEqual(bankOfOutputFile(root, at("output", "output.bin")), -1);
    assert.strictEqual(bankOfOutputFile(root, at("bank_0", "output", "bank_0.hex")), -1);
    assert.strictEqual(bankOfOutputFile(root, at("output", "bank_9.hex")), -1);
  });
});
