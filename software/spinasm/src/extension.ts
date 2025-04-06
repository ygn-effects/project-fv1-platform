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
    vscode.commands.registerCommand("spinasm.compileProgram1", compileProgram1),
    vscode.commands.registerCommand("spinasm.compileProgram2", compileProgram2),
    vscode.commands.registerCommand("spinasm.compileProgram3", compileProgram3),
    vscode.commands.registerCommand("spinasm.compileProgram4", compileProgram4),
    vscode.commands.registerCommand("spinasm.compileProgram5", compileProgram5),
    vscode.commands.registerCommand("spinasm.compileProgram6", compileProgram6),
    vscode.commands.registerCommand("spinasm.compileProgram7", compileProgram7),
    vscode.commands.registerCommand("spinasm.compileCurrentProgram", compileCurrentProgram),
    vscode.commands.registerCommand("spinasm.compileAllPrograms", compileAllPrograms),
    vscode.commands.registerCommand("spinasm.compileAllProgramsToBin", compileAllProgramsToBin),
    vscode.commands.registerCommand("spinasm.uploadProgram0", uploadProgram0),
    vscode.commands.registerCommand("spinasm.uploadProgram1", uploadProgram1),
    vscode.commands.registerCommand("spinasm.uploadProgram2", uploadProgram2),
    vscode.commands.registerCommand("spinasm.uploadProgram3", uploadProgram3),
    vscode.commands.registerCommand("spinasm.uploadProgram4", uploadProgram4),
    vscode.commands.registerCommand("spinasm.uploadProgram5", uploadProgram5),
    vscode.commands.registerCommand("spinasm.uploadProgram6", uploadProgram6),
    vscode.commands.registerCommand("spinasm.uploadProgram7", uploadProgram7),
    vscode.commands.registerCommand("spinasm.uploadCurrentProgram", uploadCurrentProgram)
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
 * @brief Compiles bank 0 program to HEX format.
 */
async function compileProgram0(): Promise<void> {
  try {
    compileProgramToHex(0);

    Logs.log(LogType.INFO, "Program 0 compilation successful");
    vscode.window.showInformationMessage("Program 0 compiled successfully!");
  }
  catch (error) {
    handleError(error, "Failed to compile program 0.");
  }
}

/**
 * @brief Compiles bank 1 program to HEX format.
 */
async function compileProgram1(): Promise<void> {
  try {
    compileProgramToHex(1);

    Logs.log(LogType.INFO, "Program 1 compilation successful");
    vscode.window.showInformationMessage("Program 1 compiled successfully!");
  }
  catch (error) {
    handleError(error, "Failed to compile program 1.");
  }
}

/**
 * @brief Compiles bank 2 program to HEX format.
 */
async function compileProgram2(): Promise<void> {
  try {
    compileProgramToHex(2);

    Logs.log(LogType.INFO, "Program 2 compilation successful");
    vscode.window.showInformationMessage("Program 2 compiled successfully!");
  }
  catch (error) {
    handleError(error, "Failed to compile program 3.");
  }
}

/**
 * @brief Compiles bank 3 program to HEX format.
 */
async function compileProgram3(): Promise<void> {
  try {
    compileProgramToHex(3);

    Logs.log(LogType.INFO, "Program 3 compilation successful");
    vscode.window.showInformationMessage("Program 3 compiled successfully!");
  }
  catch (error) {
    handleError(error, "Failed to compile program 3.");
  }
}

/**
 * @brief Compiles bank 4 program to HEX format.
 */
async function compileProgram4(): Promise<void> {
  try {
    compileProgramToHex(4);

    Logs.log(LogType.INFO, "Program 4 compilation successful");
    vscode.window.showInformationMessage("Program 4 compiled successfully!");
  }
  catch (error) {
    handleError(error, "Failed to compile program 4.");
  }
}

/**
 * @brief Compiles bank 5 program to HEX format.
 */
async function compileProgram5(): Promise<void> {
  try {
    compileProgramToHex(5);

    Logs.log(LogType.INFO, "Program 5 compilation successful");
    vscode.window.showInformationMessage("Program 5 compiled successfully!");
  }
  catch (error) {
    handleError(error, "Failed to compile program 5.");
  }
}

/**
 * @brief Compiles bank 6 program to HEX format.
 */
