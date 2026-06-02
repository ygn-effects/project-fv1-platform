import * as vscode from "vscode";
import { DocumentParser, DocumentSymbol } from "./documentParser";
import { ResourceUsage } from "./resourceAnalyzer";
import { isInstruction, isBuiltInSymbol } from "./spinasmLanguage";

// Warn before hitting the hard limits. Instructions are scarce (128 max), so
// we warn earlier in absolute terms; memory is abundant (32K samples).
const INSTRUCTION_WARNING_RATIO = 120 / 128; // ≈ 93.75% — fires with ≤8 left.
const MEMORY_WARNING_RATIO = 0.9;            // fires at 90% capacity.

export class SpinASMValidator {

  private diagnosticCollection: vscode.DiagnosticCollection;

  constructor() {
    this.diagnosticCollection = vscode.languages.createDiagnosticCollection('spinasm');
  }

  public validateDocument(document: vscode.TextDocument): void {
    if (document.languageId !== 'spinasm') {
      return;
    }

    const diagnostics = this.validate(document);
    this.diagnosticCollection.set(document.uri, diagnostics);
  }

  public clearDocument(document: vscode.TextDocument): void {
    this.diagnosticCollection.delete(document.uri);
  }

  public dispose(): void {
    this.diagnosticCollection.dispose();
  }

  private validate(document: vscode.TextDocument): vscode.Diagnostic[] {
    const diagnostics: vscode.Diagnostic[] = [];

    const parsedDoc = DocumentParser.get(document);

    // Diagnostics produced by the parser pass come first.
    for (const parserDiag of parsedDoc.diagnostics) {
      const vsDiag = new vscode.Diagnostic(
        parserDiag.range,
        parserDiag.message,
        parserDiag.severity
      );
      vsDiag.code = parserDiag.code;
      diagnostics.push(vsDiag);
    }

    const symbols = parsedDoc.symbols;

    for (let i = 0; i < document.lineCount; i++) {
      const line = document.lineAt(i);
      const lineText = line.text;

      const codeOnly = lineText.split(';')[0].trim();
      if (!codeOnly) {
        continue;
      }

      // equ/mem and bare labels are validated in the parser pass. equ accepts
      // both `EQU NAME VALUE` and the SpinASM `NAME EQU VALUE` order.
      if (/^\s*(equ|mem)\s+/i.test(codeOnly)) {
        continue;
      }

      if (/^\s*[a-zA-Z_][a-zA-Z0-9_]*\s+equ\s+/i.test(codeOnly)) {
        continue;
      }

      if (/^\s*\w+:\s*$/.test(codeOnly)) {
        continue;
      }

      if (isInstruction(codeOnly)) {
        this.validateInstruction(line, symbols, diagnostics);
      }
    }

    this.validateResourceLimits(parsedDoc.resourceUsage, diagnostics);

    return diagnostics;
  }

  private validateInstruction(
    line: vscode.TextLine,
    symbols: Map<string, DocumentSymbol>,
    diagnostics: vscode.Diagnostic[]
  ): void {
    const lineText = line.text.split(';')[0];

    // Accepts an optional `label:` prefix so `start: SOF 0,0` parses too.
    const match = /^\s*(?:[a-zA-Z_][a-zA-Z0-9_]*:\s*)?(\w+)\s+(.*)$/.exec(lineText);
    if (!match) {
      return;
    }

    const instruction = match[1].toUpperCase();
    const operands = match[2];

    const symbolRefs = operands.matchAll(/\b([a-zA-Z_][a-zA-Z0-9_]*)\b/g);
    const operandStart = lineText.indexOf(operands);

    for (const symbolMatch of symbolRefs) {
      const symbolName = symbolMatch[1];
      const symbolUpper = symbolName.toUpperCase();

      if (isBuiltInSymbol(symbolUpper)) {
        continue;
      }

      if (/^\d/.test(symbolName)) {
        continue;
      }

      if (!symbols.has(symbolUpper)) {
        const startChar = operandStart + symbolMatch.index!;
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(line.lineNumber, startChar, line.lineNumber, startChar + symbolName.length),
          `Undefined symbol '${symbolName}'`,
          vscode.DiagnosticSeverity.Error
        );
        diagnostic.code = 'undefined-symbol';
        diagnostics.push(diagnostic);
      }
    }

