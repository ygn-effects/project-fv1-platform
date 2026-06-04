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

  it("flags an undefined symbol", async () => {
    const diags = await diagnose("rdax notdefined, 1.0\n");
    assert.ok(diags.some(d => d.code === "undefined-symbol"));
  });

  it("does not flag built-in registers as undefined", async () => {
    const diags = await diagnose("rdax adcl, 1.0\nwrax dacl, 0\n");
    assert.ok(!diags.some(d => d.code === "undefined-symbol"));
  });
});
