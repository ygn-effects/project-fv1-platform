import * as vscode from "vscode";
import {
  DocumentParser,
  DocumentSymbol,
  groupColumn,
  InstructionLine,
  UnrecognizedLine,
} from "./documentParser";
import { ResourceUsage } from "./resourceAnalyzer";
import { isBuiltInSymbol } from "./spinasmLanguage";

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

    for (const instruction of parsedDoc.instructions) {
      this.validateInstruction(instruction, symbols, diagnostics);
    }

    for (const line of parsedDoc.unrecognized) {
      this.validateUnknownMnemonic(line, symbols, diagnostics);
    }

    this.validateResourceLimits(parsedDoc.resourceUsage, diagnostics);

    return diagnostics;
  }

  /**
   * Flags a code line that is neither a directive, a label nor a known
   * instruction, e.g. the typo `sfo 0,0`. asfv1 rejects these at compile time.
   */
  private validateUnknownMnemonic(
    line: UnrecognizedLine,
    symbols: Map<string, DocumentSymbol>,
    diagnostics: vscode.Diagnostic[]
  ): void {
    // asfv1 reads a token stream, so operands may continue on the next line
    // (`rdax` / `adcl, 1.0`). A line starting with a known symbol is such a
    // continuation, not a mistyped mnemonic.
    const token = line.word.toUpperCase();
    if (isBuiltInSymbol(token) || symbols.has(token)) {
      return;
    }

    const diagnostic = new vscode.Diagnostic(
      new vscode.Range(line.line, line.column, line.line, line.column + line.word.length),
      `Unknown instruction '${line.word}'`,
      vscode.DiagnosticSeverity.Error
    );
    diagnostic.code = 'unknown-instruction';
    diagnostics.push(diagnostic);
  }

  private validateInstruction(
    instruction: InstructionLine,
    symbols: Map<string, DocumentSymbol>,
    diagnostics: vscode.Diagnostic[]
  ): void {
    const { line, operands, operandsColumn } = instruction;
    if (operands === undefined) {
      return;
    }

    const symbolRefs = operands.matchAll(/\b([a-zA-Z_][a-zA-Z0-9_]*)\b/g);

    for (const symbolMatch of symbolRefs) {
      const symbolName = symbolMatch[1];
      const symbolUpper = symbolName.toUpperCase();

      if (isBuiltInSymbol(symbolUpper)) {
        continue;
      }

      // INT is asfv1's only named expression operator, e.g. `sof int(0.5*2),0`.
      if (symbolUpper === 'INT') {
        continue;
      }

      // The digits of a `$` hex literal can start with a letter (`$FFFF00`).
      if (operands[symbolMatch.index! - 1] === '$') {
        continue;
      }

      if (/^\d/.test(symbolName)) {
        continue;
      }

      if (!symbols.has(symbolUpper)) {
        const startChar = operandsColumn + symbolMatch.index!;
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(line, startChar, line, startChar + symbolName.length),
          `Undefined symbol '${symbolName}'`,
          vscode.DiagnosticSeverity.Error
        );
        diagnostic.code = 'undefined-symbol';
        diagnostics.push(diagnostic);
      }
    }

    this.validateInstructionSyntax(instruction, operands, diagnostics);
  }

  private validateInstructionSyntax(
    instruction: InstructionLine,
    operands: string,
    diagnostics: vscode.Diagnostic[]
  ): void {
    const { line, mnemonic, operandsColumn, codeLength } = instruction;

    const requiresComma = new Set([
      'RDAX', 'WRAX', 'RDFX', 'WRLX', 'WRHX', 'MAXX',
      'SOF', 'LOG', 'EXP',
      'RDA', 'WRA', 'WRAP'
    ]);

    if (requiresComma.has(mnemonic)) {
      if (!operands.includes(',')) {
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(line, 0, line, codeLength),
          `Instruction '${mnemonic}' requires two operands separated by comma`,
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

    if (coefficientInstructions.has(mnemonic)) {
      const coeffMatch = /,\s*([-+]?\d+\.?\d*)/d.exec(operands);
      if (coeffMatch) {
        const coeff = parseFloat(coeffMatch[1]);

        // S1_14 fixed-point operand range is approximately -2.0 to 2.0.
        if (Math.abs(coeff) > 2.0) {
          const startChar = groupColumn(coeffMatch, 1, operandsColumn);
          const diagnostic = new vscode.Diagnostic(
            new vscode.Range(line, startChar, line, startChar + coeffMatch[1].length),
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
