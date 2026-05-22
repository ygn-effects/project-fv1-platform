import * as vscode from "vscode";
import * as fsPromises from "fs/promises";
import Project from "./project";
import Config from "./config";
import Logs, { LogType } from "./logs";

export enum BankStatus {
  Empty = 0,        // No .spn file
  NotCompiled = 1,  // .spn exists, no .hex
  UpToDate = 2,     // .hex exists and is newer than .spn
  OutOfDate = 3     // .hex exists but .spn is newer
}

export interface CachedBankInfo {
  status: BankStatus;
  spnFile: string | null;
  hexFile: string | null;
  spnTime: Date | null;
  hexTime: Date | null;
}

/**
 * @class ProjectManager
 * @brief Owns one Project per workspace root and maintains a warm bank-status cache.
 *
 * Commands ask the manager for the Project rather than instantiating one themselves;
 * the manager subscribes to project events to keep its cache fresh.
 */
export class ProjectManager {
  private static instance: ProjectManager | null = null;

  private projectCache = new Map<string, CachedBankInfo[]>();
  private projects = new Map<string, Project>();
  private projectSubscriptions = new Map<string, vscode.Disposable[]>();

  // In-flight promises so concurrent callers share work instead of duplicating it.
  private projectPromises = new Map<string, Promise<Project | null>>();
  private refreshPromises = new Map<string, Promise<void>>();

  private onDidChangeProjectEmitter = new vscode.EventEmitter<void>();
  public readonly onDidChangeProject = this.onDidChangeProjectEmitter.event;

  private constructor() {}

  public static getInstance(): ProjectManager {
    if (!ProjectManager.instance) {
      ProjectManager.instance = new ProjectManager();
    }
    return ProjectManager.instance;
  }

  public async initializeWorkspace(): Promise<void> {
    const folders = vscode.workspace.workspaceFolders;
    if (!folders) {
      return;
    }

    const tasks = folders.map(async (folder) => {
      const rootPath = folder.uri.fsPath;
      Logs.log(LogType.INFO, `Initializing cache scan for: ${rootPath}`);
      try {
        await this.refreshProject(rootPath);
      } catch (err) {
        Logs.log(LogType.ERROR, `Failed to initialize cache scan for ${rootPath}: ${(err as Error).message}`);
      }
    });

    await Promise.all(tasks);
  }

  public hasCache(rootPath: string): boolean {
    return this.projectCache.has(rootPath);
  }

  /**
   * Returns the cached Project for a workspace, creating it on first access.
   * Concurrent callers share the same in-flight creation promise.
   */
  public getProject(rootPath: string): Promise<Project | null> {
    const existing = this.projects.get(rootPath);
    if (existing) {
      return Promise.resolve(existing);
    }

    let promise = this.projectPromises.get(rootPath);
    if (!promise) {
      promise = this.createProject(rootPath);
      this.projectPromises.set(rootPath, promise);
      promise.finally(() => this.projectPromises.delete(rootPath));
    }
    return promise;
  }

  private async createProject(rootPath: string): Promise<Project | null> {
    const compilerPath = Config.getCompilerPath();
    if (!compilerPath) {
      return null;
    }

    const project = new Project(rootPath);
    await project.buildSetup(compilerPath, Config.getCompilerArgs());

    const subs: vscode.Disposable[] = [
      project.onDidCompile((bank) => {
        this.refreshBank(rootPath, bank).catch(err => {
          Logs.log(LogType.ERROR, `Post-compile cache refresh failed: ${(err as Error).message}`);
        });
      }),
      project.onDidChangeStructure(() => {
        this.refreshProject(rootPath).catch(err => {
          Logs.log(LogType.ERROR, `Post-structure cache refresh failed: ${(err as Error).message}`);
        });
      }),
    ];

    this.projects.set(rootPath, project);
    this.projectSubscriptions.set(rootPath, subs);
    return project;
  }

  /**
   * Drops the cached Project for a workspace. Call after config changes that
   * affect compiler path/args.
   */
  public invalidate(rootPath: string): void {
    const subs = this.projectSubscriptions.get(rootPath);
    if (subs) {
      subs.forEach(d => d.dispose());
      this.projectSubscriptions.delete(rootPath);
    }
    const project = this.projects.get(rootPath);
    if (project) {
      project.dispose();
      this.projects.delete(rootPath);
    }
    this.projectCache.delete(rootPath);
  }

  public invalidateAll(): void {
    for (const rootPath of Array.from(this.projects.keys())) {
      this.invalidate(rootPath);
    }
    // Clear any caches for workspaces that were placeholder-only
    this.projectCache.clear();
  }

