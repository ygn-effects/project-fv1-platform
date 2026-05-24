import * as vscode from "vscode";
import { ResourceAnalyzer, ResourceUsage } from "./resourceAnalyzer";
import { REGISTER_COUNT, INSTRUCTION_LIMIT, MEMORY_SAMPLES } from "./fv1Constants";

let resourceStatusBar: vscode.StatusBarItem;
let currentUsage: ResourceUsage | null = null;

// Cached tooltip body so we don't rebuild the MarkdownString every debounce
// tick when nothing visible changed.
let lastTooltipDetail: string | null = null;

export function initializeResourceStatusBar(context: vscode.ExtensionContext): void {
  resourceStatusBar = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 99);
  resourceStatusBar.command = "spinasm.showResourceUsage";
  context.subscriptions.push(resourceStatusBar);

  context.subscriptions.push(
    vscode.window.onDidChangeActiveTextEditor((editor) => {
      if (editor && editor.document.languageId === 'spinasm') {
        updateResourceStatusBar(editor.document);
      } else {
        resourceStatusBar.hide();
      }
    })
  );

  context.subscriptions.push(
    vscode.workspace.onDidChangeTextDocument((e) => {
      if (e.document.languageId === 'spinasm') {
        debounceUpdate(e.document);
      }
    })
  );

  // Defer the first paint so activate() returns quickly.
  setImmediate(() => {
    const editor = vscode.window.activeTextEditor;
    if (editor && editor.document.languageId === 'spinasm') {
      updateResourceStatusBar(editor.document);
    }
  });
}

export function disposeResourceStatusBar(): void {
  if (resourceStatusBar) {
    resourceStatusBar.dispose();
  }
}

let updateTimer: NodeJS.Timeout | null = null;

function debounceUpdate(document: vscode.TextDocument): void {
  if (updateTimer) {
    clearTimeout(updateTimer);
  }

  updateTimer = setTimeout(() => {
    updateResourceStatusBar(document);
  }, 500);
}

function updateResourceStatusBar(document: vscode.TextDocument): void {
  try {
    const usage = ResourceAnalyzer.analyze(document);
    currentUsage = usage;

    resourceStatusBar.text = ResourceAnalyzer.formatStatusBar(usage);
    resourceStatusBar.color = ResourceAnalyzer.getStatusBarColor(
      ResourceAnalyzer.getWorstSeverity(usage)
    );

    const detail = ResourceAnalyzer.formatDetailed(usage);
    if (detail !== lastTooltipDetail) {
      const tooltip = new vscode.MarkdownString();
      tooltip.appendMarkdown('**SpinASM Resource Usage**\n\n');
      tooltip.appendMarkdown('Click for detailed breakdown\n\n');
      tooltip.appendCodeblock(detail, 'text');
      resourceStatusBar.tooltip = tooltip;
      lastTooltipDetail = detail;
    }

    resourceStatusBar.show();
  } catch {
    resourceStatusBar.hide();
  }
}

export async function showResourceUsage(): Promise<void> {
  if (!currentUsage) {
    vscode.window.showInformationMessage("No SpinASM file is currently open.");
    return;
  }

  const usage = currentUsage;
  const items = [];

  const regSeverity = ResourceAnalyzer.getSeverity(usage.registers.percentage);
  const regIcon = regSeverity === 'critical' ? '🔴' : regSeverity === 'warning' ? '⚠️' : '✅';

  items.push({
    label: `${regIcon} Registers: ${usage.registers.used.size} / ${REGISTER_COUNT}`,
    detail: `${Math.round(usage.registers.percentage)}% capacity`,
    description: regSeverity === 'critical' ? 'CRITICAL' : regSeverity === 'warning' ? 'WARNING' : 'OK'
  });

  if (usage.registers.aliases.size > 0) {
    const aliasedRegs = Array.from(usage.registers.aliases.entries())
      .sort((a, b) => a[1] - b[1])
      .map(([name, num]) => `reg${num} (${name})`)
      .join(', ');

    items.push({
      label: `   ${aliasedRegs}`,
      detail: 'Aliased registers in use'
    });
  }

  const unaliasedRegs = Array.from(usage.registers.used)
    .filter(num => !Array.from(usage.registers.aliases.values()).includes(num))
    .sort((a, b) => a - b);

  if (unaliasedRegs.length > 0) {
    items.push({
      label: `   ${unaliasedRegs.map(n => `reg${n}`).join(', ')}`,
      detail: 'Direct register usage (no alias)'
    });
  }

  items.push({ label: '', detail: '' });

  const instSeverity = ResourceAnalyzer.getSeverity(usage.instructions.percentage);
  const instIcon = instSeverity === 'critical' ? '🔴' : instSeverity === 'warning' ? '⚠️' : '✅';
  const remaining = usage.instructions.limit - usage.instructions.count;

  items.push({
    label: `${instIcon} Instructions: ${usage.instructions.count} / ${INSTRUCTION_LIMIT}`,
    detail: `${Math.round(usage.instructions.percentage)}% capacity - ${remaining} remaining`,
    description: instSeverity === 'critical' ? 'CRITICAL' : instSeverity === 'warning' ? 'WARNING' : 'OK'
  });

  if (instSeverity === 'critical') {
    items.push({
      label: `   ⚠️ Only ${remaining} instructions left!`,
      detail: 'Consider optimizing or splitting into multiple programs'
    });
  }
  else if (instSeverity === 'warning') {
    items.push({
      label: `   ${remaining} instructions remaining`,
      detail: 'Approaching limit - plan accordingly'
    });
  }

  items.push({ label: '', detail: '' });

  const memSeverity = ResourceAnalyzer.getSeverity(usage.memory.percentage);
  const memIcon = memSeverity === 'critical' ? '🔴' : memSeverity === 'warning' ? '⚠️' : '✅';
  const memRemaining = usage.memory.total - usage.memory.used;

  items.push({
    label: `${memIcon} Memory: ${usage.memory.used} / ${MEMORY_SAMPLES} samples`,
    detail: `${Math.round(usage.memory.percentage)}% capacity - ${memRemaining} samples remaining`,
    description: memSeverity === 'critical' ? 'CRITICAL' : memSeverity === 'warning' ? 'WARNING' : 'OK'
  });

  if (usage.memory.allocations.size > 0) {
    for (const [name, size] of usage.memory.allocations) {
      const percent = ((size / usage.memory.total) * 100).toFixed(1);
      items.push({
        label: `   ${name}: ${size} samples`,
        detail: `${percent}% of total memory`
      });
    }
  }
  else {
    items.push({
      label: `   No delay memory allocated`,
      detail: `All ${MEMORY_SAMPLES} samples available`
    });
  }

  await vscode.window.showQuickPick(items, {
    placeHolder: 'SpinASM Resource Usage - Current File'
  });
}
