import * as vscode from "vscode";
import { isInstruction } from "./spinasmLanguage";

/**
 * @interface ResourceUsage
 * @brief Represents the resource usage of a SpinASM program
 */
export interface ResourceUsage {
  registers: {
    used: Set<number>;              // Set of register numbers in use
    aliases: Map<string, number>;   // Symbol name -> register number
    total: number;                  // Total available (32)
    percentage: number;             // Usage percentage
  };
  instructions: {
    count: number;                  // Number of instructions
    limit: number;                  // Instruction limit (128)
    percentage: number;             // Usage percentage
  };
  memory: {
    used: number;                   // Total memory used (samples)
    total: number;                  // Total available (32768)
    allocations: Map<string, number>; // Buffer name -> size
    percentage: number;             // Usage percentage
  };
}

/**
 * @class ResourceAnalyzer
 * @brief Analyzes SpinASM code to track register, instruction, and memory usage
 */
export class ResourceAnalyzer {

  // Cache keyed by document URI + version to avoid redundant analysis
  private static cache = new Map<string, { version: number; usage: ResourceUsage }>();

  /**
   * @brief Analyze a SpinASM document for resource usage (cached by document version)
   */
  public static analyze(document: vscode.TextDocument): ResourceUsage {
    const cacheKey = document.uri.toString();
    const cached = this.cache.get(cacheKey);

    if (cached && cached.version === document.version) {
      return cached.usage;
    }

    const text = document.getText();
    const lines = text.split('\n');

    // Initialize tracking
    const registers = new Set<number>();
    const registerAliases = new Map<string, number>();
    const memoryAllocations = new Map<string, number>();
    let instructionCount = 0;

    // Parse the document
    for (const line of lines) {
      // Remove comments
      const codeOnly = line.split(';')[0].trim();
      if (!codeOnly) continue;

      // Track register aliases (equ)
      this.parseRegisterAlias(codeOnly, registerAliases, registers);

      // Track memory allocations (mem)
      this.parseMemoryAllocation(codeOnly, memoryAllocations);

      // Track instructions
      if (isInstruction(codeOnly)) {
        instructionCount++;
        // Also track direct register usage in instructions
        this.parseDirectRegisterUsage(codeOnly, registers);
      }
    }

    // Calculate percentages
    const registerPercentage = (registers.size / 32) * 100;
    const instructionPercentage = (instructionCount / 128) * 100;

    const totalMemory = Array.from(memoryAllocations.values()).reduce((a, b) => a + b, 0);
    const memoryPercentage = (totalMemory / 32768) * 100;

    const usage: ResourceUsage = {
      registers: {
        used: registers,
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
    };

    this.cache.set(cacheKey, { version: document.version, usage });
    return usage;
  }

  /**
   * @brief Parse register alias declarations (equ)
   */
  private static parseRegisterAlias(
    line: string,
    aliases: Map<string, number>,
    registers: Set<number>
  ): void {
    // Match: equ <name> reg<N>
    const equMatch = /^\s*equ\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+reg(\d+)/i.exec(line);
    if (equMatch) {
      const symbolName = equMatch[1];
      const registerNum = parseInt(equMatch[2], 10);

      if (registerNum >= 0 && registerNum <= 31) {
        aliases.set(symbolName, registerNum);
        registers.add(registerNum);
      }
    }

    // Also match built-in registers like ADCL, ADCR, DACL, DACR
    // These are typically reg24-27 but we'll just track their usage
    const builtInMatch = /\b(adcl|adcr|dacl|dacr)\b/i.exec(line);
    if (builtInMatch) {
      // Built-in registers - we'll count them separately if needed
      // For now, just note they're used (don't add to set since they're implicit)
    }
  }

  /**
   * @brief Parse memory allocation declarations (mem)
   */
  private static parseMemoryAllocation(
    line: string,
    allocations: Map<string, number>
  ): void {
    // Match: mem <name> <size>
    const memMatch = /^\s*mem\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+(\d+)/i.exec(line);
    if (memMatch) {
      const bufferName = memMatch[1];
      const size = parseInt(memMatch[2], 10);

      allocations.set(bufferName, size);
    }
  }

  /**
   * @brief Parse direct register usage in instructions
   */
  private static parseDirectRegisterUsage(
    line: string,
    registers: Set<number>
  ): void {
    // Match direct register references like "reg5" or "REG12"
    const regMatches = line.matchAll(/\breg(\d+)\b/gi);
    for (const match of regMatches) {
      const registerNum = parseInt(match[1], 10);
      if (registerNum >= 0 && registerNum <= 31) {
        registers.add(registerNum);
      }
    }
  }

  /**
   * @brief Get severity level based on percentage
   */
  public static getSeverity(percentage: number): 'safe' | 'warning' | 'critical' {
    if (percentage >= 90) return 'critical';
    if (percentage >= 75) return 'warning';
    return 'safe';
  }

  /**
   * @brief Get color for status bar based on severity
   */
  public static getStatusBarColor(percentage: number): string | undefined {
    const severity = this.getSeverity(percentage);
    switch (severity) {
      case 'critical': return '#ff4444'; // Red
      case 'warning': return '#ffaa00';  // Orange/Yellow
      case 'safe': return undefined;     // Default color
    }
  }

  /**
   * @brief Format resource usage for status bar (compact)
   */
  public static formatStatusBar(usage: ResourceUsage): string {
    const r = usage.registers.used.size;
    const i = usage.instructions.count;
    const m = Math.round(usage.memory.used / 1024); // Convert to KB

    return `📊 R:${r}/32 I:${i}/128 M:${m}K`;
  }

  /**
   * @brief Get the worst severity across all resources
   */
  public static getWorstSeverity(usage: ResourceUsage): 'safe' | 'warning' | 'critical' {
    const severities = [
      this.getSeverity(usage.registers.percentage),
      this.getSeverity(usage.instructions.percentage),
      this.getSeverity(usage.memory.percentage)
    ];

    if (severities.includes('critical')) return 'critical';
    if (severities.includes('warning')) return 'warning';
    return 'safe';
  }

  /**
   * @brief Format detailed resource usage for tooltip or quick pick
   */
  public static formatDetailed(usage: ResourceUsage): string {
    const lines: string[] = [];

    // Registers
    const regSeverity = this.getSeverity(usage.registers.percentage);
    const regIcon = regSeverity === 'critical' ? '🔴' : regSeverity === 'warning' ? '⚠️' : '✓';
    lines.push(`${regIcon} Registers: ${usage.registers.used.size} / 32 (${Math.round(usage.registers.percentage)}%)`);

    if (usage.registers.used.size > 0) {
      const regList = Array.from(usage.registers.used).sort((a, b) => a - b);
      const aliasedRegs: string[] = [];

      // Show aliased registers
      for (const [name, num] of usage.registers.aliases) {
        aliasedRegs.push(`reg${num} (${name})`);
      }

      if (aliasedRegs.length > 0) {
        lines.push(`  Used: ${aliasedRegs.join(', ')}`);
      }
    }

    lines.push(''); // Blank line

    // Instructions
    const instSeverity = this.getSeverity(usage.instructions.percentage);
    const instIcon = instSeverity === 'critical' ? '🔴' : instSeverity === 'warning' ? '⚠️' : '✓';
    const remaining = usage.instructions.limit - usage.instructions.count;
    lines.push(`${instIcon} Instructions: ${usage.instructions.count} / 128 (${Math.round(usage.instructions.percentage)}%)`);

    if (instSeverity === 'critical') {
      lines.push(`  CRITICAL: Only ${remaining} instructions remaining!`);
    }
    else if (instSeverity === 'warning') {
      lines.push(`  Warning: ${remaining} instructions remaining`);
    }
    else {
      lines.push(`  Safe - ${remaining} instructions remaining`);
    }

    lines.push(''); // Blank line

    // Memory
    const memSeverity = this.getSeverity(usage.memory.percentage);
    const memIcon = memSeverity === 'critical' ? '🔴' : memSeverity === 'warning' ? '⚠️' : '✓';
    lines.push(`${memIcon} Memory: ${usage.memory.used} / 32768 samples (${Math.round(usage.memory.percentage)}%)`);

    if (usage.memory.allocations.size > 0) {
      for (const [name, size] of usage.memory.allocations) {
        lines.push(`  ${name}: ${size} samples`);
      }
    }
    else {
      lines.push('  No delay memory allocated');
    }

    return lines.join('\n');
  }
}
