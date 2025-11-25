import * as vscode from "vscode";
import Project from "./project";
import Config from "./config";
import Logs, { LogType } from "./logs";
import Utils from "./utils";
import Programmer from "./programmer";
import * as path from "path";
import * as fs from "fs";

// Global status bar item
let bankStatusBar: vscode.StatusBarItem;

export function activate(context: vscode.ExtensionContext): void {
  Logs.createChannel();
  Logs.log(LogType.INFO, "Extension activated");

  // Create status bar item
  bankStatusBar = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 100);
  bankStatusBar.command = "spinasm.showBankStatus";
  context.subscriptions.push(bankStatusBar);

  // Register file system watcher for compile-on-save
  const watcher = vscode.workspace.createFileSystemWatcher("**/*.spn");
  context.subscriptions.push(watcher);

  watcher.onDidChange(async (uri) => {
    if (Config.getCompileOnSave()) {
      await handleCompileOnSave(uri);
    }
  });

  // Update status bar when active editor changes
  context.subscriptions.push(
    vscode.window.onDidChangeActiveTextEditor(() => updateBankStatusBar())
  );

  // Update status bar when document changes (for initial load)
  context.subscriptions.push(
    vscode.workspace.onDidOpenTextDocument(() => updateBankStatusBar())
  );

  context.subscriptions.push(
    // Global / Project Management
    vscode.commands.registerCommand("spinasm.createProject", createProject),
    vscode.commands.registerCommand("spinasm.checkProjectSettings", checkHardwareConnection),
    vscode.commands.registerCommand("spinasm.showSerialConfig", showConfig),
    vscode.commands.registerCommand("spinasm.showBankStatus", showBankStatus),

    // Serial Port Detection
    vscode.commands.registerCommand("spinasm.selectSerialPort", selectSerialPort),
    vscode.commands.registerCommand("spinasm.autoDetectProgrammer", autoDetectProgrammer),

    // Current File Operations
    vscode.commands.registerCommand("spinasm.compileCurrentProgram", compileCurrentProgram),
    vscode.commands.registerCommand("spinasm.uploadCurrentProgram", uploadCurrentProgram),
    vscode.commands.registerCommand("spinasm.compileAndUploadCurrentProgram", compileAndUploadCurrentProgram),

    // Batch Operations
    vscode.commands.registerCommand("spinasm.compileAllPrograms", compileAllPrograms),
    vscode.commands.registerCommand("spinasm.compileAllProgramsToBin", compileAllProgramsToBin),
    vscode.commands.registerCommand("spinasm.uploadAllPrograms", uploadAllPrograms),
    vscode.commands.registerCommand("spinasm.compileAndUploadAllPrograms", compileAndUploadAllPrograms),

    // Generic Bank Operations (Prompts user for bank 0-7)
    vscode.commands.registerCommand("spinasm.compileBank", async () => {
      const bank = await pickBank();

      if (bank !== undefined) {
        await compileBank(bank);
      }
    }),

    vscode.commands.registerCommand("spinasm.uploadBank", async () => {
      const bank = await pickBank();

      if (bank !== undefined) {
        await uploadBank(bank);
      }
    }),

    vscode.commands.registerCommand("spinasm.compileAndUploadBank", async () => {
      const bank = await pickBank();

      if (bank !== undefined) {
        await compileAndUploadBank(bank);
      }
    })
  );

  // Warn user if config is missing on startup
  if (Config.isConfigMissing()) {
    vscode.window.showWarningMessage("SpinASM: Compiler path or Serial port is not configured. Please check your Settings.");
  }

  Logs.log(LogType.INFO, "Commands registered successfully");
}

export function deactivate(): void {
  Logs.disposeChannel();
}

// =============================================================================
// STATUS BAR MANAGEMENT
// =============================================================================

/**
 * @brief Updates the status bar with current bank compilation status.
 */
async function updateBankStatusBar(): Promise<void> {
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

    for (let i = 0; i < 8; i++) {
      if (programs[i]) {
        // Check if hex file exists
        const hexFile = project.getOutput(i);
        const isCompiled = hexFile && fs.existsSync(hexFile);
        statusText += isCompiled ? `[${i}✓]` : `[${i}✗]`;
      } else {
        statusText += `[${i}-]`;
      }
    }

    bankStatusBar.text = statusText;
    bankStatusBar.tooltip = "Click to view bank details";
    bankStatusBar.show();
  }
  catch (error) {
    // If there's an error, just hide the status bar
    bankStatusBar.hide();
  }
}