  /**
   * Synchronously gets cached bank information.
   * If cache is missing, returns a placeholder and triggers an async refresh.
   */
  public getBanksSync(rootPath: string): CachedBankInfo[] {
    const cached = this.projectCache.get(rootPath);
    if (cached) {
      return cached;
    }

    const placeholder: CachedBankInfo[] = Array.from({ length: 8 }, () => ({
      status: BankStatus.Empty,
      spnFile: null,
      hexFile: null,
      spnTime: null,
      hexTime: null
    }));
    this.projectCache.set(rootPath, placeholder);

    // refreshProject dedupes internally, so this won't kick off a second scan
    // if one is already in flight from initializeWorkspace.
    this.refreshProject(rootPath).catch(err => {
      Logs.log(LogType.ERROR, `Background project scan failed: ${(err as Error).message}`);
    });

    return placeholder;
  }

  /**
   * Recomputes the cache entry for a single bank using the owned Project's
   * already-scanned program list. No directory enumeration.
   */
  public async refreshBank(rootPath: string, bankIndex: number): Promise<void> {
    if (bankIndex < 0 || bankIndex >= 8) {
      return;
    }

    try {
      const project = await this.getProject(rootPath);
      if (!project) {
        return;
      }

      const spnFile = project.getAllPrograms()[bankIndex];
      const updatedInfo = await this.buildBankInfo(spnFile, project.getOutput(bankIndex));

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

      this.onDidChangeProjectEmitter.fire();
    } catch (error) {
      Logs.log(LogType.ERROR, `Error refreshing bank ${bankIndex}: ${(error as Error).message}`);
    }
  }

  /**
   * Rescans programs on disk and rebuilds the full cache entry.
   * Concurrent callers share a single in-flight refresh.
   */
  public refreshProject(rootPath: string): Promise<void> {
    const existing = this.refreshPromises.get(rootPath);
    if (existing) {
      return existing;
    }

    const promise = this.doRefreshProject(rootPath);
    this.refreshPromises.set(rootPath, promise);
    promise.finally(() => this.refreshPromises.delete(rootPath));
    return promise;
  }

  private async doRefreshProject(rootPath: string): Promise<void> {
    try {
      // If the Project already existed before this call, the on-disk structure
      // may have changed since its last scan. Rescan. If we're about to create
      // it for the first time, buildSetup will scan, so skip.
      const alreadyHadProject = this.projects.has(rootPath);

      const project = await this.getProject(rootPath);
      if (!project) {
        return;
      }

      if (alreadyHadProject) {
        await project.scanPrograms();
      }

      const programs = project.getAllPrograms();
      const tasks = Array.from({ length: 8 }, (_, i) =>
        this.buildBankInfo(programs[i], project.getOutput(i))
      );

      const updatedBanks = await Promise.all(tasks);
      this.projectCache.set(rootPath, updatedBanks);
      this.onDidChangeProjectEmitter.fire();
    } catch (error) {
      Logs.log(LogType.ERROR, `Failed to fully refresh project at ${rootPath}: ${(error as Error).message}`);
    }
  }

  private async buildBankInfo(spnFile: string | null, hexFile: string | null): Promise<CachedBankInfo> {
    if (!spnFile) {
      return {
        status: BankStatus.Empty,
        spnFile: null,
        hexFile: null,
        spnTime: null,
        hexTime: null
      };
    }

    if (!hexFile) {
      return {
        status: BankStatus.NotCompiled,
        spnFile,
        hexFile: null,
        spnTime: null,
        hexTime: null
      };
    }

    try {
      await fsPromises.access(hexFile);

      const [spnStats, hexStats] = await Promise.all([
        fsPromises.stat(spnFile),
        fsPromises.stat(hexFile)
      ]);

      const spnTime = spnStats.mtime;
      const hexTime = hexStats.mtime;

      return {
        status: hexTime >= spnTime ? BankStatus.UpToDate : BankStatus.OutOfDate,
        spnFile,
        hexFile,
        spnTime,
        hexTime
      };
    } catch {
      return {
        status: BankStatus.NotCompiled,
        spnFile,
        hexFile: hexFile,
        spnTime: null,
        hexTime: null
      };
    }
  }

  public getBankIndexFromPath(filePath: string): number {
    const match = /[\\/]bank_([0-7])[\\/]/i.exec(filePath);
    if (match) {
      return parseInt(match[1], 10);
    }
    return -1;
  }
}
