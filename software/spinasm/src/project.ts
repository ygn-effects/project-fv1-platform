import * as fs from "fs";
import * as fsPromises from "fs/promises";
import * as path from "path";
import * as cp from "child_process";
import * as vscode from "vscode";
import Logs, { LogType } from "./logs";

/**
 * @class Project
 * @brief Owns workspace state and runs the compiler. Emits events for the manager
 * to refresh its cache; never reaches back into ProjectManager itself.
 */
export default class Project {
  private rootFolder: string;
  private outputFolder: string;
  private outputBinFile: string;
  private compiler: string;
  private compilerArguments: string[];
  private programs: (string | null)[];
  private outputs: string[];

  private readonly onDidCompileEmitter = new vscode.EventEmitter<number>();
  public readonly onDidCompile = this.onDidCompileEmitter.event;

  private readonly onDidChangeStructureEmitter = new vscode.EventEmitter<void>();
  public readonly onDidChangeStructure = this.onDidChangeStructureEmitter.event;

  constructor(folder: string) {
    this.rootFolder = folder;
    this.outputFolder = path.join(this.rootFolder, "output");
    this.outputBinFile = path.join(this.outputFolder, "output.bin");
    this.compiler = "";
    this.compilerArguments = [];
    this.programs = [];
    this.outputs = [];
  }

  public getRootFolder(): string {
    return this.rootFolder;
  }

  public configure(compiler: string, compilerArgs: string[]): void {
    this.compiler = compiler;
    this.compilerArguments = [...compilerArgs];

    Logs.log(LogType.INFO, `Compiler set to: ${compiler}`);
    Logs.log(LogType.INFO, `Compiler args: ${compilerArgs.join(" ")}`);
  }

  public async buildSetup(compiler: string, compilerArgs: string[]): Promise<void> {
    this.configure(compiler, compilerArgs);
    await this.scanPrograms();
  }

  public async checkCompiler(): Promise<void> {
    try {
      await fsPromises.access(this.compiler, fs.constants.X_OK);
    }
    catch {
      throw new Error(`Compiler path invalid or not executable: ${this.compiler}`);
    }

    Logs.log(LogType.INFO, `Compiler found at ${this.compiler}`);
  }

  public async createProjectStructure(): Promise<void> {
    const programContent = "; Blank SpinASM program";

    for (let i = 0; i < 8; i++) {
      const folder = path.join(this.rootFolder, `bank_${i}`);
      const file = path.join(folder, `${i}_programName.spn`);

      try {
        try {
          await fsPromises.access(folder);
        }
        catch {
          Logs.log(LogType.INFO, `Creating folder: ${folder}`);

          await fsPromises.mkdir(folder, { recursive: true });
        }

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

    try {
      await fsPromises.mkdir(this.outputFolder, { recursive: true });
    }
    catch (error) {
      if ((error as NodeJS.ErrnoException).code !== 'EEXIST') {
        throw new Error(`Could not create output folder: ${(error as Error).message}`);
      }
    }

    this.onDidChangeStructureEmitter.fire();
  }

  public async compileProgramToHex(program: number): Promise<void> {
    await this.removeHexProgram(this.outputs[program]);

    if (!this.programs[program]) {
      throw new Error(`Program at index ${program} does not exist.`);
    }

    const args = [...this.compilerArguments, "-p", program.toString(), this.programs[program]!, this.outputs[program]];
    const result = await this.runCompiler(args);

    if (result !== 0) {
      throw new Error(`Compilation failed for program ${program} with return code: ${result}`);
    }

    Logs.log(LogType.INFO, "Compilation succeeded.");

    this.onDidCompileEmitter.fire(program);
  }

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

  public async scanPrograms(): Promise<void> {
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

  public async removeHexProgram(path: string): Promise<void> {
    if (! path) {
      return;
    }

    try {
      await fsPromises.unlink(path);
    }
    catch (error) {
      if ((error as NodeJS.ErrnoException).code !== 'ENOENT') {
        throw error;
      }
    }
  }

  public getProgramBankByPath(path: string | undefined | null): number {
    if (typeof path === 'undefined' || path === null) {
      return -1;
    }

    return this.programs.indexOf(path);
  }

  public getAllPrograms(): (string | null)[] {
    return this.programs;
  }

  public getOutput(bank: number): (string | null) {
    return this.outputs[bank];
  }

  public dispose(): void {
    this.onDidCompileEmitter.dispose();
    this.onDidChangeStructureEmitter.dispose();
  }
}