/**
 * @brief Shows detailed information about bank compilation status.
 */
async function showBankStatus(): Promise<void> {
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
      let status = "";
      let detail = "";

      if (programs[i]) {
        const hexFile = project.getOutput(i);
        const isCompiled = hexFile && fs.existsSync(hexFile);
        const fileName = path.basename(programs[i]!);

        if (isCompiled) {
          status = `✓ Bank ${i}: ${fileName}`;
          detail = "Compiled and ready to upload";
        }
        else {
          status = `✗ Bank ${i}: ${fileName}`;
          detail = "Not compiled yet";
        }
      }
      else {
        status = `- Bank ${i}: Empty`;
        detail = "No program file";
      }

      items.push({
        label: status,
        detail: detail,
        bank: i,
        hasProgram: !!programs[i]
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
      }
    }
  }
  catch (error) {
    handleError(error, "Failed to show bank status");
  }
}

// =============================================================================
// COMPILE ON SAVE
// =============================================================================

/**
 * @brief Handles automatic compilation when a .spn file is saved.
 */
async function handleCompileOnSave(uri: vscode.Uri): Promise<void> {
  const folder = vscode.workspace.getWorkspaceFolder(uri)?.uri.fsPath;

  if (!folder) {
    return;
  }

  try {
    const project = new Project(folder);
    const compilerPath = Config.getCompilerPath();
    const compilerArgs = Config.getCompilerArgs();

    if (!compilerPath) {
      return; // Silently skip if compiler not configured
    }

    await project.buildSetup(compilerPath, compilerArgs);
    const bank = project.getProgramBankByPath(uri.fsPath);

    if (bank === -1) {
      return; // Not a valid project program
    }

    Logs.log(LogType.INFO, `Compile-on-save: Compiling bank ${bank}...`);
    await project.compileProgramToHex(bank);
    Logs.log(LogType.INFO, `Compile-on-save: Bank ${bank} compiled successfully`);

    // Update status bar after successful compilation
    await updateBankStatusBar();

  } catch (error) {
    // Log error but don't show intrusive notifications for auto-compile
    Logs.log(LogType.ERROR, `Compile-on-save failed: ${(error as Error).message}`);
  }
}

// =============================================================================
// UI HELPERS
// =============================================================================

/**
 * @brief prompts the user to select a bank number (0-7) from a dropdown.
 * @returns The selected bank number, or undefined if cancelled.
 */
async function pickBank(): Promise<number | undefined> {
  const items = [];

  for (let i = 0; i < 8; i++) {
    items.push({
      label: `Bank ${i}`,
      description: `Program ${i}`,
      bankId: i
    });
  }

  const selection = await vscode.window.showQuickPick(items, {
    placeHolder: "Select the target EEPROM bank (0-7)",
  });

  return selection ? selection.bankId : undefined;
}

// =============================================================================
// SERIAL PORT DETECTION
// =============================================================================

/**
 * @brief Presents a list of available serial ports for manual selection.
 */
async function selectSerialPort(): Promise<void> {
  try {
    const ports = await Utils.listSerialPorts();

    if (ports.length === 0) {
      vscode.window.showErrorMessage("No serial ports detected on this system.");
      return;
    }

    const items = ports.map((port: any) => ({
      label: port.path,
      description: port.manufacturer || "",
      detail: `VID: ${port.vendorId || 'N/A'} | PID: ${port.productId || 'N/A'}`,
      path: port.path
    }));

    const selection = await vscode.window.showQuickPick(items, {
      placeHolder: "Select serial port for FV-1 programmer"
    });

    if (selection) {
      await Config.setSerialPort(selection.path);

      Logs.log(LogType.INFO, `Serial port set to: ${selection.path}`);
      vscode.window.showInformationMessage(`Serial port set to: ${selection.path}`);
    }
  }
  catch (error) {
    handleError(error, "Failed to list serial ports");
  }
}

/**
 * @brief Automatically detects the FV-1 programmer by probing available ports.
 */
