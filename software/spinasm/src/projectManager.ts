import * as vscode from "vscode";
import * as path from "path";
import * as fsPromises from "fs/promises";
import * as fs from "fs";
import Project from "./project";
import Config from "./config";
import Logs, { LogType } from "./logs";

/**
 * @enum BankStatus
 * @brief Represents the compilation status of a bank
 */
export enum BankStatus {
  Empty = 0,        // No .spn file
  NotCompiled = 1,  // .spn exists, no .hex
  UpToDate = 2,     // .hex exists and is newer than .spn
  OutOfDate = 3     // .hex exists but .spn is newer
}

/**
 * @interface CachedBankInfo
 * @brief Information about a single bank stored in memory
 */
export interface CachedBankInfo {
  status: BankStatus;
  spnFile: string | null;
  hexFile: string | null;
  spnTime: Date | null;
  hexTime: Date | null;
}

/**
 * @class ProjectManager
 * @brief Centralized service that maintains a warm in-memory cache of project bank states.
 * * Prevents redundant disc-bound traversals when changing active editor tabs.
 * Uses a file system watcher to reactively refresh specific banks.
 */
export class ProjectManager {
  private static instance: ProjectManager | null = null;

  // Cache storage mapping: workspaceRoot -> array of 8 CachedBankInfos
  private projectCache = new Map<string, CachedBankInfo[]>();

  // Event emitter to notify the status bar or diagnostics of cache updates
  private onDidChangeProjectEmitter = new vscode.EventEmitter<void>();
  public readonly onDidChangeProject = this.onDidChangeProjectEmitter.event;

  private constructor() {}

  /**
   * @brief Gets the singleton instance of the ProjectManager
   */
  public static getInstance(): ProjectManager {
    if (!ProjectManager.instance) {
      ProjectManager.instance = new ProjectManager();
    }
    return ProjectManager.instance;
  }

  /**
   * @brief Initializes project scanning for workspace folders.
   */
  public async initializeWorkspace(): Promise<void> {
    const folders = vscode.workspace.workspaceFolders;
    if (!folders) {
      return;
    }

    for (const folder of folders) {
      const rootPath = folder.uri.fsPath;
      Logs.log(LogType.INFO, `Initializing cache scan for: ${rootPath}`);
      await this.refreshProject(rootPath);
    }
  }

  /**
   * @brief Checks if project cache exists for the specified root path.
   */
  public hasCache(rootPath: string): boolean {
    return this.projectCache.has(rootPath);
  }

  /**
   * @brief Synchronously gets cached bank information.
   * If cache is missing, triggers an asynchronous scan in the background.
   */
  public getBanksSync(rootPath: string): CachedBankInfo[] {
    if (!this.projectCache.has(rootPath)) {
      // Return a temporary blank list and trigger a background refresh
      const placeholder: CachedBankInfo[] = Array.from({ length: 8 }, () => ({
        status: BankStatus.Empty,
        spnFile: null,
        hexFile: null,
        spnTime: null,
        hexTime: null
      }));
      this.projectCache.set(rootPath, placeholder);

      this.refreshProject(rootPath).then(() => {
        this.onDidChangeProjectEmitter.fire();
      }).catch(err => {
        Logs.log(LogType.ERROR, `Background project scan failed: ${err.message}`);
      });

      return placeholder;
    }

    return this.projectCache.get(rootPath)!;
  }

