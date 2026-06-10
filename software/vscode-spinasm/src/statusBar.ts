import * as vscode from "vscode";
import * as path from "path";
import Config from "./config";
import { ProjectManager, BankStatus } from "./projectManager";
import { BANK_COUNT } from "./fv1Constants";

let bankStatusBar: vscode.StatusBarItem;

export function initializeBankStatusBar(context: vscode.ExtensionContext): void {
  bankStatusBar = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 100);
  bankStatusBar.command = "spinasm.showBankStatus";
  context.subscriptions.push(bankStatusBar);

  context.subscriptions.push(
    ProjectManager.getInstance().onDidChangeProject(() => updateBankStatusBarImmediate())
  );

  context.subscriptions.push(
    vscode.window.onDidChangeActiveTextEditor(() => updateBankStatusBarImmediate())
  );

  // Non-spinasm docs can't affect bank status, so skip them to avoid
  // rebuilding on every README open.
  context.subscriptions.push(
    vscode.workspace.onDidOpenTextDocument((doc) => {
      if (doc.languageId === 'spinasm') {
        updateBankStatusBarImmediate();
      }
    })
  );

  context.subscriptions.push(
    vscode.workspace.onDidChangeConfiguration((e) => {
      if (e.affectsConfiguration('spinasm.statusBar.enabled')) {
        updateBankStatusBarImmediate();
      }
    })
  );

  updateBankStatusBarImmediate();
}

export function disposeBankStatusBar(): void {
  if (bankStatusBar) {
    bankStatusBar.dispose();
  }
}

/**
 * The folder whose banks the status bar shows: the active editor's workspace
 * folder when there is one, else the first folder. Keeps the bar consistent
 * with the folder-picking commands in multi-root workspaces.
 */
function getStatusBarFolder(): string | undefined {
  const activeUri = vscode.window.activeTextEditor?.document.uri;
  if (activeUri) {
    const folder = vscode.workspace.getWorkspaceFolder(activeUri);
    if (folder) {
      return folder.uri.fsPath;
    }
  }
  return vscode.workspace.workspaceFolders?.[0]?.uri.fsPath;
}

function getStatusSymbol(status: BankStatus): string {
  switch (status) {
    case BankStatus.Empty:
      return '-';
    case BankStatus.NotCompiled:
      return '✗';
    case BankStatus.UpToDate:
      return '✓';
    case BankStatus.OutOfDate:
      return '⚠';
    default:
      return '?';
  }
}

function updateBankStatusBarImmediate(): void {
  if (!Config.getStatusBarEnabled()) {
    bankStatusBar.hide();
    return;
  }

  const folder = getStatusBarFolder();

  if (!folder) {
    bankStatusBar.hide();
    return;
  }

  try {
    const manager = ProjectManager.getInstance();
    const cachedBanks = manager.getBanksSync(folder);

    let statusText = "SpinASM: ";
    let hasOutOfDate = false;
    let hasAmbiguous = false;

    for (let i = 0; i < BANK_COUNT; i++) {
      const bankInfo = cachedBanks[i];
      const symbol = getStatusSymbol(bankInfo.status);
      statusText += `[${i}${symbol}]`;

      if (bankInfo.status === BankStatus.OutOfDate) {
        hasOutOfDate = true;
      }
      if (bankInfo.extraFiles.length > 0) {
        hasAmbiguous = true;
      }
    }

    bankStatusBar.text = statusText;

    const tooltipLines: string[] = [];
    if (hasOutOfDate) {
      tooltipLines.push("⚠ Some programs need recompilation");
    }
    if (hasAmbiguous) {
      tooltipLines.push("⚠ Some banks contain more than one .spn file");
    }
    tooltipLines.push("Click for details");
    bankStatusBar.tooltip = tooltipLines.join("\n");

    bankStatusBar.show();
  } catch {
    bankStatusBar.hide();
  }
}

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

export async function showBankStatus(): Promise<void> {
  const folder = getStatusBarFolder();

  if (!folder) {
    vscode.window.showInformationMessage("No workspace folder open.");
    return;
  }

  try {
    const manager = ProjectManager.getInstance();
    const cachedBanks = manager.getBanksSync(folder);
    const items = [];

    for (let i = 0; i < BANK_COUNT; i++) {
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

        if (bankInfo.extraFiles.length > 0) {
          const ignored = bankInfo.extraFiles.map(f => path.basename(f)).join(", ");
          detail = `${detail}  •  ⚠ also in folder (ignored): ${ignored}`;
          description = description ? `${description} • ⚠ ambiguous` : "⚠ ambiguous";
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