async function compileProgram6(): Promise<void> {
  try {
    compileProgramToHex(6);

    Logs.log(LogType.INFO, "Program 6 compilation successful");
    vscode.window.showInformationMessage("Program 6 compiled successfully!");
  }
  catch (error) {
    handleError(error, "Failed to compile program 6.");
  }
}

/**
 * @brief Compiles bank 6 program to HEX format.
 */
async function compileProgram7(): Promise<void> {
  try {
    compileProgramToHex(7);

    Logs.log(LogType.INFO, "Program 7 compilation successful");
    vscode.window.showInformationMessage("Program 7 compiled successfully!");
  }
  catch (error) {
    handleError(error, "Failed to compile program 7.");
  }
}

/**
 * @brief Compiles the current program to HEX.
 */
async function compileCurrentProgram(): Promise<void> {
  const folder = await getWorkspaceFolder();
  if (!folder) {
    return;
  }

  try {
    const { compilerPath, compilerArgs } = loadProjectSettings(folder);
    const project = new Project(folder);
    project.buildSetup(compilerPath, compilerArgs);

    const currentProgram = project.getProgramBankByPath(vscode.window.activeTextEditor?.document.uri.fsPath);
    project.compileProgramToHex(currentProgram);

    Logs.log(LogType.INFO, `Program ${currentProgram} compilation successful`);
    vscode.window.showInformationMessage(`Program ${currentProgram} compiled successfully!`);
  }
  catch (error) {
    handleError(error, "Failed to compile current program.");
  }
}

/**
 * @brief Compiles all the available programs to HEX.
 */
async function compileAllPrograms(): Promise<void> {
  const folder = await getWorkspaceFolder();
  if (!folder) {
    return;
  }

  try {
    const { compilerPath, compilerArgs } = loadProjectSettings(folder);
    const project = new Project(folder);
    project.buildSetup(compilerPath, compilerArgs);

    const programs = project.getAllPrograms();

    for (const programPath of programs) {
      const currentProgram = project.getProgramBankByPath(programPath);
      project.buildSetup(compilerPath, compilerArgs);

      project.compileProgramToHex(currentProgram);

      Logs.log(LogType.INFO, `Program ${currentProgram} compilation successful`);
    }

    vscode.window.showInformationMessage("All programs compiled successfully!");
  }
  catch (error) {
    handleError(error, "Failed to compile all programs.");
  }
}


/**
 * @brief Compiles all the available programs to BIN.
 */
async function compileAllProgramsToBin(): Promise<void> {
  const folder = await getWorkspaceFolder();
  if (!folder) {
    return;
  }

  try {
    const { compilerPath, compilerArgs } = loadProjectSettings(folder);
    const project = new Project(folder);
    project.buildSetup(compilerPath, compilerArgs);

    const programs = project.getAllPrograms();

    for (const programPath of programs) {
      const currentProgram = project.getProgramBankByPath(programPath);
      project.buildSetup(compilerPath, compilerArgs);

      project.compileProgramToBin(currentProgram);

      Logs.log(LogType.INFO, `Program ${currentProgram} compilation successful`);
    }

    vscode.window.showInformationMessage("All programs compiled successfully!");
  }
  catch (error) {
    handleError(error, "Failed to compile all programs.");
  }
}

/**
 * @brief Upload bank 0 program.
 */
async function uploadProgram0(): Promise<void> {
  try {
    await uploadProgram(0);

    Logs.log(LogType.INFO, "Program 0 upload successful");
    vscode.window.showInformationMessage("Program 0 uploaded successfully!");
  }
  catch (error) {
    handleError(error, "Failed to upload program 0.");
  }
}

/**
 * @brief Upload bank 1 program.
 */
async function uploadProgram1(): Promise<void> {
  try {
    await uploadProgram(1);

    Logs.log(LogType.INFO, "Program 1 upload successful");
    vscode.window.showInformationMessage("Program 1 uploaded successfully!");
  }
  catch (error) {
    handleError(error, "Failed to upload program 1.");
  }
}

/**
 * @brief Upload bank 2 program.
 */