  /**
   * @brief Perform a lightweight background check on a single target bank.
   * Highly useful to call on individual file saves or compilation successes.
   */
  public async refreshBank(rootPath: string, bankIndex: number): Promise<void> {
    if (bankIndex < 0 || bankIndex >= 8) {
      return;
    }

    try {
      const project = new Project(rootPath);
      const compilerPath = Config.getCompilerPath();
      const compilerArgs = Config.getCompilerArgs();

      if (!compilerPath) {
        return;
      }

      await project.buildSetup(compilerPath, compilerArgs);
      const programs = project.getAllPrograms();
      const spnFile = programs[bankIndex];

      // Default empty state
      let updatedInfo: CachedBankInfo = {
        status: BankStatus.Empty,
        spnFile: null,
        hexFile: null,
        spnTime: null,
        hexTime: null
      };

      if (spnFile) {
        const hexFile = project.getOutput(bankIndex);
        if (hexFile) {
          try {
            await fsPromises.access(hexFile);

            const [spnStats, hexStats] = await Promise.all([
              fsPromises.stat(spnFile),
              fsPromises.stat(hexFile)
            ]);

            const spnTime = spnStats.mtime;
            const hexTime = hexStats.mtime;

            updatedInfo = {
              status: hexTime >= spnTime ? BankStatus.UpToDate : BankStatus.OutOfDate,
              spnFile,
              hexFile,
              spnTime,
              hexTime
            };
          } catch {
            // Hex file missing or inaccessible
            updatedInfo = {
              status: BankStatus.NotCompiled,
              spnFile,
              hexFile,
              spnTime: null,
              hexTime: null
            };
          }
        } else {
          updatedInfo = {
            status: BankStatus.NotCompiled,
            spnFile,
            hexFile: null,
            spnTime: null,
            hexTime: null
          };
        }
      }

      // Merge into workspace array in cache
      let cachedArray = this.projectCache.get(rootPath);
      if (!cachedArray) {
        cachedArray = Array.from({ length: 8 }, () => ({
          status: BankStatus.Empty,
          spnFile: null,
          hexFile: null,
          spnTime: null,
          hexTime: null
        }));
      }

      cachedArray[bankIndex] = updatedInfo;
      this.projectCache.set(rootPath, cachedArray);

      // Fire notification to trigger responsive UI updates
      this.onDidChangeProjectEmitter.fire();
    } catch (error) {
      Logs.log(LogType.ERROR, `Error refreshing bank ${bankIndex}: ${(error as Error).message}`);
    }
  }

  /**
   * @brief Refreshes all 8 banks for the specified root path.
   * Executed when a project is first loaded, structured, or configured.
   */
  public async refreshProject(rootPath: string): Promise<void> {
    try {
      const project = new Project(rootPath);
      const compilerPath = Config.getCompilerPath();
      const compilerArgs = Config.getCompilerArgs();

      if (!compilerPath) {
        return;
      }

      await project.buildSetup(compilerPath, compilerArgs);
      const programs = project.getAllPrograms();
      const updatedBanks: CachedBankInfo[] = [];

      for (let i = 0; i < 8; i++) {
        const spnFile = programs[i];

        if (!spnFile) {
          updatedBanks.push({
            status: BankStatus.Empty,
            spnFile: null,
            hexFile: null,
            spnTime: null,
            hexTime: null
          });
          continue;
        }

        const hexFile = project.getOutput(i);

        if (!hexFile) {
          updatedBanks.push({
            status: BankStatus.NotCompiled,
            spnFile,
            hexFile: null,
            spnTime: null,
            hexTime: null
          });
          continue;
        }

        try {
          await fsPromises.access(hexFile);

          const [spnStats, hexStats] = await Promise.all([
            fsPromises.stat(spnFile),
            fsPromises.stat(hexFile)
          ]);

          const spnTime = spnStats.mtime;
          const hexTime = hexStats.mtime;

          updatedBanks.push({
            status: hexTime >= spnTime ? BankStatus.UpToDate : BankStatus.OutOfDate,
            spnFile,
            hexFile,
            spnTime,
            hexTime
          });
        } catch {
          updatedBanks.push({
            status: BankStatus.NotCompiled,
            spnFile,
            hexFile,
            spnTime: null,
            hexTime: null
          });
        }
      }

      this.projectCache.set(rootPath, updatedBanks);
      this.onDidChangeProjectEmitter.fire();
    } catch (error) {
      Logs.log(LogType.ERROR, `Failed to fully refresh project at ${rootPath}: ${(error as Error).message}`);
    }
  }

  /**
   * @brief Helper to resolve bank index from file path based on folder naming conventions.
   * Matches `bank_([0-7])` structures instantly without doing disk reads.
   */
  public getBankIndexFromPath(filePath: string): number {
    const match = /[\\/]bank_([0-7])[\\/]/i.exec(filePath);
    if (match) {
      return parseInt(match[1], 10);
    }
    return -1;
  }
}
