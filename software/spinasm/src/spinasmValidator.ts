import * as vscode from "vscode";
import { ResourceAnalyzer } from "./resourceAnalyzer";

/**
 * @interface SymbolDefinition
 * @brief Represents a symbol defined in SpinASM code
 */
interface SymbolDefinition {
  name: string;
  type: 'register' | 'memory' | 'constant' | 'label';
  value?: string | number;
  line: number;
  character: number;
}

/**
 * @class SpinASMValidator
 * @brief Validates SpinASM code and provides diagnostics
 *
 * This validator catches common errors:
 * - Undefined symbols
 * - Duplicate definitions
 * - Invalid syntax
 * - Resource limit violations
 * - Basic instruction validation
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
   * @brief Clear all diagnostics
   */
  public clearAll(): void {
    this.diagnosticCollection.clear();
  }

  /**
   * @brief Dispose the diagnostic collection
   */
  public dispose(): void {
    this.diagnosticCollection.dispose();
  }

  /**
   * @brief Main validation function
   */
  private validate(document: vscode.TextDocument): vscode.Diagnostic[] {
    const diagnostics: vscode.Diagnostic[] = [];

    // Pass 1: Collect all symbol definitions
    const symbols = this.collectSymbols(document, diagnostics);

    // Pass 2: Validate symbol usage and instruction syntax
    for (let i = 0; i < document.lineCount; i++) {
      const line = document.lineAt(i);
      const lineText = line.text;

      // Skip comments and empty lines
      const codeOnly = lineText.split(';')[0].trim();
      if (!codeOnly) {
        continue;
      }

      // Skip directives and labels (already validated in pass 1)
      if (/^\s*(equ|mem)\s+/i.test(codeOnly)) {
        continue;
      }

      if (/^\s*\w+:\s*$/.test(codeOnly)) {
        continue;
      }

      // Validate instructions
      if (this.isInstruction(codeOnly)) {
        this.validateInstruction(line, symbols, diagnostics);
      }
    }

    // Pass 3: Check resource limits using ResourceAnalyzer
    this.validateResourceLimits(document, diagnostics);

    return diagnostics;
  }

  /**
   * @brief Collect all symbol definitions (equ, mem, labels)
   */
  private collectSymbols(
    document: vscode.TextDocument,
    diagnostics: vscode.Diagnostic[]
  ): Map<string, SymbolDefinition> {

    const symbols = new Map<string, SymbolDefinition>();
    const text = document.getText();

    // Reserved symbols (built-in registers, LFO names, etc.)
    const reserved = new Set([
      'ADCL', 'ADCR', 'DACL', 'DACR',
      'POT0', 'POT1', 'POT2',
      'SIN0', 'SIN1', 'RMP0', 'RMP1',
      'SIN0_RATE', 'SIN0_RANGE', 'SIN1_RATE', 'SIN1_RANGE',
      'RMP0_RATE', 'RMP0_RANGE', 'RMP1_RATE', 'RMP1_RANGE',
      'ADDR_PTR',
      'SIN', 'COS', 'REG', 'COMPC', 'COMPA', 'RPTR2', 'NA',
      'RUN', 'ZRC', 'ZRO', 'GEZ', 'NEG',
      'RDA', 'SOF', 'RDAL'  // Reserved as per asfv1 compiler
    ]);

    // Add REG0-REG31 to reserved
    for (let i = 0; i <= 31; i++) {
      reserved.add(`REG${i}`);
    }

    // Parse EQU declarations
    const equRegex = /^\s*equ\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(.+?)(?:;.*)?$/gmi;
    let match;

    while ((match = equRegex.exec(text)) !== null) {
      const symbolName = match[1];
      const symbolValue = match[2].trim();
      const line = text.substring(0, match.index).split('\n').length - 1;
      const character = match.index - text.lastIndexOf('\n', match.index) - 1;

      // Check for reserved names
      if (reserved.has(symbolName.toUpperCase())) {
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(line, character, line, character + 3 + symbolName.length),
          `Reserved symbol '${symbolName}' cannot be redefined`,
          vscode.DiagnosticSeverity.Error
        );
        diagnostic.code = 'reserved-symbol';
        diagnostics.push(diagnostic);
        continue;
      }

      // Check for duplicate definitions
      if (symbols.has(symbolName.toUpperCase())) {
        const prevDef = symbols.get(symbolName.toUpperCase())!;
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(line, character, line, character + 3 + symbolName.length),
          `Symbol '${symbolName}' already defined on line ${prevDef.line + 1}`,
          vscode.DiagnosticSeverity.Error
        );
        diagnostic.code = 'duplicate-symbol';
        diagnostics.push(diagnostic);
        continue;
      }

      // Determine if this is a register alias or constant
      const isRegister = /^(?:reg\d+|adcl|adcr|dacl|dacr|pot[0-2])$/i.test(symbolValue);

      symbols.set(symbolName.toUpperCase(), {
        name: symbolName,
        type: isRegister ? 'register' : 'constant',
        value: symbolValue,
        line: line,
        character: character
      });
    }

    // Parse MEM declarations
    const memRegex = /^\s*mem\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(\d+)/gmi;

    while ((match = memRegex.exec(text)) !== null) {
      const symbolName = match[1];
      const size = parseInt(match[2], 10);
      const line = text.substring(0, match.index).split('\n').length - 1;
      const character = match.index - text.lastIndexOf('\n', match.index) - 1;

      // Check for reserved names
      if (reserved.has(symbolName.toUpperCase())) {
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(line, character, line, character + 3 + symbolName.length),
          `Reserved symbol '${symbolName}' cannot be redefined`,
          vscode.DiagnosticSeverity.Error
        );
        diagnostic.code = 'reserved-symbol';
        diagnostics.push(diagnostic);
        continue;
      }

      // Check for duplicate definitions
      if (symbols.has(symbolName.toUpperCase())) {
        const prevDef = symbols.get(symbolName.toUpperCase())!;
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(line, character, line, character + 3 + symbolName.length),
          `Symbol '${symbolName}' already defined on line ${prevDef.line + 1}`,
          vscode.DiagnosticSeverity.Error
        );
        diagnostic.code = 'duplicate-symbol';
        diagnostics.push(diagnostic);
        continue;
      }

      // Validate memory size
      if (size <= 0 || size > 32767) {
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(line, 0, line, match[0].length),
          `Memory size ${size} out of range (must be 1-32767)`,
          vscode.DiagnosticSeverity.Error
        );
        diagnostic.code = 'invalid-memory-size';
        diagnostics.push(diagnostic);
      }

      symbols.set(symbolName.toUpperCase(), {
        name: symbolName,
        type: 'memory',
        value: size,
        line: line,
        character: character
      });
    }

    // Parse label definitions
    const labelRegex = /^\s*([a-zA-Z_][a-zA-Z0-9_]*):/gm;

    while ((match = labelRegex.exec(text)) !== null) {
      const symbolName = match[1];
      const line = text.substring(0, match.index).split('\n').length - 1;
      const character = match.index - text.lastIndexOf('\n', match.index) - 1;

      // Skip if already defined as equ/mem
      if (symbols.has(symbolName.toUpperCase())) {
        continue;
      }

      // Check for reserved names
      if (reserved.has(symbolName.toUpperCase())) {
        const diagnostic = new vscode.Diagnostic(
          new vscode.Range(line, character, line, character + symbolName.length + 1),
          `Reserved symbol '${symbolName}' cannot be used as label`,
          vscode.DiagnosticSeverity.Error
        );
        diagnostic.code = 'reserved-symbol';
        diagnostics.push(diagnostic);
        continue;
      }

      symbols.set(symbolName.toUpperCase(), {
        name: symbolName,
        type: 'label',
        line: line,
        character: character
      });
    }

    return symbols;
  }

  /**
   * @brief Check if a line contains an instruction
   */
  private isInstruction(line: string): boolean {
    const instructionPattern = /^\s*\b(wra|rda|rdax|wrax|sof|and|or|xor|skp|ldax|log|exp|mulx|rdfx|wrlx|wrhx|wrap|cho|wlds|wldr|jam|not|clr|absa|rmpa|maxx|nop|jmp|raw)\b/i;
    return instructionPattern.test(line);
  }

  /**
   * @brief Validate an instruction line
   */
  private validateInstruction(
    line: vscode.TextLine,
    symbols: Map<string, SymbolDefinition>,
    diagnostics: vscode.Diagnostic[]
  ): void {
    const lineText = line.text.split(';')[0]; // Remove comments

    // Extract instruction and operands
    const match = /^\s*(\w+)\s+(.*)$/.exec(lineText);
    if (!match) {
      return;
    }

    const instruction = match[1].toUpperCase();
    const operands = match[2];

    // Validate symbol references in operands
    // Look for potential symbol names (alphanumeric starting with letter)
    const symbolRefs = operands.matchAll(/\b([a-zA-Z_][a-zA-Z0-9_]*)\b/g);

    for (const symbolMatch of symbolRefs) {
      const symbolName = symbolMatch[1];
      const symbolUpper = symbolName.toUpperCase();

      // Skip if it's a built-in register or keyword
      if (this.isBuiltInSymbol(symbolUpper)) {
        continue;
      }

      // Skip numeric-looking things
      if (/^\d/.test(symbolName)) {
        continue;
      }

      // Check if symbol is defined
      if (!symbols.has(symbolUpper)) {
        const startChar = lineText.indexOf(symbolName);
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
   * @brief Check if a symbol is a built-in (register, LFO, etc.)
   */
  private isBuiltInSymbol(symbol: string): boolean {
    const builtIns = new Set([
      'ADCL', 'ADCR', 'DACL', 'DACR',
      'POT0', 'POT1', 'POT2',
      'SIN0', 'SIN1', 'RMP0', 'RMP1',
      'SIN0_RATE', 'SIN0_RANGE', 'SIN1_RATE', 'SIN1_RANGE',
      'RMP0_RATE', 'RMP0_RANGE', 'RMP1_RATE', 'RMP1_RANGE',
      'ADDR_PTR',
      'SIN', 'COS', 'REG', 'COMPC', 'COMPA', 'RPTR2', 'NA',
      'RUN', 'ZRC', 'ZRO', 'GEZ', 'NEG',
      'RDA', 'SOF', 'RDAL'
    ]);

    // Check for REGxx pattern
    if (/^REG\d+$/i.test(symbol)) {
      return true;
    }

    return builtIns.has(symbol);
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

    // Check for obvious coefficient range errors (basic check)
    // Only check for instructions that actually use coefficients
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
   * @brief Validate resource limits using ResourceAnalyzer
   */
  private validateResourceLimits(
    document: vscode.TextDocument,
    diagnostics: vscode.Diagnostic[]
  ): void {
    const usage = ResourceAnalyzer.analyze(document);

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
      // Warning when approaching limit
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