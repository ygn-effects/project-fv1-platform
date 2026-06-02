import * as fs from "fs";
import * as fsPromises from "fs/promises";
import * as path from "path";
import * as cp from "child_process";
import * as vscode from "vscode";
import Logs, { LogType } from "./logs";
import { BANK_COUNT, BANK_SIZE_BYTES, EEPROM_SIZE_BYTES } from "./fv1Constants";

/**
 * Compares two filesystem paths, tolerant of separator and `.`/`..` differences
 * and — on Windows — drive-letter / casing differences. VS Code's `uri.fsPath`
 * and our `path.join`-built program paths can disagree on drive-letter case
 * (e.g. `c:\` vs `C:\`), which would make an exact string match miss.
 */
function pathsEqual(a: string, b: string): boolean {
  const na = path.resolve(a);
  const nb = path.resolve(b);
  return process.platform === "win32"
    ? na.toLowerCase() === nb.toLowerCase()
    : na === nb;
}

/**
 * Owns workspace state and runs the compiler. Emits events for the manager
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
  // Per bank: extra .spn files found in the folder beyond the chosen program.
  // Non-empty means the bank is ambiguous and we picked one deterministically.
  private programExtras: string[][];

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
    this.programExtras = [];
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
    for (let i = 0; i < BANK_COUNT; i++) {
      const folder = path.join(this.rootFolder, `bank_${i}`);
      const file = path.join(folder, "program.spn");

      // One .spn per bank_<N>/ folder; the filename is free-form because the
      // bank is decided by the folder, not the name.
      const programContent = `; Bank ${i} program — rename this file freely.\n`;

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

  /**
   * Builds a single EEPROM_SIZE_BYTES image by compiling each present bank
   * with `asfv1 -p N` and merging.
   *
   * asfv1 emits `(N+1) * BANK_SIZE_BYTES` for `-p N`: banks 0..(N-1) zero-padded,
   * the assembled program at offset `N * BANK_SIZE_BYTES`. We extract just the
   * bank-N slice from each output. Missing banks remain `0x00` to match asfv1's
   * own padding convention.
   */
  public async compileAllProgramsToCombinedBin(): Promise<void> {
    const eeprom = Buffer.alloc(EEPROM_SIZE_BYTES, 0x00);
    const tempFiles: string[] = [];

    try {
      for (let bank = 0; bank < BANK_COUNT; bank++) {
        const programPath = this.programs[bank];
        if (!programPath) {
          continue;
        }

        const tempBin = path.join(this.outputFolder, `.bank_${bank}.bin`);
        tempFiles.push(tempBin);

        const args = [...this.compilerArguments, "-p", bank.toString(), programPath, tempBin];
        const result = await this.runCompiler(args);

        if (result !== 0) {
          throw new Error(`Compilation failed for bank ${bank} with return code: ${result}`);
        }

        const bankBytes = await fsPromises.readFile(tempBin);
        const expected = (bank + 1) * BANK_SIZE_BYTES;
        if (bankBytes.length !== expected) {
          throw new Error(`Bank ${bank} compiled to ${bankBytes.length} bytes, expected ${expected}`);
        }

        bankBytes.copy(eeprom, bank * BANK_SIZE_BYTES, bank * BANK_SIZE_BYTES, expected);
      }

      await fsPromises.writeFile(this.outputBinFile, eeprom);
      Logs.log(LogType.INFO, `Combined EEPROM image written: ${this.outputBinFile} (${EEPROM_SIZE_BYTES} bytes)`);
    } finally {
      // Clean up temp bank files even on failure.
      await Promise.all(tempFiles.map(async (f) => {
        try {
          await fsPromises.unlink(f);
        } catch {
          // already gone or never created
        }
      }));
    }
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
    this.programExtras = [];

    for (let i = 0; i < BANK_COUNT; i++) {
      const currentFolder = path.join(this.rootFolder, `bank_${i}`);

      this.programs[i] = null;
      this.outputs[i] = "";
      this.programExtras[i] = [];

      try {
        await fsPromises.access(currentFolder);
        const files = await fsPromises.readdir(currentFolder);

        // Any .spn in the folder is a candidate — the bank is decided by the
        // folder, not the filename. Sort for a stable choice and keep the rest
        // as "extras" so an ambiguous folder is surfaced, not silently resolved.
        const candidates = files
          .filter(file => /\.spn$/i.test(file))
          .sort((a, b) => a.localeCompare(b));

        if (candidates.length === 0) {
          continue;
        }

        const [chosen, ...extras] = candidates;
        this.programs[i] = path.join(currentFolder, chosen);
        this.outputs[i] = path.join(this.outputFolder, `bank_${i}.hex`);
        this.programExtras[i] = extras.map(file => path.join(currentFolder, file));

        if (extras.length > 0) {
          Logs.log(
            LogType.WARNING,
            `Bank ${i}: ${candidates.length} .spn files in bank_${i}/ — using ` +
            `"${chosen}", ignoring ${extras.map(e => `"${e}"`).join(", ")}.`
          );
        }
      }
      catch {
        // folder doesn't exist or can't be read; treat as empty bank
      }
    }
  }

  public getExtraPrograms(bank: number): string[] {
    return this.programExtras[bank] ?? [];
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

  public getProgramBankByPath(filePath: string | undefined | null): number {
    if (filePath === undefined || filePath === null) {
      return -1;
    }

    return this.programs.findIndex(program => program !== null && pathsEqual(program, filePath));
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
