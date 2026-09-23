import * as assert from "assert";
import * as vscode from "vscode";
import { SpinASMValidator } from "../spinasmValidator";

/** Runs the validator over content and returns the diagnostics it produced. */
async function diagnose(content: string): Promise<readonly vscode.Diagnostic[]> {
  const doc = await vscode.workspace.openTextDocument({ content, language: "spinasm" });
  const validator = new SpinASMValidator();
  validator.validateDocument(doc);
  const diags = vscode.languages.getDiagnostics(doc.uri);
  validator.dispose();
  return diags;
}

describe("SpinASMValidator", () => {
  it("does not warn on a valid LOG D operand outside ±2 (D is S4.6, range ±16)", async () => {
    const diags = await diagnose("log 0.5, 8\n");
    assert.ok(
      !diags.some(d => d.code === "coefficient-range"),
      "log D=8 is valid and must not trigger a coefficient-range warning"
    );
  });

  it("still warns on an out-of-range RDAX coefficient (S1.14 is -2..2)", async () => {
    const diags = await diagnose("rdax adcl, 3.0\n");
    assert.ok(
      diags.some(d => d.code === "coefficient-range"),
      "rdax C=3.0 should warn"
    );
  });

  it("flags a missing comma on a two-operand instruction", async () => {
    const diags = await diagnose("rdax adcl 1.0\n");
    assert.ok(diags.some(d => d.code === "missing-comma"));
  });

  it("flags a missing comma on delay-RAM instructions (RDA/WRA/WRAP)", async () => {
    const diags = await diagnose("mem del 1000\nrda del 0.5\nwra del 0.5\nwrap del 0.5\n");
    const missing = diags.filter(d => d.code === "missing-comma");
    assert.strictEqual(missing.length, 3, "rda/wra/wrap without comma should each warn");
  });

  it("flags an undefined symbol", async () => {
    const diags = await diagnose("rdax notdefined, 1.0\n");
    assert.ok(diags.some(d => d.code === "undefined-symbol"));
  });

  it("does not flag built-in registers as undefined", async () => {
    const diags = await diagnose("rdax adcl, 1.0\nwrax dacl, 0\n");
    assert.ok(!diags.some(d => d.code === "undefined-symbol"));
  });

  it("does not flag $ hex literals whose digits start with a letter", async () => {
    const diags = await diagnose("and $FFFF00\nor $ff_ff00\n");
    assert.ok(!diags.some(d => d.code === "undefined-symbol"));
  });

  it("does not flag asfv1's INT operator", async () => {
    const diags = await diagnose("sof int(0.5*2), 0\n");
    assert.ok(!diags.some(d => d.code === "undefined-symbol"));
  });

  it("places an undefined-symbol diagnostic on the operand, not the mnemonic", async () => {
    const diags = await diagnose("and d\n");
    const diag = diags.find(d => d.code === "undefined-symbol");
    assert.ok(diag);
    assert.strictEqual(diag.range.start.character, 4);
  });

  it("places a coefficient-range warning on the coefficient", async () => {
    const diags = await diagnose("rdax reg3, 3\n");
    const diag = diags.find(d => d.code === "coefficient-range");
    assert.ok(diag);
    assert.strictEqual(diag.range.start.character, 11);
  });

  it("flags two 16384-sample blocks as exceeding delay memory", async () => {
    const diags = await diagnose("mem a 16384\nmem b 16384\n");
    assert.ok(diags.some(d => d.code === "memory-limit"));
  });

  it("accepts two 16383-sample blocks, which fill delay memory exactly", async () => {
    const diags = await diagnose("mem a 16383\nmem b 16383\n");
    assert.ok(!diags.some(d => d.code === "memory-limit"));
  });

  it("flags an unknown mnemonic on the mistyped word", async () => {
    const diags = await diagnose("sfo 0, 0\nstart: rdx adcl, 1.0\n");
    const unknown = diags.filter(d => d.code === "unknown-instruction");
    assert.strictEqual(unknown.length, 2);
    assert.strictEqual(unknown[0].range.start.character, 0);
    assert.strictEqual(unknown[1].range.start.character, 7);
  });

  it("does not flag directives, labels or name-first MEM as unknown instructions", async () => {
    const diags = await diagnose("equ gain 0.5\ngain2 equ 0.2\nmem del 100\nDel2 MEM 100\nloop:\n  sof 0, 0\n");
    assert.ok(!diags.some(d => d.code === "unknown-instruction"));
  });

  it("does not flag an operand continued on the next line", async () => {
    const diags = await diagnose("equ vol reg0\nrdax\n  adcl, 1.0\nwrax\n  vol, 0\n");
    assert.ok(!diags.some(d => d.code === "unknown-instruction"));
  });
});
