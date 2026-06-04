import * as assert from "assert";
import * as vscode from "vscode";
import { CompilerDiagnostics } from "../compilerDiagnostics";

describe("CompilerDiagnostics", () => {
  it("maps an asfv1 error to a diagnostic on the right line, then clears", async () => {
    const doc = await vscode.workspace.openTextDocument({
      content: "sof 0,0\nrdax adcl, 1.0\n",
      language: "spinasm",
    });
    const cd = new CompilerDiagnostics();

    cd.report(doc.uri, "parse error: bad thing on line 2");
    let diags = vscode.languages.getDiagnostics(doc.uri);
    const compiled = diags.filter(d => d.source === "asfv1");
    assert.strictEqual(compiled.length, 1);
    assert.strictEqual(compiled[0].range.start.line, 1); // line 2 -> 0-based index 1
    assert.strictEqual(compiled[0].severity, vscode.DiagnosticSeverity.Error);

    // Clean output replaces the set with nothing.
    cd.report(doc.uri, "");
    diags = vscode.languages.getDiagnostics(doc.uri);
    assert.ok(!diags.some(d => d.source === "asfv1"));

    cd.dispose();
  });

  it("maps a warning to a Warning-severity diagnostic", async () => {
    const doc = await vscode.workspace.openTextDocument({
      content: "wrax dacl, 0\n",
      language: "spinasm",
    });
    const cd = new CompilerDiagnostics();

    cd.report(doc.uri, "warning: S1_14 arg clamped to 0xffff for WRAX on line 1");
    const compiled = vscode.languages.getDiagnostics(doc.uri).filter(d => d.source === "asfv1");
    assert.strictEqual(compiled.length, 1);
    assert.strictEqual(compiled[0].severity, vscode.DiagnosticSeverity.Warning);

    cd.dispose();
  });
});
