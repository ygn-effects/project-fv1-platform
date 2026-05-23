import * as vscode from "vscode";
import * as path from "path";
import Config from "./config";
import { ProjectManager, BankStatus } from "./projectManager";

// Global status bar item
let bankStatusBar: vscode.StatusBarItem;

/**
 * @brief Initialize and register the status bar
 */
export function initializeBankStatusBar(context: vscode.ExtensionContext): void {
  // Create status bar item
  bankStatusBar = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 100);
  bankStatusBar.command = "spinasm.showBankStatus";
  context.subscriptions.push(bankStatusBar);

  // Subscribe to ProjectManager updates. UI updates immediately when the cache alters.
  context.subscriptions.push(
    ProjectManager.getInstance().onDidChangeProject(() => updateBankStatusBarImmediate())
  );

  // Update status bar instantly when active editor changes (No Disk I/O!)
  context.subscriptions.push(
    vscode.window.onDidChangeActiveTextEditor(() => updateBankStatusBarImmediate())
  );

  // Refresh the bar when a spinasm file is opened. Non-spinasm docs don't
  // affect bank status, so skip them to avoid rebuilding on every README open.
  context.subscriptions.push(
    vscode.workspace.onDidOpenTextDocument((doc) => {
      if (doc.languageId === 'spinasm') {
        updateBankStatusBarImmediate();
      }
    })
  );

  // Update when settings change
  context.subscriptions.push(
    vscode.workspace.onDidChangeConfiguration((e) => {
      if (e.affectsConfiguration('spinasm.statusBar.enabled')) {
        updateBankStatusBarImmediate();
      }
    })
  );

  // Initial update
  updateBankStatusBarImmediate();
}

/**
 * @brief Dispose the status bar on deactivation
 */
export function disposeBankStatusBar(): void {
  if (bankStatusBar) {
    bankStatusBar.dispose();
  }
}

/**
 * @brief Get the status symbol for a bank
 */
function getStatusSymbol(status: BankStatus): string {
  switch (status) {
    case BankStatus.Empty:
      return '-';
    case BankStatus.NotCompiled:
      return '✗';
    case BankStatus.UpToDate:
      return '✓';
    case BankStatus.OutOfDate:
      return '⚠';  // Warning symbol - hex is stale
    default:
      return '?';
  }
}

/**
 * @brief Updates the status bar with current bank compilation status synchronously from cache.
 */
function updateBankStatusBarImmediate(): void {
  // Check if status bar is enabled
  if (!Config.getStatusBarEnabled()) {
    bankStatusBar.hide();
    return;
  }

  const folder = vscode.workspace.workspaceFolders?.[0]?.uri.fsPath;

  if (!folder) {
    bankStatusBar.hide();
    return;
  }

  try {
    const manager = ProjectManager.getInstance();
    const cachedBanks = manager.getBanksSync(folder);

    let statusText = "SpinASM: ";
    let hasOutOfDate = false;

    for (let i = 0; i < 8; i++) {
      const bankInfo = cachedBanks[i];
      const symbol = getStatusSymbol(bankInfo.status);
      statusText += `[${i}${symbol}]`;

      if (bankInfo.status === BankStatus.OutOfDate) {
        hasOutOfDate = true;
      }
    }

    bankStatusBar.text = statusText;

    // Update tooltip based on status
    if (hasOutOfDate) {
      bankStatusBar.tooltip = "⚠ Some programs need recompilation (click for details)";
    } else {
      bankStatusBar.tooltip = "Click to view bank details";
    }

    bankStatusBar.show();
  } catch (error) {
    // Hide bar silently on parsing errors
    bankStatusBar.hide();
  }
}

/**
 * @brief Format time difference in a human-readable way
 */
function formatTimeDiff(date1: Date, date2: Date): string {
  const diffMs = Math.abs(date1.getTime() - date2.getTime());
  const diffSec = Math.floor(diffMs / 1000);
  const diffMin = Math.floor(diffSec / 60);
  const diffHour = Math.floor(diffMin / 60);
  const diffDay = Math.floor(diffHour / 24);

  if (diffDay > 0) {
    return `${diffDay} day${diffDay > 1 ? 's' : ''} ago`;
  } else if (diffHour > 0) {
    return `${diffHour} hour${diffHour > 1 ? 's' : ''} ago`;
  } else if (diffMin > 0) {
    return `${diffMin} minute${diffMin > 1 ? 's' : ''} ago`;
  } else {
    return `${diffSec} second${diffSec > 1 ? 's' : ''} ago`;
  }
}

/**
 * @brief Shows detailed information about bank compilation status
 */
export async function showBankStatus(): Promise<void> {
  const folder = vscode.workspace.workspaceFolders?.[0]?.uri.fsPath;

  if (!folder) {
    vscode.window.showInformationMessage("No workspace folder open.");
    return;
  }

  try {
    const manager = ProjectManager.getInstance();
    const cachedBanks = manager.getBanksSync(folder);
    const items = [];

    for (let i = 0; i < 8; i++) {
      const bankInfo = cachedBanks[i];
      let label = "";
      let detail = "";
      let description = "";

      if (bankInfo.status === BankStatus.Empty || !bankInfo.spnFile) {
        label = `- Bank ${i}: Empty`;
        detail = "No program file";
      } else {
        const fileName = path.basename(bankInfo.spnFile);

        switch (bankInfo.status) {
          case BankStatus.UpToDate:
            label = `✓ Bank ${i}: ${fileName}`;
            detail = "Compiled and ready to upload";
            if (bankInfo.hexTime) {
              description = `Compiled ${formatTimeDiff(new Date(), bankInfo.hexTime)}`;
            }
            break;

          case BankStatus.OutOfDate:
            label = `⚠ Bank ${i}: ${fileName}`;
            detail = "Source modified after compilation - needs recompile";
            if (bankInfo.spnTime && bankInfo.hexTime) {
              description = `Modified ${formatTimeDiff(bankInfo.hexTime, bankInfo.spnTime)} after compile`;
            }
            break;

          case BankStatus.NotCompiled:
            label = `✗ Bank ${i}: ${fileName}`;
            detail = "Not compiled yet";
            break;
        }
      }

      items.push({
        label: label,
        detail: detail,
        description: description,
        bank: i,
        hasProgram: bankInfo.status !== BankStatus.Empty,
        needsCompile: bankInfo.status === BankStatus.NotCompiled || bankInfo.status === BankStatus.OutOfDate
      });
    }

    const selection = await vscode.window.showQuickPick(items, {
      placeHolder: "Bank Status - Select a bank to open its file"
    });

    if (selection && selection.hasProgram) {
      const programPath = cachedBanks[selection.bank].spnFile;
      if (programPath) {
        const doc = await vscode.workspace.openTextDocument(programPath);
        await vscode.window.showTextDocument(doc);

        // If the bank needs compilation, offer to compile it
        if (selection.needsCompile) {
          const action = await vscode.window.showInformationMessage(
            `Bank ${selection.bank} needs compilation`,
            "Compile Now",
            "Compile & Upload"
          );

          if (action === "Compile Now") {
            vscode.commands.executeCommand("spinasm.compileCurrentProgram");
          } else if (action === "Compile & Upload") {
            vscode.commands.executeCommand("spinasm.compileAndUploadCurrentProgram");
          }
        }
      }
    }
  } catch (error) {
    vscode.window.showErrorMessage(`Failed to show bank status: ${(error as Error).message}`);
  }
}
