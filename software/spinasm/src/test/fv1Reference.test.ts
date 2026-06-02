import * as assert from "assert";
import { INSTRUCTIONS, BUILT_IN_SYMBOLS } from "../spinasmLanguage";
import { getInstructionDoc, getSymbolDoc } from "../fv1Reference";

describe("fv1Reference", () => {
  it("documents every instruction in the INSTRUCTIONS set", () => {
    for (const name of INSTRUCTIONS) {
      assert.ok(getInstructionDoc(name), `missing instruction doc for ${name}`);
    }
  });

  it("documents every built-in symbol (directly or as an instruction)", () => {
    for (const name of BUILT_IN_SYMBOLS) {
      const doc = getInstructionDoc(name) ?? getSymbolDoc(name);
      assert.ok(doc, `missing doc for built-in symbol ${name}`);
    }
  });

  it("looks up instructions case-insensitively", () => {
    assert.deepStrictEqual(getInstructionDoc("rdax"), getInstructionDoc("RDAX"));
    assert.ok(getInstructionDoc("Sof"));
  });

  it("resolves numbered registers dynamically", () => {
    assert.ok(getSymbolDoc("REG0"));
    assert.ok(getSymbolDoc("reg31"));
    assert.match(getSymbolDoc("REG7")!.summary, /register 7/i);
  });

  it("returns undefined for unknown words", () => {
    assert.strictEqual(getInstructionDoc("TEMPO"), undefined);
    assert.strictEqual(getSymbolDoc("TEMPO"), undefined);
  });

  it("provides a signature and summary for each entry", () => {
    const rdax = getInstructionDoc("RDAX")!;
    assert.ok(rdax.signature.length > 0);
    assert.ok(rdax.summary.length > 0);
  });
});
