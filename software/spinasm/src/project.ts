import * as fs from "fs";
import * as path from "path";
import * as cp from "child_process";
import Logs, { LogType } from "./logs";

/**
 * @class Project
 * @brief Manages project files, directories, compilation workflow, and output handling for SpinASM.
 *
 * Encapsulates operations related to creating project structure, compiling SpinASM code,
 * and handling compiler outputs within a VSCode workspace.
 */
export default class Project {
  private rootFolder: string;              ///< Root project directory.
  private outputFolder: string;            ///< Directory to store compilation outputs.
  private iniFilePath: string;             ///< Path to project settings (`settings.ini`).
  private outputBinFile: string;           ///< Path for compiled binary file.
  private compiler: string;                ///< Path to compiler executable.
  private compilerArguments: string[];     ///< Arguments passed to the compiler.
  private programs: (string | null)[];     ///< Paths of available program files.
  private outputs: string[];               ///< Output file paths for compiled programs.

  /**
   * @brief Constructs a new Project instance.
   * @param folder Path to the project's root directory.
   */
  constructor(folder: string) {
    this.rootFolder = folder;
    this.outputFolder = path.join(this.rootFolder, "output");
    this.iniFilePath = path.join(this.rootFolder, "settings.ini");
    this.outputBinFile = path.join(this.outputFolder, "output.bin");
    this.compiler = "";
    this.compilerArguments = [];
    this.programs = [];
    this.outputs = [];
  }

  /**
   * @brief Checks whether the project structure exists.
   * @returns True if the project is empty (no settings.ini).
   */
  public emptyProject(): boolean {
    return !fs.existsSync(this.iniFilePath);
  }

  /**
   * @brief Validates the compiler's availability and functionality.
   * @throws Error if the compiler path is invalid or the compiler fails execution.
   */
  public checkCompiler(): void {
    if (!fs.existsSync(this.compiler)) {
      throw new Error(`Compiler path invalid: ${this.compiler}`);
    }

    Logs.log(LogType.INFO, "Compiler path valid");
    this.compilerArguments = ["-v"];
    const result = this.runCompiler();

    if (result !== 0) {
      throw new Error(`Compiler check failed with return code: ${result}`);
    }

    Logs.log(LogType.INFO, "Compiler working successfully");
  }

  /**
   * @brief Creates project directories, placeholder program files, and a default settings.ini.
   * @throws Error if directory or file creation fails.
   */
  public createProjectStructure(): void {
    const programContent = "; Blank SpinASM program";

    const iniFileContent = `
;Project config file

[asfv1]
;Path of the executable
path = C:\\path\\to\\asfv1.exe

;Compiler options:
; -c clamp out-of-range values
; -s SpinASM compatibility
; -q suppress warnings
options = -s

[serial]
port = COM6
baudrate = 57600
`;

    // Create bank folders and default programs
    for (let i = 0; i < 8; i++) {
      const folder = path.join(this.rootFolder, `bank_${i}`);
      const file = path.join(folder, `${i}_programName.spn`);

      try {
        if (!fs.existsSync(folder)) {
          Logs.log(LogType.INFO, `Creating folder: ${folder}`);
          fs.mkdirSync(folder, { recursive: true });
        }

        if (!fs.existsSync(file)) {
          Logs.log(LogType.INFO, `Creating file: ${file}`);
          fs.writeFileSync(file, programContent);
        }
        else {
          Logs.log(LogType.INFO, `Program file already exists: ${file}`);
        }
      }
      catch (error) {
        throw new Error(`Error setting up bank ${i}: ${(error as Error).message}`);
      }
    }

    // Create output directory
    try {
      if (!fs.existsSync(this.outputFolder)) {
        Logs.log(LogType.INFO, `Creating output folder: ${this.outputFolder}`);
        fs.mkdirSync(this.outputFolder, { recursive: true });
      }
    }
    catch (error) {
      throw new Error(`Could not create output folder: ${(error as Error).message}`);
    }

    // Create settings.ini if it doesn't exist
    try {
      if (!fs.existsSync(this.iniFilePath)) {
        Logs.log(LogType.INFO, `Creating ini file: ${this.iniFilePath}`);
        fs.writeFileSync(this.iniFilePath, iniFileContent.trim());
      }
    }
    catch (error) {
      throw new Error(`Could not create ini file: ${(error as Error).message}`);
    }
  }

