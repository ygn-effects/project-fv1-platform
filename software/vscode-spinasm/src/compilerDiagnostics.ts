import * as vscode from "vscode";
import { parseAsfv1Output, CompilerProblem } from "./asfv1Output";
import { pathsEqual } from "./pathUtils";

/**
 * Owns a diagnostic collection for problems reported by the asfv1 compiler,
 * kept separate from the live validator's collection so the two don't clobber
 * each other. `report` replaces a file's compiler problems from a fresh stderr
 * capture; empty/clean output clears them.
 */
export class CompilerDiagnostics {
  private readonly collection: vscode.DiagnosticCollection;

  constructor() {
    this.collection = vscode.languages.createDiagnosticCollection("spinasm-compiler");
  }

  /** Replaces `uri`'s compiler diagnostics from one asfv1 stderr capture. */
  public report(uri: vscode.Uri, stderr: string): void {
    const problems = parseAsfv1Output(stderr);
    if (problems.length === 0) {
      this.collection.delete(uri);
      return;
    }

    const doc = vscode.workspace.textDocuments.find(d => pathsEqual(d.uri.fsPath, uri.fsPath));
    this.collection.set(uri, problems.map(p => this.toDiagnostic(p, doc)));
  }

  /** Drops `uri`'s compiler diagnostics (e.g. after an edit makes them stale). */
  public clear(uri: vscode.Uri): void {
    this.collection.delete(uri);
  }

  public dispose(): void {
    this.collection.dispose();
  }

  private toDiagnostic(problem: CompilerProblem, doc: vscode.TextDocument | undefined): vscode.Diagnostic {
    const lineIndex = problem.line !== null ? Math.max(0, problem.line - 1) : 0;
    const range =
      doc && lineIndex < doc.lineCount
        ? doc.lineAt(lineIndex).range
        : new vscode.Range(lineIndex, 0, lineIndex, 256);

    const severity =
      problem.severity === "warning"
        ? vscode.DiagnosticSeverity.Warning
        : vscode.DiagnosticSeverity.Error;

    const diagnostic = new vscode.Diagnostic(range, problem.message, severity);
    diagnostic.source = "asfv1";
    return diagnostic;
  }
}
