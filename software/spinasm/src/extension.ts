import * as vscode from "vscode";
import * as path from "path";
import Project from "./project";
import Config from "./config";
import Logs, { LogType } from "./logs";

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
    vscode.commands.registerCommand("spinasm.checkCompiler", checkCompiler),
    vscode.commands.registerCommand("spinasm.compileProgram", compileProgram),
    vscode.commands.registerCommand("spinasm.showSerialConfig", showSerialConfig)
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
 * @brief Validates the SpinASM compiler setup.
 */
async function checkCompiler(): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  try {
    const { compilerPath, compilerArgs } = loadCompilerSettings(folder);
    const project = new Project(folder);
    project.buildSetup(compilerPath, compilerArgs);
    project.checkCompiler();

    Logs.log(LogType.INFO, "Compiler validation succeeded");
    vscode.window.showInformationMessage("Compiler is working correctly!");
  }
  catch (error) {
    handleError(error, "Failed to validate the compiler");
  }
}

/**
 * @brief Compiles the first SpinASM program (bank 0) to HEX format.
 */
async function compileProgram(): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  try {
    const { compilerPath, compilerArgs } = loadCompilerSettings(folder);
    const project = new Project(folder);
    project.buildSetup(compilerPath, compilerArgs);
    project.compileProgramToHex(0);

    Logs.log(LogType.INFO, "Program compilation successful");
    vscode.window.showInformationMessage("Program compiled successfully!");
  }
  catch (error) {
    handleError(error, "Failed to compile program");
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
 * @brief Retrieves compiler path and arguments from project settings.
 *
 * @param folder - The workspace folder path.
 * @returns Object containing compiler path and arguments.
 */
function loadCompilerSettings(folder: string): { compilerPath: string; compilerArgs: string[] } {
  const config = new Config(path.join(folder, "settings.ini"));

  return {
    compilerPath: config.readCompilerCommand(),
    compilerArgs: config.readCompilerArgs(),
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
