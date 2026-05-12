import * as vscode from "vscode";
import { ResourceAnalyzer, ResourceUsage } from "./resourceAnalyzer";

// Global status bar item for resource tracking
let resourceStatusBar: vscode.StatusBarItem;
let currentUsage: ResourceUsage | null = null;

/**
 * @brief Initialize the resource tracking status bar
 */
export function initializeResourceStatusBar(context: vscode.ExtensionContext): void {
  // Create status bar item (to the right of the bank status)
  resourceStatusBar = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 99);
  resourceStatusBar.command = "spinasm.showResourceUsage";
  context.subscriptions.push(resourceStatusBar);

  // Update when active editor changes
  context.subscriptions.push(
    vscode.window.onDidChangeActiveTextEditor((editor) => {
      if (editor && editor.document.languageId === 'spinasm') {
        updateResourceStatusBar(editor.document);
      } else {
        resourceStatusBar.hide();
      }
    })
  );

  // Update on document changes (typing)
  context.subscriptions.push(
    vscode.workspace.onDidChangeTextDocument((e) => {
      if (e.document.languageId === 'spinasm') {
        // Debounce updates - only update after user stops typing
        debounceUpdate(e.document);
      }
    })
  );

  // Initial update
  const editor = vscode.window.activeTextEditor;
  if (editor && editor.document.languageId === 'spinasm') {
    updateResourceStatusBar(editor.document);
  }
}

/**
 * @brief Dispose the resource status bar
 */
export function disposeResourceStatusBar(): void {
  if (resourceStatusBar) {
    resourceStatusBar.dispose();
  }
}

// Debounce timer
let updateTimer: NodeJS.Timeout | null = null;

/**
 * @brief Debounce resource updates to avoid updating on every keystroke
 */
function debounceUpdate(document: vscode.TextDocument): void {
  if (updateTimer) {
    clearTimeout(updateTimer);
  }

  updateTimer = setTimeout(() => {
    updateResourceStatusBar(document);
  }, 500); // Update 500ms after user stops typing
}

/**
 * @brief Update the resource status bar with current document analysis
 */
function updateResourceStatusBar(document: vscode.TextDocument): void {
  try {
    // Analyze the document
    const usage = ResourceAnalyzer.analyze(document);
    currentUsage = usage;

    // Format status bar text
    const statusText = ResourceAnalyzer.formatStatusBar(usage);
    resourceStatusBar.text = statusText;

    // Set color based on worst severity
    const worstSeverity = ResourceAnalyzer.getWorstSeverity(usage);
    const color = ResourceAnalyzer.getStatusBarColor(
      worstSeverity === 'critical' ? 95 :
      worstSeverity === 'warning' ? 80 :
      50
    );

    if (color) {
      resourceStatusBar.color = color;
    } else {
      resourceStatusBar.color = undefined;
    }

    // Set tooltip
    const tooltip = new vscode.MarkdownString();
    tooltip.appendMarkdown('**SpinASM Resource Usage**\n\n');
    tooltip.appendMarkdown('Click for detailed breakdown\n\n');
    tooltip.appendCodeblock(ResourceAnalyzer.formatDetailed(usage), 'text');
    resourceStatusBar.tooltip = tooltip;

    resourceStatusBar.show();

    // Show warnings if approaching limits
    showWarningsIfNeeded(usage);

  } catch (error) {
    // If analysis fails, hide the status bar
    resourceStatusBar.hide();
  }
}

// Track which warnings have already been shown to avoid spam
const shownWarnings = new Set<string>();

/**
 * @brief Show warning messages if resources are running low (once per resource)
 */
