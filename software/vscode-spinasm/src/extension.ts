import * as vscode from "vscode";
import type Project from "./project";
import type UtilsType from "./utils";
import type { SerialPortInfo } from "./utils";
import type ProgrammerType from "./programmer";
import Config from "./config";
import Logs, { LogType } from "./logs";
import { SpinASMSemanticTokensProvider, SpinASMHoverProvider } from "./spinasmSemanticTokens";
import { SpinASMDefinitionProvider, SpinASMCompletionProvider } from "./spinasmLanguageProviders";
import { initializeBankStatusBar, showBankStatus } from "./statusBar";
import { initializeResourceStatusBar, showResourceUsage } from "./resourceStatusBar";
import { SpinASMValidator } from "./spinasmValidator";
import { DocumentParser } from "./documentParser";
import { ProjectManager } from "./projectManager";
import { CompilerDiagnostics } from "./compilerDiagnostics";
import { BANK_COUNT } from "./fv1Constants";
import { findDirtyProgramPaths } from "./dirtyProgramGuard";
import { validateIntelHexForBank } from "./intelHex";
import { readIntelHexData } from "./intelHexFile";
import { writeAndVerifyBankProgram } from "./programUpload";
import { OperationQueue } from "./operationQueue";
import { bankFolderName, bankOfOutputFile, bankOfSourceFile } from "./bankLayout";
import { describeStaleOutputs, OutputState } from "./staleOutputGuard";

let validator: SpinASMValidator;

// One queue for the whole extension, not one per workspace folder: the
// programmer's serial port is shared by every folder.
const operations = new OperationQueue();

/**
 * Runs `task` once every earlier compile, upload or hardware operation has
 * finished. With `progress`, the notification says it is waiting meanwhile.
 */
async function runExclusive<T>(
  task: () => Promise<T>,
  progress?: vscode.Progress<{ message?: string }>
): Promise<T> {
  const waiting = progress !== undefined && operations.isBusy;
  if (waiting) {
    progress.report({ message: "Waiting for the previous operation to finish..." });
  }

  return operations.run(async () => {
    if (waiting) {
      progress.report({ message: "" });
    }
    return task();
  });
}

// Lazy-loaded serialport-dependent modules; loading them eagerly costs hundreds of
// ms because @serialport/bindings-cpp is a native module. Commands that need them
// require() on first use via these getters.

function getUtils(): typeof UtilsType {
  return require("./utils.js").default;
}

function getProgrammer(): typeof ProgrammerType {
  return require("./programmer.js").default;
}