async function autoDetectProgrammer(): Promise<void> {
  await vscode.window.withProgress({
    location: vscode.ProgressLocation.Notification,
    title: "Detecting FV-1 programmer...",
    cancellable: false
  }, async () => {
    try {
      const baudRate = Config.getBaudRate();
      const detectedPort = await require('./utils').default.detectProgrammer(baudRate);

      if (detectedPort) {
        await Config.setSerialPort(detectedPort);

        Logs.log(LogType.INFO, `Programmer detected at: ${detectedPort}`);
        vscode.window.showInformationMessage(`Programmer detected and configured: ${detectedPort}`);
      }
      else {
        vscode.window.showWarningMessage(
          "Could not auto-detect programmer. Please select port manually.",
          "Select Port"
        ).then(action => {
          if (action === "Select Port") {
            vscode.commands.executeCommand("spinasm.selectSerialPort");
          }
        });
      }
    }
    catch (error) {
      handleError(error, "Failed to detect programmer");
    }
  });
}

// =============================================================================
// CORE OPERATIONS
// =============================================================================

async function compileBank(bank: number): Promise<void> {
  await runOperation(async (project) => {
    await project.compileProgramToHex(bank);

    Logs.log(LogType.INFO, `Program ${bank} compilation successful`);
    vscode.window.showInformationMessage(`Program ${bank} compiled successfully!`);
  }, "Compilation Failed", `Compiling Bank ${bank}...`);
}

async function uploadBank(bank: number): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  await runOperation(async (project, settings) => {
    await performUpload(project, settings, bank);

    Logs.log(LogType.INFO, `Program ${bank} upload successful`);
    vscode.window.showInformationMessage(`Program ${bank} uploaded successfully!`);
  }, `Failed to upload program ${bank}`, `Uploading Bank ${bank}...`);
}

async function compileAndUploadBank(bank: number): Promise<void> {
  await runOperation(async (project, settings) => {
    await project.compileProgramToHex(bank);
    await performUpload(project, settings, bank);

    Logs.log(LogType.INFO, `Program ${bank} compiled and uploaded successfully`);
    vscode.window.showInformationMessage(`Program ${bank} compiled and uploaded successfully!`);
  }, `Failed to compile and upload program ${bank}`, `Compiling & Uploading Bank ${bank}...`);
}

// =============================================================================
// BULK & CURRENT OPERATIONS
// =============================================================================

async function compileCurrentProgram(): Promise<void> {
  await runOperation(async (project) => {
    const currentProgram = getCurrentBank(project);

    if (currentProgram === -1) {
      throw new Error("Current file is not a valid project program.");
    }

    await project.compileProgramToHex(currentProgram);

    vscode.window.showInformationMessage(`Program ${currentProgram} compiled successfully!`);
  }, "Failed to compile current program", "Compiling current program...");
}

async function uploadCurrentProgram(): Promise<void> {
  await runOperation(async (project, settings) => {
    const currentProgram = getCurrentBank(project);

    if (currentProgram === -1) {
      throw new Error("Current file is not a valid project program.");
    }

    await performUpload(project, settings, currentProgram);
    vscode.window.showInformationMessage(`Program ${currentProgram} uploaded successfully!`);
  }, "Failed to upload current program", "Uploading Current Program...");
}

async function compileAndUploadCurrentProgram(): Promise<void> {
  await runOperation(async (project, settings) => {
    const currentProgram = getCurrentBank(project);

    if (currentProgram === -1) {
      throw new Error("Current file is not a valid project program.");
    }

    await project.compileProgramToHex(currentProgram);
    await performUpload(project, settings, currentProgram);
    vscode.window.showInformationMessage(`Program ${currentProgram} compiled and uploaded successfully!`);
  }, "Failed to compile and upload current program", "Compiling & Uploading Current Program...");
}

async function compileAllPrograms(): Promise<void> {
  await runOperation(async (project, settings) => {
    const programs = project.getAllPrograms();

    for (const programPath of programs) {
      if(!programPath) {
        continue;
      }

      const bank = project.getProgramBankByPath(programPath);
      await project.buildSetup(settings.compilerPath, settings.compilerArgs);
      await project.compileProgramToHex(bank);
    }

    vscode.window.showInformationMessage("All programs compiled successfully!");
  }, "Failed to compile all programs", "Compiling all programs...");
}

