import * as fs from "fs";
import * as path from "path";
import * as cp from "child_process";
import Logs, { LogType } from "./logs";

/**
 * @class Project
 * @brief Manages project files, directories, and compilation workflow.
 */
export default class Project {
  private rootFolder: string;
  private outputFolder: string;
  private outputBinFile: string;
  private compiler: string;
  private compilerArguments: string[];
  private programs: (string | null)[];
  private outputs: string[];

  constructor(folder: string) {
    this.rootFolder = folder;
    this.outputFolder = path.join(this.rootFolder, "output");
    this.outputBinFile = path.join(this.outputFolder, "output.bin");
    this.compiler = "";
    this.compilerArguments = [];
    this.programs = [];
    this.outputs = [];
  }

  /**
   * @brief Sets the compiler configuration for the build session.
   */
  public buildSetup(compiler: string, compilerArgs: string[]): void {
    this.compiler = compiler;
    this.compilerArguments = [...compilerArgs];

    Logs.log(LogType.INFO, `Compiler set to: ${compiler}`);
    Logs.log(LogType.INFO, `Compiler args: ${compilerArgs.join(" ")}`);

    this.getAvailablePrograms();
  }

  /**
   * @brief Validates the compiler's functionality.
   */
  public checkCompiler(): void {
    if (!this.compiler || !fs.existsSync(this.compiler)) {
      throw new Error(`Compiler path invalid or unset: ${this.compiler}`);
    }

    Logs.log(LogType.INFO, `Compiler found at ${this.compiler}`);
  }

  /**
   * @brief Creates project bank structure (folders 0-7).
   */
  public createProjectStructure(): void {
    const programContent = "; Blank SpinASM program";

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
      }
      catch (error) {
        throw new Error(`Error setting up bank ${i}: ${(error as Error).message}`);
      }
    }

    // Create output directory
    try {
      if (!fs.existsSync(this.outputFolder)) {
        fs.mkdirSync(this.outputFolder, { recursive: true });
      }
    }
    catch (error) {
      throw new Error(`Could not create output folder: ${(error as Error).message}`);
    }
  }

  /**
   * @brief Compiles the specified program index into a `.hex` file.
   */
  public compileProgramToHex(program: number): void {
    this.removeHexProgram(this.outputs[program]);

    if (!this.programs[program]) {
      throw new Error(`Program at index ${program} does not exist.`);
    }

    // Clone args to avoid mutating the class state permanently
    const args = [...this.compilerArguments, "-p", program.toString(), this.programs[program]!, this.outputs[program]];

    const result = this.runCompiler(args);

    if (result !== 0) {
      throw new Error(`Compilation failed for program ${program} with return code: ${result}`);
    }

    Logs.log(LogType.INFO, "Compilation succeeded.");
  }

  /**
   * @brief Compiles the specified program index into a `.bin` file.
   */
  public compileProgramToBin(program: number): void {
    if (!this.programs[program]) {
      throw new Error(`Program at index ${program} does not exist.`);
    }

    const args = [...this.compilerArguments, "-p", program.toString(), this.programs[program]!, this.outputBinFile];

    const result = this.runCompiler(args);
    if (result !== 0) {
      throw new Error(`Compilation failed for program ${program} with return code: ${result}`);
    }

    Logs.log(LogType.INFO, "Compilation succeeded.");
  }

  /**
   * @brief Runs the compiler process synchronously.
   */
  private runCompiler(args: string[]): number {
    Logs.log(LogType.INFO, `Running: ${this.compiler} ${args.join(" ")}`);

    const output = cp.spawnSync(this.compiler, args, { encoding: "utf8" });

    if (output.stdout) {
      Logs.log(LogType.INFO, `Compiler stdout: ${output.stdout}`);
    }

    if (output.stderr) {
      // Differentiate between actual errors and warnings based on return code
      if (output.status === 0) {
        Logs.log(LogType.INFO, `Compiler warning: ${output.stderr}`);
      }
      else {
        Logs.log(LogType.ERROR, `Compiler stderr: ${output.stderr}`);
      }
    }

    return output.status ?? 1;
  }

  private getAvailablePrograms(): void {
    this.programs = [];
    this.outputs = [];

    for (let i = 0; i < 8; i++) {
      const currentFolder = path.join(this.rootFolder, `bank_${i}`);

      if(fs.existsSync(currentFolder)) {
        const programFile = fs.readdirSync(currentFolder).find(file => /^[0-7].*\.spn$/.test(file));

        if (programFile) {
          this.programs[i] = path.join(currentFolder, programFile);
          this.outputs[i] = path.join(this.outputFolder, `${path.parse(programFile).name}.hex`);

          continue;
        }
      }

      this.programs[i] = null;
      this.outputs[i] = "";
    }
  }

  public removeHexProgram(path: any): void {
    if (fs.existsSync(path)) {
      fs.unlinkSync(path);
    }
  }

  public getProgramBankByPath(path: string | undefined | null): number {
    if (typeof path === 'undefined' || path === null) {
      throw new Error("Invalid file path");
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