export function activate(context: vscode.ExtensionContext): void {
  Logs.createChannel();
  Logs.log(LogType.INFO, "Extension activating...");

  validator = new SpinASMValidator();
  context.subscriptions.push(validator);

  // Surfaces asfv1's own errors/warnings as inline diagnostics, in a separate
  // collection from the live validator so the two don't clobber each other.
  const compilerDiagnostics = new CompilerDiagnostics();
  context.subscriptions.push(compilerDiagnostics);

  // Don't await — activate() must return fast. getBanksSync handles cache miss
  // by returning a placeholder while a background refresh runs.
  const projectManager = ProjectManager.getInstance();
  projectManager.initializeWorkspace();

  context.subscriptions.push(
    projectManager.onDidProduceCompilerOutput(({ sourcePath, stderr }) => {
      compilerDiagnostics.report(vscode.Uri.file(sourcePath), stderr);
    })
  );

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

  // One timer per document — a single shared timer would let edits in one
  // file cancel another file's pending validation, leaving stale diagnostics.
  const validationTimers = new Map<string, NodeJS.Timeout>();

  function cancelValidation(doc: vscode.TextDocument): void {
    const key = doc.uri.toString();
    const timer = validationTimers.get(key);
    if (timer) {
      clearTimeout(timer);
      validationTimers.delete(key);
    }
  }

  function scheduleValidation(doc: vscode.TextDocument): void {
    cancelValidation(doc);
    validationTimers.set(doc.uri.toString(), setTimeout(() => {
      validationTimers.delete(doc.uri.toString());
      validator.validateDocument(doc);
    }, 500));
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
        // The last compile's diagnostics no longer line up with the edited text.
        compilerDiagnostics.clear(event.document.uri);
      }
    })
  );

  // On save the content is final — skip the debounce so diagnostics and the
  // bank cache don't lag half a second behind the file on disk.
  context.subscriptions.push(
    vscode.workspace.onDidSaveTextDocument((doc) => {
      if (doc.languageId === 'spinasm') {
        cancelValidation(doc);
        validator.validateDocument(doc);

        const rootPath = vscode.workspace.getWorkspaceFolder(doc.uri)?.uri.fsPath;
        if (rootPath) {
          const bankIndex = bankOfSourceFile(rootPath, doc.uri.fsPath);
          if (bankIndex !== -1) {
            projectManager.refreshBank(rootPath, bankIndex);
          }
        }

        // Driven by the editor save, not the file watcher, so changes made
        // outside VS Code (git pull, branch switch) don't trigger compiles.
        if (Config.getCompileOnSave()) {
          handleCompileOnSave(doc.uri);
        }
      }
    })
  );

  context.subscriptions.push(
    vscode.workspace.onDidCloseTextDocument((doc) => {
      if (doc.languageId === 'spinasm') {
        // Cancel any pending validation so it can't fire after the close and
        // re-add diagnostics to a document we just cleared.
        cancelValidation(doc);
        validator.clearDocument(doc);
        // The parser cache is keyed by URI and otherwise lives for the whole
        // session. Document versions also reset on reopen, so a stale entry
        // could match a reopened file whose content changed on disk meanwhile.
        DocumentParser.invalidate(doc);
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

    const bankIndex = bankOfSourceFile(folder, uri.fsPath);
    if (bankIndex !== -1) {
      await projectManager.refreshBank(folder, bankIndex);
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

    const bankIndex = bankOfOutputFile(folder, uri.fsPath);
    if (bankIndex !== -1) {
      await projectManager.refreshBank(folder, bankIndex);
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

  context.subscriptions.push(
    vscode.languages.registerDefinitionProvider(
      { language: 'spinasm' },
      new SpinASMDefinitionProvider()
    )
  );

  context.subscriptions.push(
    vscode.languages.registerCompletionItemProvider(
      { language: 'spinasm' },
      new SpinASMCompletionProvider()
    )
  );

  Logs.log(LogType.INFO, "SpinASM language features enabled (highlighting, hover, definition, completion)");

  context.subscriptions.push(
    vscode.commands.registerCommand("spinasm.createProject", createProject),
    vscode.commands.registerCommand("spinasm.checkProjectSettings", checkHardwareConnection),
    vscode.commands.registerCommand("spinasm.showSerialConfig", showConfig),
    vscode.commands.registerCommand("spinasm.showBankStatus", showBankStatus),
    vscode.commands.registerCommand("spinasm.showResourceUsage", showResourceUsage),

    vscode.commands.registerCommand("spinasm.selectSerialPort", selectSerialPort),
    vscode.commands.registerCommand("spinasm.autoDetectProgrammer", autoDetectProgrammer),

    vscode.commands.registerCommand("spinasm.compileCurrentProgram",
      (uri?: vscode.Uri) => runBankCommand({ kind: "current", uri }, "compile")),
    vscode.commands.registerCommand("spinasm.uploadCurrentProgram",
      (uri?: vscode.Uri) => runBankCommand({ kind: "current", uri }, "upload")),
    vscode.commands.registerCommand("spinasm.compileAndUploadCurrentProgram",
      (uri?: vscode.Uri) => runBankCommand({ kind: "current", uri }, "compileAndUpload")),

    vscode.commands.registerCommand("spinasm.compileAllPrograms", () => runBankCommand({ kind: "all" }, "compile")),
    vscode.commands.registerCommand("spinasm.compileAllProgramsToBin", compileAllProgramsToBin),
    vscode.commands.registerCommand("spinasm.uploadAllPrograms", () => runBankCommand({ kind: "all" }, "upload")),
    vscode.commands.registerCommand("spinasm.compileAndUploadAllPrograms",
      () => runBankCommand({ kind: "all" }, "compileAndUpload")),

    vscode.commands.registerCommand("spinasm.compileBank", runOnPickedBank("compile")),
    vscode.commands.registerCommand("spinasm.uploadBank", runOnPickedBank("upload")),
    vscode.commands.registerCommand("spinasm.compileAndUploadBank", runOnPickedBank("compileAndUpload"))
  );

  if (Config.isCompilerMissing()) {
    vscode.window.showWarningMessage("SpinASM: Compiler path is not configured. Please check your Settings.");
  }

  Logs.log(LogType.INFO, "Commands registered successfully");
}

// The status bar items are disposed through context.subscriptions.
export function deactivate(): void {
  Logs.disposeChannel();
}

async function handleCompileOnSave(uri: vscode.Uri): Promise<void> {
  const folder = vscode.workspace.getWorkspaceFolder(uri)?.uri.fsPath;

  if (!folder) {
    return;
  }

  try {
    await runExclusive(async () => {
      const project = await ProjectManager.getInstance().getProject(folder);
      const bank = project.getProgramBankByPath(uri.fsPath);

      if (bank === -1) {
        return;
      }

      Logs.log(LogType.INFO, `Compile-on-save: Compiling bank ${bank}...`);
      await project.compileProgramToHex(bank);
      Logs.log(LogType.INFO, `Compile-on-save: Bank ${bank} compiled successfully`);
    });
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
    const Utils = getUtils();
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
  }, async (progress) => {
    try {
      const Utils = getUtils();
      const baudRate = Config.getBaudRate();
      // Probing opens every serial port, including the one an upload may be using.
      const detectedPort = await runExclusive(() => Utils.detectProgrammer(baudRate), progress);

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

/** Which banks a compile / upload command acts on. */
type BankTarget =
  | { kind: "current"; uri?: vscode.Uri }
  | { kind: "bank"; bank: number }
  | { kind: "all" };

type BankAction = "compile" | "upload" | "compileAndUpload";

const ACTION_WORDING: Record<BankAction, { progress: string; failure: string; success: string }> = {
  compile: { progress: "Compiling", failure: "compile", success: "compiled" },
  upload: { progress: "Uploading", failure: "upload", success: "uploaded" },
  compileAndUpload: { progress: "Compiling & uploading", failure: "compile and upload", success: "compiled and uploaded" },
};

/**
 * Runs a compile, upload or compile & upload command on the current program,
 * one bank or every populated bank. Each bank is compiled (when asked), then
 * uploaded (when asked), before moving to the next.
 */
async function runBankCommand(target: BankTarget, action: BankAction): Promise<void> {
  const compile = action !== "upload";
  const upload = action !== "compile";
  const wording = ACTION_WORDING[action];
  const subject = describeBankTarget(target);

  await runOperation(async (project, settings, progress) => {
    const banks = resolveBanks(project, target);
    if (banks.length === 0) {
      showNoProgramsWarning();
      return;
    }

    if (compile) {
      requireSavedPrograms(project, banks);
    }
    else if (!(await prepareUpload(project, banks))) {
      return;
    }

    const stepsPerBank = (compile ? 1 : 0) + (upload ? 1 : 0);
    const increment = 100 / (banks.length * stepsPerBank);

    for (const [index, bank] of banks.entries()) {
      const position = banks.length > 1 ? ` (${index + 1}/${banks.length})` : "";

      if (compile) {
        progress.report({ increment, message: `Compiling bank ${bank}...${position}` });
        await project.compileProgramToHex(bank);
      }
      if (upload) {
        progress.report({ increment, message: `Uploading bank ${bank}...${position}` });
        await performUpload(project, settings, bank);
      }

      Logs.log(LogType.INFO, `Bank ${bank} ${wording.success}${position}`);
    }

    vscode.window.showInformationMessage(
      target.kind === "all"
        ? `All programs ${wording.success} successfully! (${banks.length} banks)`
        : `Program ${banks[0]} ${wording.success} successfully!`
    );
  }, `Failed to ${wording.failure} ${subject}`, `${wording.progress} ${subject}...`, {
    requireProgrammer: upload,
    target: target.kind === "current" ? getCurrentProgramUri(target.uri) : undefined,
  });
}

function describeBankTarget(target: BankTarget): string {
  switch (target.kind) {
    case "current":
      return "current program";
    case "bank":
      return `bank ${target.bank}`;
    case "all":
      return "all programs";
  }
}

/** Bank indexes a target covers; empty only for "all" in a project with no programs. */
function resolveBanks(project: Project, target: BankTarget): number[] {
  switch (target.kind) {
    case "current": {
      const bank = getCurrentBank(project, target.uri);
      if (bank === -1) {
        throw new Error("Current file is not a valid project program.");
      }
      return [bank];
    }
    case "bank":
      if (!project.getAllPrograms()[target.bank]) {
        throw new Error(`Bank ${target.bank} has no program.`);
      }
      return [target.bank];
    case "all":
      return project.getAllPrograms().flatMap((programPath, bank) => programPath ? [bank] : []);
  }
}

/** Command handler that asks for a bank, then runs `action` on it. */
function runOnPickedBank(action: BankAction): () => Promise<void> {
  return async () => {
    const bank = await pickBank();
    if (bank !== undefined) {
      await runBankCommand({ kind: "bank", bank }, action);
    }
  };
}

async function compileAllProgramsToBin(): Promise<void> {
  await runOperation(async (project) => {
    requireSavedPrograms(project);
    await project.compileAllProgramsToCombinedBin();
    vscode.window.showInformationMessage("Combined EEPROM image written to output.bin!");
  }, "Failed to compile combined EEPROM image", "Building combined EEPROM image...");
}

async function createProject(): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  try {
    // Works before the compiler is configured; the manager rescans on the
    // structure-change event.
    const project = await ProjectManager.getInstance().getProject(folder);
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

  const Programmer = getProgrammer();

  await runExclusive(async () => {
    let programmer: ProgrammerType | null = null;

    try {
      const settings = loadSettings({ requireProgrammer: true });

      const project = await ProjectManager.getInstance().getProject(folder);
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
  });
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

type OperationProgress = vscode.Progress<{ message?: string; increment?: number }>;

async function runOperation(
  operation: (project: Project, settings: ProjectSettings, progress: OperationProgress) => Promise<void>,
  errorMessage: string,
  progressTitle: string,
  options: { requireProgrammer?: boolean; target?: vscode.Uri } = {}
): Promise<void> {
  const folder = await getWorkspaceFolder(options.target);
  if (!folder) {
    return;
  }

  await vscode.window.withProgress({
    location: vscode.ProgressLocation.Notification,
    title: progressTitle,
    cancellable: false
  }, async (progress) => {
    try {
      await runExclusive(async () => {
        const settings = loadSettings(options);
        const project = await ProjectManager.getInstance().getProject(folder);
        await operation(project, settings, progress);
      }, progress);
    }
    catch (error) {
        handleError(error, errorMessage);
    }
  });
}

const COMPILE_AND_UPLOAD = "Compile & Upload";
const UPLOAD_ANYWAY = "Upload Anyway";

/**
 * Upload-only commands send the last compiled output. Checks that each bank's
 * output exists and isn't older than its source; if not, asks whether to
 * compile those banks first or upload the old output as is. Returns false when
 * the user cancels. Runs inside the operation queue, so a compile that was
 * already queued has finished before the outputs are compared.
 */
async function prepareUpload(project: Project, banks: readonly number[]): Promise<boolean> {
  const states = new Map<number, OutputState>();

  for (const bank of banks) {
    states.set(bank, await project.getOutputState(bank));
  }

  const prompt = describeStaleOutputs(states);
  if (!prompt) {
    return true;
  }

  const actions = prompt.allowUploadAnyway ? [COMPILE_AND_UPLOAD, UPLOAD_ANYWAY] : [COMPILE_AND_UPLOAD];
  const choice = await vscode.window.showWarningMessage(
    prompt.message,
    { modal: true, detail: prompt.detail },
    ...actions
  );

  if (choice === COMPILE_AND_UPLOAD) {
    const staleBanks = banks.filter(bank => states.get(bank) !== "current");
    requireSavedPrograms(project, staleBanks);

    for (const bank of staleBanks) {
      await project.compileProgramToHex(bank);
    }
    return true;
  }

  return choice === UPLOAD_ANYWAY;
}

function showNoProgramsWarning(): void {
  vscode.window.showWarningMessage(
    `No programs found. Add a .spn file to one of the ${bankFolderName(0)} to ${bankFolderName(BANK_COUNT - 1)} folders.`
  );
}

async function performUpload(project: Project, settings: ProjectSettings, bank: number): Promise<void> {
  const hexOutput = project.getOutput(bank);
  if (!hexOutput) {
    throw new Error(`No output file found for bank ${bank}`);
  }

  // Parse and validate before loading the native serial module or opening the
  // programmer. Invalid compiler output must never reach the transport.
  const program = await readIntelHexData(hexOutput);
  validateIntelHexForBank(program, bank);

  const Programmer = getProgrammer();
  let programmer: ProgrammerType | null = null;

  try {
    programmer = new Programmer(settings.serialPort, settings.baudRate);

    await programmer.connect();

    if (! (await programmer.isProgrammerConnected())) {
      throw new Error("Programmer did not respond.");
    }
    if (! (await programmer.isEepromReady())) {
      throw new Error("EEPROM isn't responding.");
    }

    await writeAndVerifyBankProgram(programmer, bank, program);
  }
  finally {
    if (programmer) {
      await programmer.disconnect();
    }
  }
}

function getCurrentBank(project: Project, uri?: vscode.Uri): number {
  return project.getProgramBankByPath(getCurrentProgramUri(uri)?.fsPath);
}

function getCurrentProgramUri(uri?: vscode.Uri): vscode.Uri | undefined {
  // Menu invocations (editor title/context, explorer tree) pass the target
  // file's URI; palette and keybinding invocations pass nothing and fall back
  // to the active editor. Passing the URI also makes split views correct — the
  // button acts on its own editor, not whichever happens to be focused.
  return uri ?? vscode.window.activeTextEditor?.document.uri;
}

function requireSavedPrograms(project: Project, banks?: readonly number[]): void {
  const programs = project.getAllPrograms();
  const relevantPrograms = banks
    ? banks.map(bank => programs[bank] ?? null)
    : programs;
  const openDocuments = vscode.workspace.textDocuments.map(document => ({
    fsPath: document.uri.fsPath,
    isDirty: document.isDirty,
  }));
  const dirtyPrograms = findDirtyProgramPaths(relevantPrograms, openDocuments);

  if (dirtyPrograms.length === 0) {
    return;
  }

  const names = dirtyPrograms.map(programPath => vscode.workspace.asRelativePath(programPath));
  if (names.length === 1) {
    throw new Error(`${names[0]} has unsaved changes. Save it before compiling.`);
  }

  throw new Error(`These programs have unsaved changes: ${names.join(", ")}. Save them before compiling.`);
}

interface ProjectSettings {
  serialPort: string;
  baudRate: number;
}

function loadSettings(options: { requireProgrammer?: boolean } = {}): ProjectSettings {
  const { requireProgrammer = false } = options;

  // The compiler is checked by Project when it compiles, so upload-only
  // commands work without one. Only upload/hardware operations talk to the
  // programmer; compile-only commands must work with no serial port configured.
  const serialPort = Config.getSerialPort();
  if (requireProgrammer && !serialPort) {
    throw new Error("Serial port is not set in Settings.");
  }

  return {
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

async function getWorkspaceFolder(target?: vscode.Uri): Promise<string | null> {
  const folders = vscode.workspace.workspaceFolders;

  if (!folders || folders.length === 0) {
    vscode.window.showErrorMessage("No workspace folder open.");
    return null;
  }

  // A command that acts on a file belongs to that file's folder, so a
  // multi-root workspace doesn't need the picker for it.
  if (target) {
    const targetFolder = vscode.workspace.getWorkspaceFolder(target);
    if (targetFolder) {
      return targetFolder.uri.fsPath;
    }
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