async function compileAllProgramsToBin(): Promise<void> {
  await runOperation(async (project, settings) => {
    const programs = project.getAllPrograms();

    for (const programPath of programs) {
      if(!programPath) {
        continue;
      }

      const bank = project.getProgramBankByPath(programPath);
      await project.buildSetup(settings.compilerPath, settings.compilerArgs);
      await project.compileProgramToBin(bank);
    }

    vscode.window.showInformationMessage("All programs compiled to BIN successfully!");
  }, "Failed to compile all programs", "Compiling all programs to BIN...");
}

async function uploadAllPrograms(): Promise<void> {
  await vscode.window.withProgress({
    location: vscode.ProgressLocation.Notification,
    title: "Uploading all programs...",
    cancellable: false
  }, async (progress) => {
    const folder = await getWorkspaceFolder();
    if (!folder) {
      return;
    }

    try {
      const settings = loadSettings();
      const project = new Project(folder);
      await project.buildSetup(settings.compilerPath, settings.compilerArgs);

      const programs = project.getAllPrograms();
      const programsToUpload = programs.filter(p => p !== null);
      const totalPrograms = programsToUpload.length;

      let uploadedCount = 0;

      for (const programPath of programs) {
        if(!programPath) {
          continue;
        }

        const bank = project.getProgramBankByPath(programPath);

        progress.report({
          increment: (100 / totalPrograms),
          message: `Uploading bank ${bank}... (${uploadedCount + 1}/${totalPrograms})`
        });

        await performUpload(project, settings, bank);
        uploadedCount++;

        Logs.log(LogType.INFO, `Bank ${bank} uploaded successfully (${uploadedCount}/${totalPrograms})`);
      }

      vscode.window.showInformationMessage(`All programs uploaded successfully! (${uploadedCount} banks)`);
    }
    catch (error) {
      handleError(error, "Failed to upload all programs");
    }
  });
}

async function compileAndUploadAllPrograms(): Promise<void> {
  await vscode.window.withProgress({
    location: vscode.ProgressLocation.Notification,
    title: "Compiling and uploading all programs...",
    cancellable: false
  }, async (progress) => {
    const folder = await getWorkspaceFolder();
    if (!folder) {
      return;
    }

    try {
      const settings = loadSettings();
      const project = new Project(folder);
      await project.buildSetup(settings.compilerPath, settings.compilerArgs);

      const programs = project.getAllPrograms();
      const programsToProcess = programs.filter(p => p !== null);
      const totalPrograms = programsToProcess.length;

      let processedCount = 0;

      for (const programPath of programs) {
        if(!programPath) {
          continue;
        }

        const bank = project.getProgramBankByPath(programPath);

        progress.report({
          increment: (100 / (totalPrograms * 2)),
          message: `Compiling bank ${bank}... (${processedCount + 1}/${totalPrograms})`
        });

        await project.compileProgramToHex(bank);

        progress.report({
          increment: (100 / (totalPrograms * 2)),
          message: `Uploading bank ${bank}... (${processedCount + 1}/${totalPrograms})`
        });

        await performUpload(project, settings, bank);
        processedCount++;

        Logs.log(LogType.INFO, `Bank ${bank} compiled and uploaded (${processedCount}/${totalPrograms})`);
      }

      vscode.window.showInformationMessage(`All programs compiled and uploaded successfully! (${processedCount} banks)`);
    }
    catch (error) {
      handleError(error, "Failed to compile and upload all programs");
    }
  });
}

// =============================================================================
// PROJECT MANAGEMENT & UTILS
// =============================================================================

async function createProject(): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  try {
    const project = new Project(folder);

    await project.createProjectStructure();

    Logs.log(LogType.INFO, "Project structure created successfully");
    vscode.window.showInformationMessage("Project created successfully!");
  }
  catch (error) {
    handleError(error, "Failed to create project structure");
  }
}

