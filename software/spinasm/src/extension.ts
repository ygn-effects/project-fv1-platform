import * as vscode from "vscode";
import * as path from "path";
import Project from "./project";
import Config from "./config";
import Logs, { LogType } from "./logs";
import Programmer from "./programmer";

function delay(ms: number): Promise<void> {
  return new Promise(resolve => setTimeout(resolve, ms));
}

/**
 * @brief Activates the SpinASM VSCode extension.
 *
 * Initializes logging and registers all extension commands.
 *
 * @param context - VSCode extension context for command subscriptions.
 */
export function activate(context: vscode.ExtensionContext): void {
  Logs.createChannel();
  Logs.log(LogType.INFO, "Extension activated");

  context.subscriptions.push(
    vscode.commands.registerCommand("spinasm.createProject", createProject),
    vscode.commands.registerCommand("spinasm.checkProjectSettings", checkProjectSettings),
    vscode.commands.registerCommand("spinasm.showSerialConfig", showSerialConfig),
    vscode.commands.registerCommand("spinasm.compileProgram0", compileProgram0),
  );

  Logs.log(LogType.INFO, "Commands registered successfully");
}

/**
 * @brief Deactivates the SpinASM VSCode extension.
 *
 * Cleans up resources and logs the deactivation event.
 */
export function deactivate(): void {
  Logs.log(LogType.INFO, "Extension deactivated");
  Logs.disposeChannel();
}

async function test(): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  const { compilerPath, compilerArgs, serialPort, baudRate } = loadProjectSettings(folder);

  try {
    const project = new Project(folder);
    project.buildSetup(compilerPath, compilerArgs);
    project.checkCompiler();

    const programmer = new Programmer(serialPort, baudRate);
    await programmer.connect();

    const isConnected = await programmer.isProgrammerConnected();

    if (!isConnected) {
      throw new Error("Programmer did not respond correctly.");
    }

    await programmer.test();
    await programmer.disconnect();
  }
  catch (error) {
    handleError(error, "Failed to validate the compiler and programmer");
  }
  finally {
    const programmer = new Programmer(serialPort, baudRate);
    // await programmer.sendEndOrder();
    await programmer.disconnect();
  }
}

/**
 * @brief Initializes a new SpinASM project structure.
 */
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

/**
 * @brief Validates the project compiler and programmer settings
 */
async function checkProjectSettings(): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  const { compilerPath, compilerArgs, serialPort, baudRate } = loadProjectSettings(folder);

  try {
    const project = new Project(folder);
    project.buildSetup(compilerPath, compilerArgs);
    project.checkCompiler();

    const programmer = new Programmer(serialPort, baudRate);
    await programmer.connect();

    const isConnected = await programmer.isProgrammerConnected();

    if (!isConnected) {
      throw new Error("Programmer did not respond correctly.");
    }

    const eepromReady = await programmer.isEepromReady();

    if (!eepromReady) {
      throw new Error("EEPROM is not ready.");
    }

    await programmer.disconnect();

    Logs.log(LogType.INFO, "Programmer and EEPROM ready for operations.");

    Logs.log(LogType.INFO, "Compiler and programmer validation succeeded");
    vscode.window.showInformationMessage("Compiler and programmer are working correctly!");
  }
  catch (error) {
    handleError(error, "Failed to validate the compiler and programmer");
  }
  finally {
    const programmer = new Programmer(serialPort, baudRate);
    // await programmer.sendEndOrder();
    await programmer.disconnect();
  }
}

/**
 * @brief Displays the current serial configuration from project settings.
 */
async function showSerialConfig(): Promise<void> {
  const folder = await getWorkspaceFolder();
  if (!folder){
    return;
  }

  try {
    const config = new Config(path.join(folder, "settings.ini"));
    const serialPort = config.readSerialPort();
    const baudRate = config.readBaudRate();

    Logs.log(LogType.INFO, `Serial Port: ${serialPort}`);
    Logs.log(LogType.INFO, `Baud Rate: ${baudRate}`);

    vscode.window.showInformationMessage(
      `Serial Configuration:\nPort: ${serialPort}\nBaud Rate: ${baudRate}`
    );
  }
  catch (error) {
    handleError(error, "Failed to read serial configuration");
  }
}

/**
 * @brief Compiles the first SpinASM program (bank 0) to HEX format.
 */
async function compileProgram0(): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  try {
    const { compilerPath, compilerArgs } = loadProjectSettings(folder);
    const project = new Project(folder);
    project.buildSetup(compilerPath, compilerArgs);
    project.compileProgramToHex(0);

    Logs.log(LogType.INFO, "Program 0 compilation successful");
    vscode.window.showInformationMessage("Program 0 compiled successfully!");
  }
  catch (error) {
    handleError(error, "Failed to compile program 0.");
  }
}

/**
 * @brief Retrieves the project settings from the project .ini file
 *
 * @param folder - The workspace folder path.
 * @returns Object containing the project settings.
 */
function loadProjectSettings(folder: string): { compilerPath: string; compilerArgs: string[]; serialPort: string; baudRate: number } {
  const config = new Config(path.join(folder, "settings.ini"));

  return {
    compilerPath: config.readCompilerCommand(),
    compilerArgs: config.readCompilerArgs(),
    serialPort: config.readSerialPort(),
    baudRate: config.readBaudRate(),
  };
}

/**
 * @brief Handles errors by logging them and notifying the user.
 *
 * @param error - Caught error object.
 * @param message - Contextual description of the error.
 */
function handleError(error: unknown, message: string): void {
  const errorMessage = (error as Error).message;
  Logs.log(LogType.ERROR, `${message}: ${errorMessage}`);
  vscode.window.showErrorMessage(`${message}: ${errorMessage}`);
}

/**
 * @brief Retrieves the workspace folder, prompting the user if multiple are open.
 *
 * @returns Path of the selected workspace folder, or null if none selected.
 */
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