    this.validateInstructionSyntax(line, instruction, operands, diagnostics);
  }

  private validateInstructionSyntax(
    line: vscode.TextLine,
    instruction: string,
    operands: string,
    diagnostics: vscode.Diagnostic[]
  ): void {
    const lineText = line.text.split(';')[0];

    const requiresComma = new Set([
      'RDAX', 'WRAX', 'RDFX', 'WRLX', 'WRHX', 'MAXX',
      'SOF', 'LOG', 'EXP'
    ]);

    if (requiresComma.has(instruction)) {
      if (!operands.includes(',')) {
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(line.lineNumber, 0, line.lineNumber, lineText.length),
          `Instruction '${instruction}' requires two operands separated by comma`,
          vscode.DiagnosticSeverity.Error
        );
        diagnostic.code = 'missing-comma';
        diagnostics.push(diagnostic);
      }
    }

    // Instructions whose SECOND (post-comma) operand is the S1.14/S1.9
    // coefficient with a ±2 range — the value this check captures. SOF/LOG/EXP
    // are deliberately excluded: their post-comma operand is D, not the
    // coefficient, and LOG's D ranges to ±16 (S4.6), so a ±2 check would flag
    // valid code (e.g. `log 0.5, 8`).
    const coefficientInstructions = new Set([
      'RDAX', 'WRAX', 'RDFX', 'WRLX', 'WRHX', 'MAXX',
      'RDA', 'WRA', 'WRAP'
    ]);

    if (coefficientInstructions.has(instruction)) {
      const coeffMatch = /,\s*([-+]?\d+\.?\d*)/.exec(operands);
      if (coeffMatch) {
        const coeff = parseFloat(coeffMatch[1]);

        // S1_14 fixed-point operand range is approximately -2.0 to 2.0.
        if (Math.abs(coeff) > 2.0) {
          const startChar = lineText.indexOf(coeffMatch[1]);
          const diagnostic = new vscode.Diagnostic(
            new vscode.Range(line.lineNumber, startChar, line.lineNumber, startChar + coeffMatch[1].length),
            `Coefficient ${coeff} likely out of range (typical range: -2.0 to 2.0)`,
            vscode.DiagnosticSeverity.Warning
          );
          diagnostic.code = 'coefficient-range';
          diagnostics.push(diagnostic);
        }
      }
    }
  }

  private validateResourceLimits(
    usage: ResourceUsage,
    diagnostics: vscode.Diagnostic[]
  ): void {
    if (usage.instructions.count > usage.instructions.limit) {
      const diagnostic = new vscode.Diagnostic(
        new vscode.Range(0, 0, 0, 0),
        `Instruction limit exceeded: ${usage.instructions.count} / ${usage.instructions.limit}`,
        vscode.DiagnosticSeverity.Error
      );
      diagnostic.code = 'instruction-limit';
      diagnostics.push(diagnostic);
    }
    else if (usage.instructions.count > usage.instructions.limit * INSTRUCTION_WARNING_RATIO) {
      const diagnostic = new vscode.Diagnostic(
        new vscode.Range(0, 0, 0, 0),
        `Approaching instruction limit: ${usage.instructions.count} / ${usage.instructions.limit}`,
        vscode.DiagnosticSeverity.Warning
      );
      diagnostic.code = 'instruction-warning';
      diagnostics.push(diagnostic);
    }

    if (usage.memory.used > usage.memory.total) {
      const diagnostic = new vscode.Diagnostic(
        new vscode.Range(0, 0, 0, 0),
        `Memory limit exceeded: ${usage.memory.used} / ${usage.memory.total} samples`,
        vscode.DiagnosticSeverity.Error
      );
      diagnostic.code = 'memory-limit';
      diagnostics.push(diagnostic);
    }
    else if (usage.memory.used > usage.memory.total * MEMORY_WARNING_RATIO) {
      const diagnostic = new vscode.Diagnostic(
        new vscode.Range(0, 0, 0, 0),
        `Approaching memory limit: ${usage.memory.used} / ${usage.memory.total} samples`,
        vscode.DiagnosticSeverity.Warning
      );
      diagnostic.code = 'memory-warning';
      diagnostics.push(diagnostic);
    }
  }
}
