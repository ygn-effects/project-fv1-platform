import * as vscode from "vscode";
import * as path from "path";
import type Project from "./project";
import type UtilsType from "./utils";
import type { SerialPortInfo } from "./utils";
import type ProgrammerType from "./programmer";
import Config from "./config";
import Logs, { LogType } from "./logs";
import { SpinASMSemanticTokensProvider, SpinASMHoverProvider } from "./spinasmSemanticTokens";
import { initializeBankStatusBar, disposeBankStatusBar, showBankStatus } from "./statusBar";
import { initializeResourceStatusBar, disposeResourceStatusBar, showResourceUsage } from "./resourceStatusBar";
import { SpinASMValidator } from "./spinasmValidator";
import { ProjectManager } from "./projectManager";
import { BANK_COUNT } from "./fv1Constants";

let validator: SpinASMValidator;

// Lazy-loaded serialport-dependent modules; loading them eagerly costs hundreds of
// ms because @serialport/bindings-cpp is a native module. Commands that need them
// import on first use via these getters.

async function getUtils(): Promise<typeof UtilsType> {
  const mod = await import("./utils.js");
  return mod.default as unknown as typeof UtilsType;
}

async function getProgrammer(): Promise<typeof ProgrammerType> {
  const mod = await import("./programmer.js");
  return mod.default as unknown as typeof ProgrammerType;
}

async function requireProject(folder: string): Promise<Project> {
  const project = await ProjectManager.getInstance().getProject(folder);
  if (!project) {
    throw new Error("Compiler path is not set in Settings.");
  }
  return project;
}

