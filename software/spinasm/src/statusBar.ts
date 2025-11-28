import * as vscode from "vscode";
import * as fs from "fs";
import * as path from "path";
import Project from "./project";
import Config from "./config";

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

  // Update status bar when active editor changes
  context.subscriptions.push(
    vscode.window.onDidChangeActiveTextEditor(() => updateBankStatusBar())
  );

  // Update status bar when document changes (for initial load)
  context.subscriptions.push(
    vscode.workspace.onDidOpenTextDocument(() => updateBankStatusBar())
  );

  // Update when settings change (in case user toggles visibility)
  context.subscriptions.push(
    vscode.workspace.onDidChangeConfiguration((e) => {
      if (e.affectsConfiguration('spinasm.statusBar.enabled')) {
        updateBankStatusBar();
      }
    })
  );

  // Initial update
  updateBankStatusBar();
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
 * @enum BankStatus
 * @brief Represents the compilation status of a bank
 */
enum BankStatus {
  Empty = 0,        // No .spn file
  NotCompiled = 1,  // .spn exists, no .hex
  UpToDate = 2,     // .hex exists and is newer than .spn
  OutOfDate = 3     // .hex exists but .spn is newer
}

/**
 * @interface BankInfo
 * @brief Information about a single bank
 */
interface BankInfo {
  status: BankStatus;
  spnFile?: string;
  hexFile?: string;
  spnTime?: Date;
  hexTime?: Date;
}

/**
 * @brief Get the compilation status of a bank with file age tracking
 */
async function getBankStatus(
  programs: (string | null)[],
  project: Project,
  bankIndex: number
): Promise<BankInfo> {

  const spnFile = programs[bankIndex];

  if (!spnFile) {
    return { status: BankStatus.Empty };
  }

  const hexFile = project.getOutput(bankIndex);

  if (!hexFile || !fs.existsSync(hexFile)) {
    return {
      status: BankStatus.NotCompiled,
      spnFile
    };
  }

  // Both files exist - compare modification times
  try {
    const spnStats = fs.statSync(spnFile);
    const hexStats = fs.statSync(hexFile);

    const spnTime = spnStats.mtime;
    const hexTime = hexStats.mtime;

    // If .hex is newer than .spn, it's up to date
    if (hexTime >= spnTime) {
      return {
        status: BankStatus.UpToDate,
        spnFile,
        hexFile,
        spnTime,
        hexTime
      };
    } else {
      // .spn is newer than .hex - needs recompilation
      return {
        status: BankStatus.OutOfDate,
        spnFile,
        hexFile,
        spnTime,
        hexTime
      };
    }
  } catch (error) {
    // If we can't stat the files, assume not compiled
    return {
      status: BankStatus.NotCompiled,
      spnFile,
      hexFile
    };
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
 * @brief Updates the status bar with current bank compilation status
 */
export async function updateBankStatusBar(): Promise<void> {
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
    const project = new Project(folder);
    const compilerPath = Config.getCompilerPath();
    const compilerArgs = Config.getCompilerArgs();

    if (!compilerPath) {
      bankStatusBar.hide();
      return;
    }

    await project.buildSetup(compilerPath, compilerArgs);
    const programs = project.getAllPrograms();

    let statusText = "SpinASM: ";
    let hasOutOfDate = false;

    for (let i = 0; i < 8; i++) {
      const bankInfo = await getBankStatus(programs, project, i);
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
    // If there's an error, just hide the status bar
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
    const project = new Project(folder);
    const compilerPath = Config.getCompilerPath();
    const compilerArgs = Config.getCompilerArgs();

    if (!compilerPath) {
      vscode.window.showWarningMessage("Compiler path not configured.");
      return;
    }

    await project.buildSetup(compilerPath, compilerArgs);
    const programs = project.getAllPrograms();

    const items = [];

    for (let i = 0; i < 8; i++) {
      const bankInfo = await getBankStatus(programs, project, i);
      let label = "";
      let detail = "";
      let description = "";

      if (bankInfo.status === BankStatus.Empty) {
        label = `- Bank ${i}: Empty`;
        detail = "No program file";
      } else {
        const fileName = path.basename(bankInfo.spnFile!);

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
      const programPath = programs[selection.bank];
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
