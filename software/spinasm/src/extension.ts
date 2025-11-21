import * as vscode from "vscode";
import Project from "./project";
import Config from "./config";
import Logs, { LogType } from "./logs";
import Programmer from "./programmer";

export function activate(context: vscode.ExtensionContext): void {
  Logs.createChannel();
  Logs.log(LogType.INFO, "Extension activated");

  context.subscriptions.push(
    // Global / Project Management
    vscode.commands.registerCommand("spinasm.createProject", createProject),
    vscode.commands.registerCommand("spinasm.checkProjectSettings", checkHardwareConnection),
    vscode.commands.registerCommand("spinasm.showSerialConfig", showConfig),

    // Current File Operations
    vscode.commands.registerCommand("spinasm.compileCurrentProgram", compileCurrentProgram),
    vscode.commands.registerCommand("spinasm.uploadCurrentProgram", uploadCurrentProgram),
    vscode.commands.registerCommand("spinasm.compileAndUploadCurrentProgram", compileAndUploadCurrentProgram),

    // Batch Operations
    vscode.commands.registerCommand("spinasm.compileAllPrograms", compileAllPrograms),
    vscode.commands.registerCommand("spinasm.compileAllProgramsToBin", compileAllProgramsToBin),

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
// CORE OPERATIONS
// =============================================================================

async function compileBank(bank: number): Promise<void> {
  await runOperation(async (project) => {
    project.compileProgramToHex(bank);

    Logs.log(LogType.INFO, `Program ${bank} compilation successful`);
    vscode.window.showInformationMessage(`Program ${bank} compiled successfully!`);
  }, "Compilation Failed");
}

async function uploadBank(bank: number): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  try {
    const settings = loadSettings();
    const project = new Project(folder);

    project.buildSetup(settings.compilerPath, settings.compilerArgs);
    await performUpload(project, settings, bank);

    Logs.log(LogType.INFO, `Program ${bank} upload successful`);
    vscode.window.showInformationMessage(`Program ${bank} uploaded successfully!`);
  }
  catch (error) {
    handleError(error, `Failed to upload program ${bank}`);
  }
}

async function compileAndUploadBank(bank: number): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  try {
    const settings = loadSettings();
    const project = new Project(folder);

    project.buildSetup(settings.compilerPath, settings.compilerArgs);
    project.compileProgramToHex(bank);
    await performUpload(project, settings, bank);

    Logs.log(LogType.INFO, `Program ${bank} compiled and uploaded successfully`);
    vscode.window.showInformationMessage(`Program ${bank} compiled and uploaded successfully!`);
  }
  catch (error) {
    handleError(error, `Failed to compile and upload program ${bank}`);
  }
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

    project.compileProgramToHex(currentProgram);

    vscode.window.showInformationMessage(`Program ${currentProgram} compiled successfully!`);
  }, "Failed to compile current program");
}

async function uploadCurrentProgram(): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  try {
    const settings = loadSettings();
    const project = new Project(folder);

    project.buildSetup(settings.compilerPath, settings.compilerArgs);
    const currentProgram = getCurrentBank(project);

    if (currentProgram === -1) {
      throw new Error("Current file is not a valid project program.");
    }

    await performUpload(project, settings, currentProgram);

    vscode.window.showInformationMessage(`Program ${currentProgram} uploaded successfully!`);
  }
  catch (error) {
    handleError(error, "Failed to upload current program");
  }
}

async function compileAndUploadCurrentProgram(): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  try {
    const settings = loadSettings();
    const project = new Project(folder);

    project.buildSetup(settings.compilerPath, settings.compilerArgs);
    const currentProgram = getCurrentBank(project);

    if (currentProgram === -1) {
      throw new Error("Current file is not a valid project program.");
    }

    project.compileProgramToHex(currentProgram);
    await performUpload(project, settings, currentProgram);

    vscode.window.showInformationMessage(`Program ${currentProgram} compiled and uploaded successfully!`);
  }
  catch (error) {
    handleError(error, "Failed to compile and upload current program");
  }
}

async function compileAllPrograms(): Promise<void> {
  await runOperation(async (project, settings) => {
    const programs = project.getAllPrograms();

    for (const programPath of programs) {
      if(!programPath) {
        continue;
      }

      const bank = project.getProgramBankByPath(programPath);
      project.buildSetup(settings.compilerPath, settings.compilerArgs);
      project.compileProgramToHex(bank);
    }

    vscode.window.showInformationMessage("All programs compiled successfully!");
  }, "Failed to compile all programs");
}

async function compileAllProgramsToBin(): Promise<void> {
  await runOperation(async (project, settings) => {
    const programs = project.getAllPrograms();

    for (const programPath of programs) {
      if(!programPath) {
        continue;
      }

      const bank = project.getProgramBankByPath(programPath);
      project.buildSetup(settings.compilerPath, settings.compilerArgs);
      project.compileProgramToBin(bank);
    }

    vscode.window.showInformationMessage("All programs compiled to BIN successfully!");
  }, "Failed to compile all programs");
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

    project.createProjectStructure();

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

    project.buildSetup(settings.compilerPath, settings.compilerArgs);
    project.checkCompiler();

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
  errorMessage: string
): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  try {
    const settings = loadSettings();
    const project = new Project(folder);

    project.buildSetup(settings.compilerPath, settings.compilerArgs);
    await operation(project, settings);
  }
  catch (error) {
    handleError(error, errorMessage);
  }
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

    const program = programmer.readIntelHexData(hexOutput);

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
