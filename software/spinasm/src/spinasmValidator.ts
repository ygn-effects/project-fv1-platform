import * as vscode from "vscode";
import { DocumentParser, DocumentSymbol } from "./documentParser";
import { ResourceUsage } from "./resourceAnalyzer";
import { isInstruction, isBuiltInSymbol } from "./spinasmLanguage";

/**
 * @class SpinASMValidator
 * @brief Validates SpinASM code and provides diagnostics
 */
export class SpinASMValidator {

  private diagnosticCollection: vscode.DiagnosticCollection;

  constructor() {
    this.diagnosticCollection = vscode.languages.createDiagnosticCollection('spinasm');
  }

  /**
   * @brief Validate a SpinASM document and update diagnostics
   */
  public validateDocument(document: vscode.TextDocument): void {
    if (document.languageId !== 'spinasm') {
      return;
    }

    const diagnostics = this.validate(document);
    this.diagnosticCollection.set(document.uri, diagnostics);
  }

  /**
   * @brief Clear diagnostics for a document
   */
  public clearDocument(document: vscode.TextDocument): void {
    this.diagnosticCollection.delete(document.uri);
  }

  /**
   * @brief Dispose the diagnostic collection
   */
  public dispose(): void {
    this.diagnosticCollection.dispose();
  }

  /**
   * @brief Main validation function querying unified cached parser
   */
  private validate(document: vscode.TextDocument): vscode.Diagnostic[] {
    const diagnostics: vscode.Diagnostic[] = [];

    // Extract unified parsed model
    const parsedDoc = DocumentParser.get(document);

    // 1. Add static structural syntax errors found on single-pass parse
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

    // 2. Validate symbol reference usages and operands
    for (let i = 0; i < document.lineCount; i++) {
      const line = document.lineAt(i);
      const lineText = line.text;

      // Skip comments and empty lines
      const codeOnly = lineText.split(';')[0].trim();
      if (!codeOnly) {
        continue;
      }

      // Skip directives and labels (which are validated during parse stage)
      if (/^\s*(equ|mem)\s+/i.test(codeOnly)) {
        continue;
      }

      if (/^\s*\w+:\s*$/.test(codeOnly)) {
        continue;
      }

      // Validate instructions
      if (isInstruction(codeOnly)) {
        this.validateInstruction(line, symbols, diagnostics);
      }
    }

    // 3. Evaluate memory & instruction resource boundary limits
    this.validateResourceLimits(parsedDoc.resourceUsage, diagnostics);

    return diagnostics;
  }

  /**
   * @brief Validate an instruction line
   */
  private validateInstruction(
    line: vscode.TextLine,
    symbols: Map<string, DocumentSymbol>,
    diagnostics: vscode.Diagnostic[]
  ): void {
    const lineText = line.text.split(';')[0]; // Strip comments

    // Extract instruction and operands, skipping an optional `label:` prefix.
    const match = /^\s*(?:[a-zA-Z_][a-zA-Z0-9_]*:\s*)?(\w+)\s+(.*)$/.exec(lineText);
    if (!match) {
      return;
    }

    const instruction = match[1].toUpperCase();
    const operands = match[2];

    // Validate symbol references in operands
    const symbolRefs = operands.matchAll(/\b([a-zA-Z_][a-zA-Z0-9_]*)\b/g);
    const operandStart = lineText.indexOf(operands);

    for (const symbolMatch of symbolRefs) {
      const symbolName = symbolMatch[1];
      const symbolUpper = symbolName.toUpperCase();

      // Skip if it's a built-in register or keyword
      if (isBuiltInSymbol(symbolUpper)) {
        continue;
      }

      // Skip numeric-looking constants
      if (/^\d/.test(symbolName)) {
        continue;
      }

      // Check if symbol is defined
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

    // Basic syntax validation
    this.validateInstructionSyntax(line, instruction, operands, diagnostics);
  }

  /**
   * @brief Validate basic instruction syntax
   */
  private validateInstructionSyntax(
    line: vscode.TextLine,
    instruction: string,
    operands: string,
    diagnostics: vscode.Diagnostic[]
  ): void {
    const lineText = line.text.split(';')[0];

    // Instructions that require a comma
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

    // Check for obvious coefficient range errors
    const coefficientInstructions = new Set([
      'RDAX', 'WRAX', 'RDFX', 'WRLX', 'WRHX', 'MAXX',
      'SOF', 'LOG', 'EXP', 'RDA', 'WRA', 'WRAP'
    ]);

    if (coefficientInstructions.has(instruction)) {
      const coeffMatch = /,\s*([-+]?\d+\.?\d*)/.exec(operands);
      if (coeffMatch) {
        const coeff = parseFloat(coeffMatch[1]);

        // Most instructions use S1_14 format: -2.0 to ~2.0
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

  /**
   * @brief Validate resource limits
   */
  private validateResourceLimits(
    usage: ResourceUsage,
    diagnostics: vscode.Diagnostic[]
  ): void {
    // Check instruction limit
    if (usage.instructions.count > usage.instructions.limit) {
      const diagnostic = new vscode.Diagnostic(
        new vscode.Range(0, 0, 0, 0),
        `Instruction limit exceeded: ${usage.instructions.count} / ${usage.instructions.limit}`,
        vscode.DiagnosticSeverity.Error
      );
      diagnostic.code = 'instruction-limit';
      diagnostics.push(diagnostic);
    }
    else if (usage.instructions.count > 120) {
      const diagnostic = new vscode.Diagnostic(
        new vscode.Range(0, 0, 0, 0),
        `Approaching instruction limit: ${usage.instructions.count} / ${usage.instructions.limit}`,
        vscode.DiagnosticSeverity.Warning
      );
      diagnostic.code = 'instruction-warning';
      diagnostics.push(diagnostic);
    }

    // Check memory limit
    if (usage.memory.used > usage.memory.total) {
      const diagnostic = new vscode.Diagnostic(
        new vscode.Range(0, 0, 0, 0),
        `Memory limit exceeded: ${usage.memory.used} / ${usage.memory.total} samples`,
        vscode.DiagnosticSeverity.Error
      );
      diagnostic.code = 'memory-limit';
      diagnostics.push(diagnostic);
    }
    else if (usage.memory.used > 29491) { // 90% of 32768
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