async function checkHardwareConnection(): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  let programmer: Programmer | null = null;

  try {
    const settings = loadSettings();

    // Check compiler
    const project = new Project(folder);

    await project.buildSetup(settings.compilerPath, settings.compilerArgs);
    await project.checkCompiler();

    // Check hardware
    programmer = new Programmer(settings.serialPort, settings.baudRate);

    await programmer.connect();

    if (! (await programmer.isProgrammerConnected())) {
      throw new Error("Programmer did not respond.");
    }
    if (! (await programmer.isEepromReady())) {
      throw new Error("EEPROM is not ready.");
    }

    vscode.window.showInformationMessage("Compiler and Programmer are connected and ready!");
  }
  catch (error) {
    handleError(error, "Hardware check failed");
  }
  finally {
    if (programmer) {
      await programmer.disconnect();
    }
  }
}

async function showConfig(): Promise<void> {
  const port = Config.getSerialPort();
  const baud = Config.getBaudRate();
  const compiler = Config.getCompilerPath();

  Logs.log(LogType.INFO, `Config | Port: ${port} | Baud: ${baud} | Compiler: ${compiler}`);

  const action = await vscode.window.showInformationMessage(
    `Current Configuration:\nCompiler: ${compiler}\nPort: ${port}\nBaud: ${baud}`,
    "Open Settings"
  );

  if (action === "Open Settings") {
    vscode.commands.executeCommand("workbench.action.openSettings", "spinasm");
  }
}

// =============================================================================
// HELPERS
// =============================================================================

async function runOperation(
  operation: (project: Project, settings: ProjectSettings) => Promise<void>,
  errorMessage: string,
  progressTitle: string
): Promise<void> {
  const folder = await getWorkspaceFolder();
  if (!folder) {
    return;
  }

  await vscode.window.withProgress({
    location: vscode.ProgressLocation.Notification,
    title: progressTitle,
    cancellable: false
  }, async (progress) => {
    try {
        const settings = loadSettings();
        const project = new Project(folder);
        await project.buildSetup(settings.compilerPath, settings.compilerArgs);
        await operation(project, settings);
    }
    catch (error) {
        handleError(error, errorMessage);
    }
  });
}

async function performUpload(project: Project, settings: ProjectSettings, bank: number): Promise<void> {
  let programmer: Programmer | null = null;

  try {
    programmer = new Programmer(settings.serialPort, settings.baudRate);

    await programmer.connect();

    if (! (await programmer.isProgrammerConnected())) {
      throw new Error("Programmer did not respond.");
    }

    const hexOutput = project.getOutput(bank);

    if (!hexOutput) {
      throw new Error(`No output file found for bank ${bank}`);
    }

    const program = await programmer.readIntelHexData(hexOutput);

    await programmer.writeProgram(program.address, program.data);
    const programRead = await programmer.readProgram(program.address);

    if (Buffer.compare(program.data, programRead) !== 0) {
      throw new Error("Data verification failed.");
    }
  }
  finally {
    if (programmer) {
      await programmer.disconnect();
    }
  }
}

function getCurrentBank(project: Project): number {
  return project.getProgramBankByPath(vscode.window.activeTextEditor?.document.uri.fsPath);
}

interface ProjectSettings {
  compilerPath: string;
  compilerArgs: string[];
  serialPort: string;
  baudRate: number;
}

function loadSettings(): ProjectSettings {
  const compilerPath = Config.getCompilerPath();
  const serialPort = Config.getSerialPort();

  if (!compilerPath) {
    throw new Error("Compiler path is not set in Settings.");
  }

  if (!serialPort) {
    throw new Error("Serial port is not set in Settings.");
  }

  return {
    compilerPath,
    compilerArgs: Config.getCompilerArgs(),
    serialPort,
    baudRate: Config.getBaudRate(),
  };
}

function handleError(error: unknown, message: string): void {
  const errorMessage = (error as Error).message;

  Logs.log(LogType.ERROR, `${message}: ${errorMessage}`);
  vscode.window.showErrorMessage(`${message}: ${errorMessage}`);

  Logs.show();
}

async function getWorkspaceFolder(): Promise<string | null> {
  const folders = vscode.workspace.workspaceFolders;

  if (!folders || folders.length === 0) {
    vscode.window.showErrorMessage("No workspace folder open.");
    return null;
  }

  if (folders.length === 1) {
    return folders[0].uri.fsPath;
  }

  const selectedFolder = await vscode.window.showQuickPick(
    folders.map((folder) => folder.uri.fsPath),
    { placeHolder: "Select a workspace folder" }
  );

  return selectedFolder || null;
}