async function uploadProgram2(): Promise<void> {
  try {
    await uploadProgram(2);

    Logs.log(LogType.INFO, "Program 2 upload successful");
    vscode.window.showInformationMessage("Program 2 uploaded successfully!");
  }
  catch (error) {
    handleError(error, "Failed to upload program 2.");
  }
}

/**
 * @brief Upload bank 3 program.
 */
async function uploadProgram3(): Promise<void> {
  try {
    await uploadProgram(3);

    Logs.log(LogType.INFO, "Program 3 upload successful");
    vscode.window.showInformationMessage("Program 3 uploaded successfully!");
  }
  catch (error) {
    handleError(error, "Failed to upload program 3.");
  }
}

/**
 * @brief Upload bank 4 program.
 */
async function uploadProgram4(): Promise<void> {
  try {
    await uploadProgram(4);

    Logs.log(LogType.INFO, "Program 4 upload successful");
    vscode.window.showInformationMessage("Program 4 uploaded successfully!");
  }
  catch (error) {
    handleError(error, "Failed to upload program 4.");
  }
}

/**
 * @brief Upload bank 5 program.
 */
async function uploadProgram5(): Promise<void> {
  try {
    await uploadProgram(5);

    Logs.log(LogType.INFO, "Program 5 upload successful");
    vscode.window.showInformationMessage("Program 5 uploaded successfully!");
  }
  catch (error) {
    handleError(error, "Failed to upload program 5.");
  }
}

/**
 * @brief Upload bank 6 program.
 */
async function uploadProgram6(): Promise<void> {
  try {
    await uploadProgram(6);

    Logs.log(LogType.INFO, "Program 6 upload successful");
    vscode.window.showInformationMessage("Program 6 uploaded successfully!");
  }
  catch (error) {
    handleError(error, "Failed to upload program 6.");
  }
}

/**
 * @brief Upload bank 7 program.
 */
async function uploadProgram7(): Promise<void> {
  try {
    await uploadProgram(7);

    Logs.log(LogType.INFO, "Program 7 upload successful");
    vscode.window.showInformationMessage("Program 7 uploaded successfully!");
  }
  catch (error) {
    handleError(error, "Failed to upload program 7.");
  }
}

/**
 * @brief Upload current program.
 */
async function uploadCurrentProgram(): Promise<void> {
  const folder = await getWorkspaceFolder();
  if (!folder) {
    return;
  }

  try {
    const { compilerPath, compilerArgs } = loadProjectSettings(folder);

    const project = new Project(folder);
    project.buildSetup(compilerPath, compilerArgs);

    const currentProgram = project.getProgramBankByPath(vscode.window.activeTextEditor?.document.uri.fsPath);
    await uploadProgram(currentProgram);

    Logs.log(LogType.INFO, "Program 7 upload successful");
    vscode.window.showInformationMessage("Program 7 uploaded successfully!");
  }
  catch (error) {
    handleError(error, "Failed to upload program 7.");
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

async function compileProgramToHex(bank: number): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  try {
    const { compilerPath, compilerArgs } = loadProjectSettings(folder);
    const project = new Project(folder);
    project.buildSetup(compilerPath, compilerArgs);
    project.compileProgramToHex(bank);
  }
  catch (error) {
    handleError(error, `Failed to compile program ${bank}.`);
  }
}

async function uploadProgram(bank: number): Promise<void> {
  const folder = await getWorkspaceFolder();

  if (!folder) {
    return;
  }

  const { compilerPath, compilerArgs, serialPort, baudRate } = loadProjectSettings(folder);

  try {
    const project = new Project(folder);
    project.buildSetup(compilerPath, compilerArgs);

    const programmer = new Programmer(serialPort, baudRate);
    await programmer.connect();

    const isConnected = await programmer.isProgrammerConnected();

    if (!isConnected) {
      throw new Error("Programmer did not respond correctly.");
    }

    const program = programmer.readIntelHexData(project.getOutput(bank));

    await programmer.writeProgram(program.address, program.data);
    const programRead = await programmer.readProgram(program.address);
    await programmer.disconnect();

    if (Buffer.compare(program.data, programRead) != 0) {
      throw new Error("Data verification failed.");
    }
  }
  catch (error) {
    handleError(error, `Failed to upload program ${bank}.`);
  }
  finally {
    const programmer = new Programmer(serialPort, baudRate);
    await programmer.disconnect();
  }
}
