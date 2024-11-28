import * as fs from "fs";
import * as path from "path";
import * as cp from "child_process";
import Logs, { LogType } from "./logs";

/**
 * @class Project
 * @brief Manages project files, folders, compilation, and output handling for the SpinASM extension.
 */
export default class Project {
  private rootFolder: string;
  private outputFolder: string;
  private iniFilePath: string;
  private outputBinFile: string;
  private compiler: string;
  private compilerArguments: string[];
  private programs: (string | null)[];
  private outputs: string[];

  /**
   * @brief Constructor to initialize the project.
   * @param folder Path to the root folder of the project.
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
   * @brief Checks if the project is empty.
   * @returns True if the `settings.ini` file does not exist.
   */
  public emptyProject(): boolean {
    return !fs.existsSync(this.iniFilePath);
  }

  /**
   * @brief Verifies that the compiler is valid and functional.
   * @throws Error if the compiler path is invalid or the compiler fails.
   */
  public checkCompiler(): void {
    if (fs.existsSync(this.compiler)) {
      Logs.log(LogType.INFO, "Compiler path valid");
      try {
        this.compilerArguments = ["-v"];
        const result = this.runCompiler();
        if (result === 0) {
          Logs.log(LogType.INFO, "Compiler working successfully");
        }
      }
      catch (error) {
        throw new Error((error as Error).message);
      }
    }
    else {
      throw new Error("Compiler path invalid");
    }
  }

  /**
   * @brief Creates the project folder structure, including program folders, output folder, and `settings.ini`.
   * @throws Error if any folder or file creation fails.
   */
  public createProjectStructure(): void {
    const programContent = ";Blank program";

    const iniFileContent = `
;Project config file

[asfv1]
;Path of the executable
path = C:\\path\\to\\asfv1.exe

;Compiler options separated by a space:
; -c clamp out-of-range values without error
; -s read literals 2,1 as float (SpinASM compatibility)
; -q suppress warnings
options = -s

[serial]
;Serial port for the programmer
port = COM6
baudrate = 57600
`;

    for (let i = 0; i < 8; i++) {
      const folder = path.join(this.rootFolder, `bank_${i}`);
      const file = path.join(folder, `${i}_programName.spn`);

      if (!fs.existsSync(folder)) {
        try {
          Logs.log(LogType.INFO, `Creating folder: ${folder}`);
          fs.mkdirSync(folder);
        }
        catch (error) {
          throw new Error(`Could not create ${folder}: ${(error as Error).message}`);
        }

        try {
          Logs.log(LogType.INFO, `Creating file: ${file}`);
          fs.writeFileSync(file, programContent);
        }
        catch (error) {
          throw new Error(`Could not create ${file}: ${(error as Error).message}`);
        }
      }
      else {
        Logs.log(LogType.INFO, `Folder ${folder} already exists`);
        const programFile = fs
          .readdirSync(folder)
          .find((str) => str.match("^[0-7].*\\.spn"));
        if (!programFile) {
          try {
            fs.writeFileSync(file, programContent);
          }
          catch (error) {
            throw new Error(`Could not create ${file}: ${(error as Error).message}`);
          }
        }
        else {
          Logs.log(LogType.INFO, `Program already exists in folder ${folder}: ${programFile}`);
        }
      }
    }

    if (!fs.existsSync(this.outputFolder)) {
      try {
        Logs.log(LogType.INFO, `Creating output folder: ${this.outputFolder}`);
        fs.mkdirSync(this.outputFolder);
      }
      catch (error) {
        throw new Error(`Could not create ${this.outputFolder}: ${(error as Error).message}`);
      }
    }
    else {
      Logs.log(LogType.INFO, `Output folder ${this.outputFolder} already exists`);
    }

    if (!fs.existsSync(this.iniFilePath)) {
      try {
        Logs.log(LogType.INFO, `Creating ini file: ${this.iniFilePath}`);
        fs.writeFileSync(this.iniFilePath, iniFileContent);
      } catch (error) {
        throw new Error(`Could not create ${this.iniFilePath}: ${(error as Error).message}`);
      }
    }
    else {
      Logs.log(LogType.INFO, `Ini file ${this.iniFilePath} already exists`);
    }
  }

