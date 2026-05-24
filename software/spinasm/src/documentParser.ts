import * as vscode from "vscode";
import { isInstruction, BUILT_IN_SYMBOLS } from "./spinasmLanguage";
import { INSTRUCTION_LIMIT, REGISTER_COUNT, MEMORY_SAMPLES, MEMORY_MAX_INDEX } from "./fv1Constants";

export interface DocumentSymbol {
  name: string;
  type: 'register' | 'memory' | 'constant' | 'label';
  value?: string | number;
  line: number;
  character: number;
  length: number;
}

export interface ParserDiagnostic {
  range: vscode.Range;
  message: string;
  severity: vscode.DiagnosticSeverity;
  code: string;
}

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

export interface ParsedDocument {
  /** Keyed by UPPERCASE symbol name. */
  symbols: Map<string, DocumentSymbol>;
  diagnostics: ParserDiagnostic[];
  resourceUsage: ResourceUsage;
}

/**
 * Parses each document exactly once per version and caches the result. Hover,
 * resource analysis, and validation all read from this single representation.
 */
export class DocumentParser {
  private static cache = new Map<string, { version: number; parsed: ParsedDocument }>();

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

  public static invalidate(document: vscode.TextDocument): void {
    this.cache.delete(document.uri.toString());
  }

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

      // equ <name> <value>
      const equMatch = /^\s*equ\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(.+)$/i.exec(codeOnly);
      if (equMatch) {
        const symbolName = equMatch[1];
        const symbolValue = equMatch[2].trim();
        const symbolUpper = symbolName.toUpperCase();

        const charIndex = lineText.indexOf(symbolName);
        const actualChar = charIndex !== -1 ? charIndex : 0;

        if (reserved.has(symbolUpper)) {
          diagnostics.push({
            range: new vscode.Range(i, actualChar, i, actualChar + symbolName.length),
            message: `Reserved symbol '${symbolName}' cannot be redefined`,
            severity: vscode.DiagnosticSeverity.Error,
            code: 'reserved-symbol'
          });
          continue;
        }

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

        // An equ that aliases a regN counts toward register usage.
        const regNumMatch = /^reg(\d+)$/i.exec(symbolValue);
        if (regNumMatch) {
          const regNum = parseInt(regNumMatch[1], 10);
          if (regNum >= 0 && regNum < REGISTER_COUNT) {
            registerAliases.set(symbolName, regNum);
            usedRegisters.add(regNum);
          }
        }
        continue;
      }

      // mem <name> <size>
      const memMatch = /^\s*mem\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(\d+)/i.exec(codeOnly);
      if (memMatch) {
        const symbolName = memMatch[1];
        const sizeStr = memMatch[2];
        const size = parseInt(sizeStr, 10);
        const symbolUpper = symbolName.toUpperCase();

        const charIndex = lineText.indexOf(symbolName);
        const actualChar = charIndex !== -1 ? charIndex : 0;

        if (reserved.has(symbolUpper)) {
          diagnostics.push({
            range: new vscode.Range(i, actualChar, i, actualChar + symbolName.length),
            message: `Reserved symbol '${symbolName}' cannot be redefined`,
            severity: vscode.DiagnosticSeverity.Error,
            code: 'reserved-symbol'
          });
          continue;
        }

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

        if (size <= 0 || size > MEMORY_MAX_INDEX) {
          const sizeCharIndex = lineText.indexOf(sizeStr);
          const sizePos = sizeCharIndex !== -1 ? sizeCharIndex : actualChar + symbolName.length + 1;
          diagnostics.push({
            range: new vscode.Range(i, sizePos, i, sizePos + sizeStr.length),
            message: `Memory size ${size} out of range (must be 1-${MEMORY_MAX_INDEX})`,
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

      // Jump labels `<label>:`. No `continue` here: the rest of the line may
      // also be an instruction (e.g., `start: SOF 0,0`).
      const labelMatch = /^\s*([a-zA-Z_][a-zA-Z0-9_]*):/i.exec(codeOnly);
      if (labelMatch) {
        const symbolName = labelMatch[1];
        const symbolUpper = symbolName.toUpperCase();

        const charIndex = lineText.indexOf(symbolName);
        const actualChar = charIndex !== -1 ? charIndex : 0;

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

      if (isInstruction(codeOnly)) {
        instructionCount++;

        const regMatches = codeOnly.matchAll(/\breg(\d+)\b/gi);
        for (const rMatch of regMatches) {
          const registerNum = parseInt(rMatch[1], 10);
          if (registerNum >= 0 && registerNum < REGISTER_COUNT) {
            usedRegisters.add(registerNum);
          }
        }
      }
    }

    const registerPercentage = (usedRegisters.size / REGISTER_COUNT) * 100;
    const instructionPercentage = (instructionCount / INSTRUCTION_LIMIT) * 100;
    const totalMemory = Array.from(memoryAllocations.values()).reduce((a, b) => a + b, 0);
    const memoryPercentage = (totalMemory / MEMORY_SAMPLES) * 100;

    return {
      symbols,
      diagnostics,
      resourceUsage: {
        registers: {
          used: usedRegisters,
          aliases: registerAliases,
          total: REGISTER_COUNT,
          percentage: registerPercentage
        },
        instructions: {
          count: instructionCount,
          limit: INSTRUCTION_LIMIT,
          percentage: instructionPercentage
        },
        memory: {
          used: totalMemory,
          total: MEMORY_SAMPLES,
          allocations: memoryAllocations,
          percentage: memoryPercentage
        }
      }
    };
  }
}
