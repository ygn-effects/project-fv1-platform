import * as vscode from "vscode";
import Project from "./project";
import Config from "./config";
import Logs, { LogType } from "./logs";

export function activate(context: vscode.ExtensionContext): void {
  // Create the log channel
  Logs.createChannel();
  Logs.log(LogType.INFO, "Extension activated");

  // Command to initialize a blank project
  const createProjectCommand = vscode.commands.registerCommand(
    "spinasm.createProject",
    async () => {
      const folder = await getWorkspaceFolder();
      if (!folder) {
        return;
      };

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
  );

  // Command to check the compiler
  const checkCompilerCommand = vscode.commands.registerCommand(
    "spinasm.checkCompiler",
    async () => {
      const folder = await getWorkspaceFolder();
      if (!folder) {
        return;
      };

      try {
        const config = new Config(`${folder}/settings.ini`);
        const compilerPath = config.readCompilerCommand();
        const compilerArgs = config.readCompilerArgs();

        const project = new Project(folder);
        project.buildSetup(compilerPath, compilerArgs);
        project.checkCompiler();

        Logs.log(LogType.INFO, "Compiler check passed");
        vscode.window.showInformationMessage("Compiler is working correctly!");
      }
      catch (error) {
        handleError(error, "Failed to validate the compiler");
      }
    }
  );

  // Command to compile a specific program
  const compileProgramCommand = vscode.commands.registerCommand(
    "spinasm.compileProgram",
    async () => {
      const folder = await getWorkspaceFolder();
      if (!folder) {
        return;
      };

      try {
        const config = new Config(`${folder}/settings.ini`);
        const compilerPath = config.readCompilerCommand();
        const compilerArgs = config.readCompilerArgs();

        const project = new Project(folder);
        project.buildSetup(compilerPath, compilerArgs);
        project.compileProgramToHex(0); // Compile program 0 for testing

        Logs.log(LogType.INFO, "Program compiled successfully");
        vscode.window.showInformationMessage("Program compiled successfully!");
      }
      catch (error) {
        handleError(error, "Failed to compile program");
      }
    }
  );

  // Command to display serial configuration
  const showSerialConfigCommand = vscode.commands.registerCommand(
    "spinasm.showSerialConfig",
    async () => {
      const folder = await getWorkspaceFolder();
      if (!folder) {
        return;
      };

      try {
        const config = new Config(`${folder}/settings.ini`);
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
  );

  // Register all commands
  context.subscriptions.push(
    createProjectCommand,
    checkCompilerCommand,
    compileProgramCommand,
    showSerialConfigCommand
  );

  Logs.log(LogType.INFO, "Commands registered successfully");
}

export function deactivate(): void {
  Logs.log(LogType.INFO, "Extension deactivated");
}

/**
 * @brief Handles errors by logging them and showing an error message to the user.
 * @param error - The caught error.
 * @param message - A descriptive error message.
 */
function handleError(error: unknown, message: string): void {
  const errorMessage = (error as Error).message;
  Logs.log(LogType.ERROR, `${message}: ${errorMessage}`);
  vscode.window.showErrorMessage(`${message}: ${errorMessage}`);
}

/**
 * @brief Retrieves the workspace folder path, prompting the user if multiple folders are open.
 * @returns The selected workspace folder path, or null if no folder is selected.
 */
async function getWorkspaceFolder(): Promise<string | null> {
  const folders = vscode.workspace.workspaceFolders;
  if (!folders || folders.length === 0) {
    vscode.window.showErrorMessage("No workspace folder open.");
    return null;
  }

  // Allow the user to select a folder if there are multiple
  if (folders.length > 1) {
    const selectedFolder = await vscode.window.showQuickPick(
      folders.map((folder) => folder.uri.fsPath),
      { placeHolder: "Select a workspace folder" }
    );
    return selectedFolder || null;
  }

  return folders[0].uri.fsPath;
}
