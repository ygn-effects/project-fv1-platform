import * as vscode from "vscode";
import { DocumentParser, ResourceUsage } from "./documentParser";
import { REGISTER_COUNT, INSTRUCTION_LIMIT, MEMORY_SAMPLES } from "./fv1Constants";

export { ResourceUsage } from "./documentParser";

export type Severity = 'safe' | 'warning' | 'critical';

export class ResourceAnalyzer {

  public static analyze(document: vscode.TextDocument): ResourceUsage {
    return DocumentParser.get(document).resourceUsage;
  }

  public static getSeverity(percentage: number): Severity {
    if (percentage >= 90) {return 'critical';}
    if (percentage >= 75) {return 'warning';}
    return 'safe';
  }

  public static getStatusBarBackground(severity: Severity): vscode.ThemeColor | undefined {
    switch (severity) {
      case 'critical': return new vscode.ThemeColor('statusBarItem.errorBackground');
      case 'warning': return new vscode.ThemeColor('statusBarItem.warningBackground');
      case 'safe': return undefined;
    }
  }

  public static formatStatusBar(usage: ResourceUsage): string {
    const r = usage.registers.used.size;
    const i = usage.instructions.count;
    const m = Math.round(usage.memory.used / 1024);

    return `📊 R:${r}/${REGISTER_COUNT} I:${i}/${INSTRUCTION_LIMIT} M:${m}K`;
  }

  public static getWorstSeverity(usage: ResourceUsage): Severity {
    const severities = [
      this.getSeverity(usage.registers.percentage),
      this.getSeverity(usage.instructions.percentage),
      this.getSeverity(usage.memory.percentage)
    ];

    if (severities.includes('critical')) {return 'critical';}
    if (severities.includes('warning')) {return 'warning';}
    return 'safe';
  }

  public static formatDetailed(usage: ResourceUsage): string {
    const lines: string[] = [];

    const regSeverity = this.getSeverity(usage.registers.percentage);
    const regIcon = regSeverity === 'critical' ? '🔴' : regSeverity === 'warning' ? '⚠️' : '✓';
    lines.push(`${regIcon} Registers: ${usage.registers.used.size} / ${REGISTER_COUNT} (${Math.round(usage.registers.percentage)}%)`);

    if (usage.registers.used.size > 0) {
      const aliasedRegs: string[] = [];
      for (const [name, num] of usage.registers.aliases) {
        aliasedRegs.push(`reg${num} (${name})`);
      }
      if (aliasedRegs.length > 0) {
        lines.push(`  Used: ${aliasedRegs.join(', ')}`);
      }
    }

    lines.push('');

    const instSeverity = this.getSeverity(usage.instructions.percentage);
    const instIcon = instSeverity === 'critical' ? '🔴' : instSeverity === 'warning' ? '⚠️' : '✓';
    const remaining = usage.instructions.limit - usage.instructions.count;
    lines.push(`${instIcon} Instructions: ${usage.instructions.count} / ${INSTRUCTION_LIMIT} (${Math.round(usage.instructions.percentage)}%)`);

    if (instSeverity === 'critical') {
      lines.push(`  CRITICAL: Only ${remaining} instructions remaining!`);
    }
    else if (instSeverity === 'warning') {
      lines.push(`  Warning: ${remaining} instructions remaining`);
    }
    else {
      lines.push(`  Safe - ${remaining} instructions remaining`);
    }

    lines.push('');

    const memSeverity = this.getSeverity(usage.memory.percentage);
    const memIcon = memSeverity === 'critical' ? '🔴' : memSeverity === 'warning' ? '⚠️' : '✓';
    lines.push(`${memIcon} Memory: ${usage.memory.used} / ${MEMORY_SAMPLES} samples (${Math.round(usage.memory.percentage)}%)`);

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
