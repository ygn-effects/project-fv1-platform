import * as vscode from "vscode";
import { DocumentParser, ResourceUsage } from "./documentParser";

export { ResourceUsage } from "./documentParser";

export type Severity = 'safe' | 'warning' | 'critical';

/**
 * @class ResourceAnalyzer
 * @brief Analyzes SpinASM code to track register, instruction, and memory usage
 * Fully optimized to query the cached Parser model.
 */
export class ResourceAnalyzer {

  /**
   * @brief Analyze a SpinASM document for resource usage (fetches from fast warm AST cache)
   */
  public static analyze(document: vscode.TextDocument): ResourceUsage {
    return DocumentParser.get(document).resourceUsage;
  }

  /**
   * @brief Get severity level based on percentage
   */
  public static getSeverity(percentage: number): Severity {
    if (percentage >= 90) return 'critical';
    if (percentage >= 75) return 'warning';
    return 'safe';
  }

  /**
   * @brief Get color for status bar based on severity
   */
  public static getStatusBarColor(severity: Severity): string | undefined {
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
  public static getWorstSeverity(usage: ResourceUsage): Severity {
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