  /**
   * @brief Runs the compiler process synchronously.
   * @returns The compiler's exit code.
   * @throws Error if the compiler fails execution.
   */
  private runCompiler(): number {
    const output = cp.spawnSync(this.compiler, this.compilerArguments, { encoding: "utf8" });
    Logs.log(LogType.INFO, `Compiler return code: ${output.status}`);

    if (output.stdout) {
      Logs.log(LogType.INFO, `Compiler stdout: ${output.stdout}`);
    }

    if (output.stderr) {
      if (output.status === 0) {
        Logs.log(LogType.INFO, `Compiler stderr (info/warning): ${output.stderr}`);
      }
      else {
        Logs.log(LogType.ERROR, `Compiler stderr: ${output.stderr}`);
      }
    }

    return output.status ?? 1; // Defaults to 1 if undefined
  }

  /**
   * @brief Configures the compiler settings for building.
   * @param compiler Path to the compiler executable.
   * @param compilerArgs Arguments for the compiler.
   */
  public buildSetup(compiler: string, compilerArgs: string[]): void {
    this.compiler = compiler;
    this.compilerArguments = [...compilerArgs];

    Logs.log(LogType.INFO, `Compiler set to: ${compiler}`);
    Logs.log(LogType.INFO, `Compiler args: ${compilerArgs.join(" ")}`);

    this.getAvailablePrograms();
    Logs.log(LogType.INFO, "Available programs identified.");
  }

  /**
   * @brief Populates the internal arrays with available programs and corresponding outputs.
   */
  private getAvailablePrograms(): void {
    this.programs = [];
    this.outputs = [];

    for (let i = 0; i < 8; i++) {
      const currentFolder = path.join(this.rootFolder, `bank_${i}`);
      const programFile = fs.readdirSync(currentFolder).find(file => /^[0-7].*\.spn$/.test(file));

      if (programFile) {
        this.programs[i] = path.join(currentFolder, programFile);
        this.outputs[i] = path.join(this.outputFolder, `${path.parse(programFile).name}.hex`);
      }
      else {
        this.programs[i] = null;
        this.outputs[i] = "";
      }
    }
  }

  /**
   * @brief Removes all compiled `.hex` files.
   */
  public removeHexPrograms(): void {
    for (const output of this.outputs) {
      if (output && fs.existsSync(output)) {
        try {
          Logs.log(LogType.INFO, `Removing file: ${output}`);
          fs.unlinkSync(output);
        }
        catch (error) {
          throw new Error(`Could not remove ${output}: ${(error as Error).message}`);
        }
      }
    }
  }

  /**
   * @brief Removes a compiled `.hex` file.
   */
  public removeHexProgram(path: any): void {
    if (fs.existsSync(path)) {
      try {
        Logs.log(LogType.INFO, `Removing file: ${path}`);
        fs.unlinkSync(path);
      }
      catch (error) {
        throw new Error(`Could not remove ${path}: ${(error as Error).message}`);
      }
    }
  }

  /**
   * @brief Removes the compiled `.bin` file.
   */
  public removeBinPrograms(): void {
    if (fs.existsSync(this.outputBinFile)) {
      try {
        Logs.log(LogType.INFO, `Removing file: ${this.outputBinFile}`);
        fs.unlinkSync(this.outputBinFile);
      }
      catch (error) {
        throw new Error(`Could not remove ${this.outputBinFile}: ${(error as Error).message}`);
      }
    }
  }

  /**
   * @brief Compiles the specified program index into a `.hex` file.
   * @param program Index of the program to compile.
   * @throws Error if compilation fails.
   */
  public compileProgramToHex(program: number): void {
    this.removeHexProgram(this.outputs[program]);

    if (!this.programs[program]) {
      throw new Error(`Program at index ${program} does not exist.`);
    }

    this.compilerArguments.push("-p", program.toString(), this.programs[program]!, this.outputs[program]);

    const result = this.runCompiler();
    if (result !== 0) {
      throw new Error(`Compilation failed for program ${program} with return code: ${result}`);
    }

    Logs.log(LogType.INFO, "Compilation succeeded.");
  }

  /**
   * @brief Compiles the specified program index into a `.bin` file.
   * @param program Index of the program to compile.
   * @throws Error if compilation fails.
   */
  public compileProgramToBin(program: number): void {
    if (!this.programs[program]) {
      throw new Error(`Program at index ${program} does not exist.`);
    }

    this.compilerArguments.push("-p", program.toString(), this.programs[program]!, this.outputBinFile);

    const result = this.runCompiler();
    if (result !== 0) {
      throw new Error(`Compilation failed for program ${program} with return code: ${result}`);
    }

    Logs.log(LogType.INFO, "Compilation succeeded.");
  }

  public getProgramBankByPath(path: string | undefined | null): number {
    if (typeof path === 'undefined') {
      throw new Error("Path cannot be undefined");
    }

    return this.programs.indexOf(path);
  }

  public getAllPrograms(): (string | null)[] {
    return this.programs;
  }

  public getOutput(bank: number): (string | null) {
    return this.outputs[bank];
  }
}