export function activate(context: vscode.ExtensionContext): void {
  Logs.createChannel();
  Logs.log(LogType.INFO, "Extension activating...");

  validator = new SpinASMValidator();
  context.subscriptions.push(validator);

  // Don't await — activate() must return fast. getBanksSync handles cache miss
  // by returning a placeholder while a background refresh runs.
  const projectManager = ProjectManager.getInstance();
  projectManager.initializeWorkspace();

  // Drop cached projects on compiler config change so the next command picks
  // up the new path/args.
  context.subscriptions.push(
    vscode.workspace.onDidChangeConfiguration((e) => {
      if (e.affectsConfiguration('spinasm.compiler')) {
        projectManager.invalidateAll();
        projectManager.initializeWorkspace();
      }
    })
  );

  let validationTimer: NodeJS.Timeout | null = null;
  function scheduleValidation(doc: vscode.TextDocument): void {
    if (validationTimer) {
      clearTimeout(validationTimer);
    }
    validationTimer = setTimeout(() => {
      validator.validateDocument(doc);
    }, 500);
  }

  context.subscriptions.push(
    vscode.workspace.onDidOpenTextDocument((doc) => {
      if (doc.languageId === 'spinasm') {
        scheduleValidation(doc);
      }
    })
  );

  context.subscriptions.push(
    vscode.workspace.onDidChangeTextDocument((event) => {
      if (event.document.languageId === 'spinasm') {
        scheduleValidation(event.document);
      }
    })
  );

  // Save runs validation immediately and refreshes the saved bank's cache.
  context.subscriptions.push(
    vscode.workspace.onDidSaveTextDocument((doc) => {
      if (doc.languageId === 'spinasm') {
        if (validationTimer) { clearTimeout(validationTimer); }
        validator.validateDocument(doc);

        const rootPath = vscode.workspace.getWorkspaceFolder(doc.uri)?.uri.fsPath;
        if (rootPath) {
          const bankIndex = projectManager.getBankIndexFromPath(doc.uri.fsPath);
          if (bankIndex !== -1) {
            projectManager.refreshBank(rootPath, bankIndex);
          }
        }
      }
    })
  );

  context.subscriptions.push(
    vscode.workspace.onDidCloseTextDocument((doc) => {
      if (doc.languageId === 'spinasm') {
        validator.clearDocument(doc);
      }
    })
  );

  initializeBankStatusBar(context);
  initializeResourceStatusBar(context);

  const watcher = vscode.workspace.createFileSystemWatcher("**/*.spn");
  context.subscriptions.push(watcher);

  watcher.onDidChange(async (uri) => {
    const folder = vscode.workspace.getWorkspaceFolder(uri)?.uri.fsPath;
    if (!folder) { return; }

    const bankIndex = projectManager.getBankIndexFromPath(uri.fsPath);
    if (bankIndex !== -1) {
      await projectManager.refreshBank(folder, bankIndex);
    }

    if (Config.getCompileOnSave()) {
      await handleCompileOnSave(uri);
    }
  });

  // Create/delete change which file lives in each bank, so we have to
  // refreshProject (which rescans), not refreshBank.
  watcher.onDidCreate(async (uri) => {
    const folder = vscode.workspace.getWorkspaceFolder(uri)?.uri.fsPath;
    if (!folder) { return; }
    await projectManager.refreshProject(folder);
  });

  watcher.onDidDelete(async (uri) => {
    const folder = vscode.workspace.getWorkspaceFolder(uri)?.uri.fsPath;
    if (!folder) { return; }
    await projectManager.refreshProject(folder);
  });

  // Watch the output dir so external compiler runs (e.g. asfv1 from a terminal)
  // update the bank status bar too.
  const hexWatcher = vscode.workspace.createFileSystemWatcher("**/output/*.hex");
  context.subscriptions.push(hexWatcher);

  const onHexUpdate = async (uri: vscode.Uri) => {
    const folder = vscode.workspace.getWorkspaceFolder(uri)?.uri.fsPath;
    if (!folder) { return; }

    const baseName = path.basename(uri.fsPath, ".hex");
    const cachedBanks = projectManager.getBanksSync(folder);
    for (let i = 0; i < BANK_COUNT; i++) {
      if (cachedBanks[i].spnFile && path.basename(cachedBanks[i].spnFile!, ".spn") === baseName) {
        await projectManager.refreshBank(folder, i);
        break;
      }
    }
  };

  hexWatcher.onDidChange(onHexUpdate);
  hexWatcher.onDidCreate(onHexUpdate);
  hexWatcher.onDidDelete(onHexUpdate);

  context.subscriptions.push(
    vscode.languages.registerDocumentSemanticTokensProvider(
      { language: 'spinasm' },
      new SpinASMSemanticTokensProvider(),
      SpinASMSemanticTokensProvider.legend
    )
  );

  context.subscriptions.push(
    vscode.languages.registerHoverProvider(
      { language: 'spinasm' },
      new SpinASMHoverProvider()
    )
  );

  Logs.log(LogType.INFO, "SpinASM semantic highlighting enabled");

  context.subscriptions.push(
    vscode.commands.registerCommand("spinasm.createProject", createProject),
    vscode.commands.registerCommand("spinasm.checkProjectSettings", checkHardwareConnection),
    vscode.commands.registerCommand("spinasm.showSerialConfig", showConfig),
    vscode.commands.registerCommand("spinasm.showBankStatus", showBankStatus),
    vscode.commands.registerCommand("spinasm.showResourceUsage", showResourceUsage),

    vscode.commands.registerCommand("spinasm.selectSerialPort", selectSerialPort),
    vscode.commands.registerCommand("spinasm.autoDetectProgrammer", autoDetectProgrammer),

    vscode.commands.registerCommand("spinasm.compileCurrentProgram", compileCurrentProgram),
    vscode.commands.registerCommand("spinasm.uploadCurrentProgram", uploadCurrentProgram),
    vscode.commands.registerCommand("spinasm.compileAndUploadCurrentProgram", compileAndUploadCurrentProgram),

    vscode.commands.registerCommand("spinasm.compileAllPrograms", compileAllPrograms),
    vscode.commands.registerCommand("spinasm.compileAllProgramsToBin", compileAllProgramsToBin),
    vscode.commands.registerCommand("spinasm.uploadAllPrograms", uploadAllPrograms),
    vscode.commands.registerCommand("spinasm.compileAndUploadAllPrograms", compileAndUploadAllPrograms),

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

  if (Config.isConfigMissing()) {
    vscode.window.showWarningMessage("SpinASM: Compiler path or Serial port is not configured. Please check your Settings.");
  }

  Logs.log(LogType.INFO, "Commands registered successfully");
}

export function deactivate(): void {
  Logs.disposeChannel();
  disposeBankStatusBar();
  disposeResourceStatusBar();
}

async function handleCompileOnSave(uri: vscode.Uri): Promise<void> {
  const folder = vscode.workspace.getWorkspaceFolder(uri)?.uri.fsPath;

  if (!folder) {
    return;
  }

  try {
    const project = await ProjectManager.getInstance().getProject(folder);
    if (!project) {
      return;
    }

    const bank = project.getProgramBankByPath(uri.fsPath);

    if (bank === -1) {
      return;
    }

    Logs.log(LogType.INFO, `Compile-on-save: Compiling bank ${bank}...`);
    await project.compileProgramToHex(bank);
    Logs.log(LogType.INFO, `Compile-on-save: Bank ${bank} compiled successfully`);

  } catch (error) {
    Logs.log(LogType.ERROR, `Compile-on-save failed: ${(error as Error).message}`);
  }
}

async function pickBank(): Promise<number | undefined> {
  const items = [];

  for (let i = 0; i < BANK_COUNT; i++) {
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

async function selectSerialPort(): Promise<void> {
  try {
    const Utils = await getUtils();
    const ports = await Utils.listSerialPorts();

    if (ports.length === 0) {
      vscode.window.showErrorMessage("No serial ports detected on this system.");
      return;
    }

    const items = ports.map((port: SerialPortInfo) => ({
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

async function autoDetectProgrammer(): Promise<void> {
  await vscode.window.withProgress({
    location: vscode.ProgressLocation.Notification,
    title: "Detecting FV-1 programmer...",
    cancellable: false
  }, async () => {
    try {
      const Utils = await getUtils();
      const baudRate = Config.getBaudRate();
      const detectedPort = await Utils.detectProgrammer(baudRate);

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
  await runOperation(async (project) => {
    const programs = project.getAllPrograms();

    for (const programPath of programs) {
      if(!programPath) {
        continue;
      }

      const bank = project.getProgramBankByPath(programPath);
      await project.compileProgramToHex(bank);
    }

    vscode.window.showInformationMessage("All programs compiled successfully!");
  }, "Failed to compile all programs", "Compiling all programs...");
}

async function compileAllProgramsToBin(): Promise<void> {
  await runOperation(async (project) => {
    await project.compileAllProgramsToCombinedBin();
    vscode.window.showInformationMessage("Combined EEPROM image written to output.bin!");
  }, "Failed to compile combined EEPROM image", "Building combined EEPROM image...");
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
      const project = await requireProject(folder);

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
      const project = await requireProject(folder);

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

async function createProject(): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  try {
    // First-time setup: compiler may not be configured yet, so we bypass the
    // manager and use a throwaway Project just for file creation. The manager
    // picks up the new files via the create watcher.
    const mod = await import("./project.js");
    const ProjectCtor = mod.default as unknown as typeof import("./project").default;
    const project = new ProjectCtor(folder);
    await project.createProjectStructure();
    project.dispose();

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

  const Programmer = await getProgrammer();
  let programmer: ProgrammerType | null = null;

  try {
    const settings = loadSettings();

    const project = await requireProject(folder);
    await project.checkCompiler();

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
  }, async () => {
    try {
        const settings = loadSettings();
        const project = await requireProject(folder);
        await operation(project, settings);
    }
    catch (error) {
        handleError(error, errorMessage);
    }
  });
}

async function performUpload(project: Project, settings: ProjectSettings, bank: number): Promise<void> {
  const Programmer = await getProgrammer();
  let programmer: ProgrammerType | null = null;

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

/** Background callers should pass `{ showLog: false }` to avoid stealing focus. */
function handleError(error: unknown, message: string, options: { showLog?: boolean } = {}): void {
  const { showLog = true } = options;
  const errorMessage = (error as Error).message;

  Logs.log(LogType.ERROR, `${message}: ${errorMessage}`);
  vscode.window.showErrorMessage(`${message}: ${errorMessage}`);

  if (showLog) {
    Logs.show();
  }
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