  /**
   * @brief Runs the compiler with the specified arguments.
   * @returns The compiler's return code.
   * @throws Error if the compiler fails to execute.
   */
  private runCompiler(): number {
    try {
      const output = cp.spawnSync(this.compiler, this.compilerArguments, { encoding: "utf8" });
      Logs.log(LogType.INFO, `asfv1 return code: ${output.status}`);

      if (output.stdout) {
        Logs.log(LogType.INFO, `asfv1 stdout: ${output.stdout}`);
      }
      if (output.stderr) {
        Logs.log(LogType.ERROR, `asfv1 stderr: ${output.stderr}`);
      }

      return output.status || 1; // Default to non-zero if undefined
    }
    catch (error) {
      throw new Error((error as Error).message);
    }
  }

  /**
   * @brief Sets up the compiler for a build.
   * @param compiler Path to the compiler executable.
   * @param compilerArgs Array of compiler arguments.
   */
  public buildSetup(compiler: string, compilerArgs: string[]): void {
    this.compiler = compiler;
    this.compilerArguments = [...compilerArgs];

    Logs.log(LogType.INFO, `Compiler: ${compiler}`);
    Logs.log(LogType.INFO, `Compiler args: ${compilerArgs.join(" ")}`);

    this.getAvailablePrograms();

    Logs.log(LogType.INFO, "Available programs:");
    this.programs.forEach((program, index) => {
      Logs.log(LogType.INFO, `Program ${index}: ${program}`);
    });
    this.outputs.forEach((output, index) => {
      Logs.log(LogType.INFO, `Output ${index}: ${output}`);
    });

    Logs.log(LogType.INFO, "Compiler ready");
  }

  /**
   * @brief Identifies available programs and their outputs in the project structure.
   */
  private getAvailablePrograms(): void {
    this.programs = [];
    this.outputs = [];

    for (let i = 0; i < 8; i++) {
      const currentProgramFolder = path.join(this.rootFolder, `bank_${i}`);
      const programFile = fs
        .readdirSync(currentProgramFolder)
        .find((str) => str.match("^[0-7].*\\.spn"));

      if (programFile) {
        const programPath = path.join(currentProgramFolder, programFile);
        const outputPath = path.join(this.outputFolder, `${path.parse(programFile).name}.hex`);

        this.programs[i] = programPath;
        this.outputs[i] = outputPath;
      }
      else {
        this.programs[i] = null;
        this.outputs[i] = "";
      }
    }
  }

  /**
   * @brief Removes all `.hex` program files from the output folder.
   */
  public removeHexPrograms(): void {
    this.outputs.forEach((output) => {
      if (fs.existsSync(output)) {
        try {
          Logs.log(LogType.INFO, `Removing file: ${output}`);
          fs.unlinkSync(output);
        }
        catch (error) {
          throw new Error(`Could not remove ${output}: ${(error as Error).message}`);
        }
      }
      else {
        Logs.log(LogType.INFO, `File: ${output} does not exist`);
      }
    });
  }

  /**
   * @brief Removes the `.bin` output file from the output folder.
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
    else {
      Logs.log(LogType.INFO, `File: ${this.outputBinFile} does not exist`);
    }
  }

  /**
   * @brief Compiles a specified program to `.hex` format.
   * @param program Index of the program to compile.
   * @throws Error if the compilation fails.
   */
  public compileProgramToHex(program: number): void {
    try {
      this.removeHexPrograms();
      if (this.programs[program]) {
        this.compilerArguments.push("-p", program.toString(), this.programs[program]!, this.outputs[program]);
        const result = this.runCompiler();

        if (result !== 0) {
          throw new Error(`Compile failed with return code: ${result}`);
        }
        Logs.log(LogType.INFO, "Compile succeeded");
      }
    }
    catch (error) {
      throw new Error((error as Error).message);
    }
  }
}