function showWarningsIfNeeded(usage: ResourceUsage): void {
  const instSeverity = ResourceAnalyzer.getSeverity(usage.instructions.percentage);
  const memSeverity = ResourceAnalyzer.getSeverity(usage.memory.percentage);
  const regSeverity = ResourceAnalyzer.getSeverity(usage.registers.percentage);

  if (instSeverity === 'critical' && !shownWarnings.has('instructions')) {
    shownWarnings.add('instructions');
    const remaining = usage.instructions.limit - usage.instructions.count;
    vscode.window.showWarningMessage(
      `SpinASM: Approaching instruction limit! Only ${remaining} instructions remaining (${usage.instructions.count}/128)`
    );
  } else if (instSeverity !== 'critical') {
    shownWarnings.delete('instructions');
  }

  if (memSeverity === 'critical' && !shownWarnings.has('memory')) {
    shownWarnings.add('memory');
    const remaining = usage.memory.total - usage.memory.used;
    vscode.window.showWarningMessage(
      `SpinASM: Approaching memory limit! Only ${remaining} samples remaining (${usage.memory.used}/32768)`
    );
  } else if (memSeverity !== 'critical') {
    shownWarnings.delete('memory');
  }

  if (regSeverity === 'critical' && !shownWarnings.has('registers')) {
    shownWarnings.add('registers');
    const remaining = usage.registers.total - usage.registers.used.size;
    vscode.window.showWarningMessage(
      `SpinASM: Approaching register limit! Only ${remaining} registers remaining (${usage.registers.used.size}/32)`
    );
  } else if (regSeverity !== 'critical') {
    shownWarnings.delete('registers');
  }
}

/**
 * @brief Show detailed resource usage in a QuickPick
 */
export async function showResourceUsage(): Promise<void> {
  if (!currentUsage) {
    vscode.window.showInformationMessage("No SpinASM file is currently open.");
    return;
  }

  const usage = currentUsage;

  const items = [];

  // === REGISTERS ===
  const regSeverity = ResourceAnalyzer.getSeverity(usage.registers.percentage);
  const regIcon = regSeverity === 'critical' ? '🔴' : regSeverity === 'warning' ? '⚠️' : '✅';

  items.push({
    label: `${regIcon} Registers: ${usage.registers.used.size} / 32`,
    detail: `${Math.round(usage.registers.percentage)}% capacity`,
    description: regSeverity === 'critical' ? 'CRITICAL' : regSeverity === 'warning' ? 'WARNING' : 'OK'
  });

  // Show register details
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

  // Show unaliased direct register usage
  const unaliasedRegs = Array.from(usage.registers.used)
    .filter(num => !Array.from(usage.registers.aliases.values()).includes(num))
    .sort((a, b) => a - b);

  if (unaliasedRegs.length > 0) {
    items.push({
      label: `   ${unaliasedRegs.map(n => `reg${n}`).join(', ')}`,
      detail: 'Direct register usage (no alias)'
    });
  }

  items.push({ label: '', detail: '' }); // Separator

  // === INSTRUCTIONS ===
  const instSeverity = ResourceAnalyzer.getSeverity(usage.instructions.percentage);
  const instIcon = instSeverity === 'critical' ? '🔴' : instSeverity === 'warning' ? '⚠️' : '✅';
  const remaining = usage.instructions.limit - usage.instructions.count;

  items.push({
    label: `${instIcon} Instructions: ${usage.instructions.count} / 128`,
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

  items.push({ label: '', detail: '' }); // Separator

  // === MEMORY ===
  const memSeverity = ResourceAnalyzer.getSeverity(usage.memory.percentage);
  const memIcon = memSeverity === 'critical' ? '🔴' : memSeverity === 'warning' ? '⚠️' : '✅';
  const memRemaining = usage.memory.total - usage.memory.used;

  items.push({
    label: `${memIcon} Memory: ${usage.memory.used} / 32768 samples`,
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
      detail: 'All 32768 samples available'
    });
  }

  await vscode.window.showQuickPick(items, {
    placeHolder: 'SpinASM Resource Usage - Current File'
  });
}

/**
 * @brief Force update of resource status bar (call after saving, compiling, etc.)
 */
export function forceUpdateResourceStatusBar(): void {
  const editor = vscode.window.activeTextEditor;
  if (editor && editor.document.languageId === 'spinasm') {
    updateResourceStatusBar(editor.document);
  }
}
