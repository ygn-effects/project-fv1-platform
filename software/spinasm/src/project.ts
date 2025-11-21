import * as fs from "fs";
import * as fsPromises from "fs/promises";
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
  public async buildSetup(compiler: string, compilerArgs: string[]): Promise<void> {
    this.compiler = compiler;
    this.compilerArguments = [...compilerArgs];

    Logs.log(LogType.INFO, `Compiler set to: ${compiler}`);
    Logs.log(LogType.INFO, `Compiler args: ${compilerArgs.join(" ")}`);

    await this.getAvailablePrograms();
  }

  /**
   * @brief Validates the compiler's functionality.
   */
  public async checkCompiler(): Promise<void> {
    try {
      await fsPromises.access(this.compiler, fs.constants.X_OK);
    }
    catch {
      throw new Error(`Compiler path invalid or not executable: ${this.compiler}`);
    }

    Logs.log(LogType.INFO, `Compiler found at ${this.compiler}`);
  }

  /**
   * @brief Creates project bank structure (folders 0-7).
   */
  public async createProjectStructure(): Promise<void> {
    const programContent = "; Blank SpinASM program";

    // Create bank folders and default programs
    for (let i = 0; i < 8; i++) {
      const folder = path.join(this.rootFolder, `bank_${i}`);
      const file = path.join(folder, `${i}_programName.spn`);

      try {
        // Check if folder exists
        try {
          await fsPromises.access(folder);
        }
        catch {
          Logs.log(LogType.INFO, `Creating folder: ${folder}`);

          await fsPromises.mkdir(folder, { recursive: true });
        }

        // Check if file exists
        try {
          await fsPromises.access(file);
        }
        catch {
          Logs.log(LogType.INFO, `Creating file: ${file}`);

          await fsPromises.writeFile(file, programContent);
        }
      }
      catch (error) {
        throw new Error(`Error setting up bank ${i}: ${(error as Error).message}`);
      }
    }

    // Create output directory
    try {
      await fsPromises.mkdir(this.outputFolder, { recursive: true });
    }
    catch (error) {
      if ((error as NodeJS.ErrnoException).code !== 'EEXIST') {
        throw new Error(`Could not create output folder: ${(error as Error).message}`);
      }
    }
  }

  /**
   * @brief Compiles the specified program index into a `.hex` file.
   */
  public async compileProgramToHex(program: number): Promise<void> {
    await this.removeHexProgram(this.outputs[program]);

    if (!this.programs[program]) {
      throw new Error(`Program at index ${program} does not exist.`);
    }

    // Clone args to avoid mutating the class state permanently
    const args = [...this.compilerArguments, "-p", program.toString(), this.programs[program]!, this.outputs[program]];
    const result = await this.runCompiler(args);

    if (result !== 0) {
      throw new Error(`Compilation failed for program ${program} with return code: ${result}`);
    }

    Logs.log(LogType.INFO, "Compilation succeeded.");
  }

  /**
   * @brief Compiles the specified program index into a `.bin` file.
   */
  public async compileProgramToBin(program: number): Promise<void> {
    if (!this.programs[program]) {
      throw new Error(`Program at index ${program} does not exist.`);
    }

    const args = [...this.compilerArguments, "-p", program.toString(), this.programs[program]!, this.outputBinFile];
    const result = await this.runCompiler(args);

    if (result !== 0) {
      throw new Error(`Compilation failed for program ${program} with return code: ${result}`);
    }

    Logs.log(LogType.INFO, "Compilation succeeded.");
  }

  /**
   * @brief Runs the compiler process asynchronously.
   */
  private runCompiler(args: string[]): Promise<number> {
    Logs.log(LogType.INFO, `Running: ${this.compiler} ${args.join(" ")}`);

    return new Promise((resolve, reject) => {
      const process = cp.spawn(this.compiler, args);

      let stderr = "";
      let stdout = "";

      if (process.stdout) {
        process.stdout.on('data', (data) => {
          stdout += data.toString();
        });
      }

      if (process.stderr) {
        process.stderr.on('data', (data) => {
          stderr += data.toString();
        });
      }

      process.on('error', (err) => {
        Logs.log(LogType.ERROR, `Failed to start compiler: ${err.message}`);
        reject(err);
      });

      process.on('close', (code) => {
        if (stdout) {
          Logs.log(LogType.INFO, `Compiler stdout: ${stdout}`);
        }

        if (stderr) {
          if (code === 0) {
            Logs.log(LogType.INFO, `Compiler warning: ${stderr}`);
          }
          else {
            Logs.log(LogType.ERROR, `Compiler stderr: ${stderr}`);
          }
        }

        resolve(code ?? 1);
      });
    });
  }

  private async getAvailablePrograms(): Promise<void> {
    this.programs = [];
    this.outputs = [];

    for (let i = 0; i < 8; i++) {
      const currentFolder = path.join(this.rootFolder, `bank_${i}`);

      try {
        await fsPromises.access(currentFolder);
        const files = await fsPromises.readdir(currentFolder);
        const programFile = files.find(file => /^[0-7].*\.spn$/.test(file));

        if (programFile) {
          this.programs[i] = path.join(currentFolder, programFile);
          this.outputs[i] = path.join(this.outputFolder, `${path.parse(programFile).name}.hex`);
          continue;
        }
      }
      catch {
        // Folder doesn't exist or can't be read
      }

      this.programs[i] = null;
      this.outputs[i] = "";
    }
  }

  public async removeHexProgram(path: any): Promise<void> {
    if (! path) {
      return;
    }

    try {
      await fsPromises.unlink(path);
    }
    catch (error) {
      // Ignore if file missing
      if ((error as NodeJS.ErrnoException).code !== 'ENOENT') {
        throw error;
      }
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
