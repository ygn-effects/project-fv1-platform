import * as vscode from "vscode";
import { isInstruction, BUILT_IN_SYMBOLS } from "./spinasmLanguage";

/**
 * @interface DocumentSymbol
 * @brief Represents a parsed symbol definition in SpinASM code
 */
export interface DocumentSymbol {
  name: string;
  type: 'register' | 'memory' | 'constant' | 'label';
  value?: string | number;
  line: number;
  character: number;
  length: number;
}

/**
 * @interface ParserDiagnostic
 * @brief Lightweight representation of syntax/logical errors found during parsing
 */
export interface ParserDiagnostic {
  range: vscode.Range;
  message: string;
  severity: vscode.DiagnosticSeverity;
  code: string;
}

/**
 * @interface ResourceUsage
 * @brief Complete register, instruction, and memory metrics of the program
 */
export interface ResourceUsage {
  registers: {
    used: Set<number>;
    aliases: Map<string, number>;
    total: number;
    percentage: number;
  };
  instructions: {
    count: number;
    limit: number;
    percentage: number;
  };
  memory: {
    used: number;
    total: number;
    allocations: Map<string, number>;
    percentage: number;
  };
}

/**
 * @interface ParsedDocument
 * @brief Unified model representing the parsed state of a SpinASM document
 */
export interface ParsedDocument {
  symbols: Map<string, DocumentSymbol>; // Keyed by UPPERCASE name
  diagnostics: ParserDiagnostic[];
  resourceUsage: ResourceUsage;
}

/**
 * @class DocumentParser
 * @brief Central parser that analyzes SpinASM text exactly once per document version.
 * Caches results in-memory, providing O(1) syntax validation and resource metrics.
 */
export class DocumentParser {
  private static cache = new Map<string, { version: number; parsed: ParsedDocument }>();

  /**
   * @brief Gets the parsed representation of the document.
   * Leverages internal caching to bypass repetitive parsing on identical document versions.
   */
  public static get(document: vscode.TextDocument): ParsedDocument {
    const cacheKey = document.uri.toString();
    const cached = this.cache.get(cacheKey);

    if (cached && cached.version === document.version) {
      return cached.parsed;
    }

    const parsed = this.parse(document);
    this.cache.set(cacheKey, { version: document.version, parsed });
    return parsed;
  }

  /**
   * @brief Manually invalidates the parser cache for a specific document.
   */
  public static invalidate(document: vscode.TextDocument): void {
    this.cache.delete(document.uri.toString());
  }

  /**
   * @brief Single-pass parsing of the assembly document.
   */
  private static parse(document: vscode.TextDocument): ParsedDocument {
    const symbols = new Map<string, DocumentSymbol>();
    const diagnostics: ParserDiagnostic[] = [];
    const usedRegisters = new Set<number>();
    const registerAliases = new Map<string, number>();
    const memoryAllocations = new Map<string, number>();
    let instructionCount = 0;

    const reserved = BUILT_IN_SYMBOLS;

    for (let i = 0; i < document.lineCount; i++) {
      const line = document.lineAt(i);
      const lineText = line.text;
      const commentIndex = lineText.indexOf(';');
      const codeOnly = (commentIndex !== -1 ? lineText.substring(0, commentIndex) : lineText).trim();

      if (!codeOnly) {
        continue;
      }

      // 1. Parse EQU declarations: equ <name> <value>
      const equMatch = /^\s*equ\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(.+)$/i.exec(codeOnly);
      if (equMatch) {
        const symbolName = equMatch[1];
        const symbolValue = equMatch[2].trim();
        const symbolUpper = symbolName.toUpperCase();

        const charIndex = lineText.indexOf(symbolName);
        const actualChar = charIndex !== -1 ? charIndex : 0;

        // Validation: Reserved Name Check
        if (reserved.has(symbolUpper)) {
          diagnostics.push({
            range: new vscode.Range(i, actualChar, i, actualChar + symbolName.length),
            message: `Reserved symbol '${symbolName}' cannot be redefined`,
            severity: vscode.DiagnosticSeverity.Error,
            code: 'reserved-symbol'
          });
          continue;
        }

        // Validation: Duplicate Definition Check
        if (symbols.has(symbolUpper)) {
          const prevDef = symbols.get(symbolUpper)!;
          diagnostics.push({
            range: new vscode.Range(i, actualChar, i, actualChar + symbolName.length),
            message: `Symbol '${symbolName}' already defined on line ${prevDef.line + 1}`,
            severity: vscode.DiagnosticSeverity.Error,
            code: 'duplicate-symbol'
          });
          continue;
        }

        const isRegister = /^(?:reg\d+|adcl|adcr|dacl|dacr|pot[0-2])$/i.test(symbolValue);

        symbols.set(symbolUpper, {
          name: symbolName,
          type: isRegister ? 'register' : 'constant',
          value: symbolValue,
          line: i,
          character: actualChar,
          length: symbolName.length
        });

        // Registry for Resource Allocation
        const regNumMatch = /^reg(\d+)$/i.exec(symbolValue);
        if (regNumMatch) {
          const regNum = parseInt(regNumMatch[1], 10);
          if (regNum >= 0 && regNum <= 31) {
            registerAliases.set(symbolName, regNum);
            usedRegisters.add(regNum);
          }
        }
        continue;
      }

      // 2. Parse MEM declarations: mem <name> <size>
      const memMatch = /^\s*mem\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(\d+)/i.exec(codeOnly);
      if (memMatch) {
        const symbolName = memMatch[1];
        const sizeStr = memMatch[2];
        const size = parseInt(sizeStr, 10);
        const symbolUpper = symbolName.toUpperCase();

        const charIndex = lineText.indexOf(symbolName);
        const actualChar = charIndex !== -1 ? charIndex : 0;

        // Validation: Reserved Name Check
        if (reserved.has(symbolUpper)) {
          diagnostics.push({
            range: new vscode.Range(i, actualChar, i, actualChar + symbolName.length),
            message: `Reserved symbol '${symbolName}' cannot be redefined`,
            severity: vscode.DiagnosticSeverity.Error,
            code: 'reserved-symbol'
          });
          continue;
        }

        // Validation: Duplicate Definition Check
        if (symbols.has(symbolUpper)) {
          const prevDef = symbols.get(symbolUpper)!;
          diagnostics.push({
            range: new vscode.Range(i, actualChar, i, actualChar + symbolName.length),
            message: `Symbol '${symbolName}' already defined on line ${prevDef.line + 1}`,
            severity: vscode.DiagnosticSeverity.Error,
            code: 'duplicate-symbol'
          });
          continue;
        }

        // Validation: Size Boundary Limit
        if (size <= 0 || size > 32767) {
          const sizeCharIndex = lineText.indexOf(sizeStr);
          const sizePos = sizeCharIndex !== -1 ? sizeCharIndex : actualChar + symbolName.length + 1;
          diagnostics.push({
            range: new vscode.Range(i, sizePos, i, sizePos + sizeStr.length),
            message: `Memory size ${size} out of range (must be 1-32767)`,
            severity: vscode.DiagnosticSeverity.Error,
            code: 'invalid-memory-size'
          });
        }

        symbols.set(symbolUpper, {
          name: symbolName,
          type: 'memory',
          value: size,
          line: i,
          character: actualChar,
          length: symbolName.length
        });

        memoryAllocations.set(symbolName, size);
        continue;
      }

      // 3. Parse JUMP labels: <label>: (No early exit, permitting sequential line instructions to parse)
      const labelMatch = /^\s*([a-zA-Z_][a-zA-Z0-9_]*):/i.exec(codeOnly);
      if (labelMatch) {
        const symbolName = labelMatch[1];
        const symbolUpper = symbolName.toUpperCase();

        const charIndex = lineText.indexOf(symbolName);
        const actualChar = charIndex !== -1 ? charIndex : 0;

        // Validation: Reserved Name Check
        if (reserved.has(symbolUpper)) {
          diagnostics.push({
            range: new vscode.Range(i, actualChar, i, actualChar + symbolName.length + 1),
            message: `Reserved symbol '${symbolName}' cannot be used as label`,
            severity: vscode.DiagnosticSeverity.Error,
            code: 'reserved-symbol'
          });
        } else if (!symbols.has(symbolUpper)) {
          symbols.set(symbolUpper, {
            name: symbolName,
            type: 'label',
            line: i,
            character: actualChar,
            length: symbolName.length
          });
        }
      }

      // 4. Trace Instruction Counts & Register Usage in Real Time
      if (isInstruction(codeOnly)) {
        instructionCount++;

        // Track standard internal direct registers
        const regMatches = codeOnly.matchAll(/\breg(\d+)\b/gi);
        for (const rMatch of regMatches) {
          const registerNum = parseInt(rMatch[1], 10);
          if (registerNum >= 0 && registerNum <= 31) {
            usedRegisters.add(registerNum);
          }
        }
      }
    }

    // Compute hardware limits percentages
    const registerPercentage = (usedRegisters.size / 32) * 100;
    const instructionPercentage = (instructionCount / 128) * 100;
    const totalMemory = Array.from(memoryAllocations.values()).reduce((a, b) => a + b, 0);
    const memoryPercentage = (totalMemory / 32768) * 100;

    return {
      symbols,
      diagnostics,
      resourceUsage: {
        registers: {
          used: usedRegisters,
          aliases: registerAliases,
          total: 32,
          percentage: registerPercentage
        },
        instructions: {
          count: instructionCount,
          limit: 128,
          percentage: instructionPercentage
        },
        memory: {
          used: totalMemory,
          total: 32768,
          allocations: memoryAllocations,
          percentage: memoryPercentage
        }
      }
    };
  }
}
