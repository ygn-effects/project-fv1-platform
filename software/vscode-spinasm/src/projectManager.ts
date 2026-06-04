import * as vscode from "vscode";
import * as fsPromises from "fs/promises";
import Project from "./project";
import Config from "./config";
import Logs, { LogType } from "./logs";
import { BANK_COUNT } from "./fv1Constants";

export enum BankStatus {
  Empty = 0,
  NotCompiled = 1,
  UpToDate = 2,     // .hex exists and is newer than .spn
  OutOfDate = 3     // .hex exists but .spn is newer
}

export interface CachedBankInfo {
  status: BankStatus;
  spnFile: string | null;
  hexFile: string | null;
  spnTime: Date | null;
  hexTime: Date | null;
  /** Extra .spn files in the bank folder beyond the chosen program; non-empty = ambiguous. */
  extraFiles: string[];
}

/**
 * Owns one Project per workspace root and keeps a warm bank-status cache.
 * Commands ask the manager for the Project rather than instantiating one;
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

  // Forwarded from each owned Project so the extension can surface asfv1 output
  // as diagnostics without holding project references itself.
  private onDidProduceCompilerOutputEmitter = new vscode.EventEmitter<{ sourcePath: string; stderr: string }>();
  public readonly onDidProduceCompilerOutput = this.onDidProduceCompilerOutputEmitter.event;

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

  /** Returns the cached Project for a workspace, creating it on first access. */
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
      project.onCompilerOutput((e) => this.onDidProduceCompilerOutputEmitter.fire(e)),
    ];

    this.projects.set(rootPath, project);
    this.projectSubscriptions.set(rootPath, subs);
    return project;
  }

  /** Drops the cached Project. Call after compiler config changes. */
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
    // Also drop placeholder-only entries for workspaces we never fully scanned.
    this.projectCache.clear();
  }

  /** Returns cached bank info, kicking off a background scan on cache miss. */
  public getBanksSync(rootPath: string): CachedBankInfo[] {
    const cached = this.projectCache.get(rootPath);
    if (cached) {
      return cached;
    }

    const placeholder: CachedBankInfo[] = Array.from({ length: BANK_COUNT }, () => ({
      status: BankStatus.Empty,
      spnFile: null,
      hexFile: null,
      spnTime: null,
      hexTime: null,
      extraFiles: []
    }));
    this.projectCache.set(rootPath, placeholder);

    // refreshProject dedupes in-flight, so this is safe even when
    // initializeWorkspace already kicked off a scan.
    this.refreshProject(rootPath).catch(err => {
      Logs.log(LogType.ERROR, `Background project scan failed: ${(err as Error).message}`);
    });

    return placeholder;
  }

  /** Recomputes one bank's cache entry from the owned Project's scanned list — no directory I/O. */
  public async refreshBank(rootPath: string, bankIndex: number): Promise<void> {
    if (bankIndex < 0 || bankIndex >= BANK_COUNT) {
      return;
    }

    try {
      const project = await this.getProject(rootPath);
      if (!project) {
        return;
      }

      const spnFile = project.getAllPrograms()[bankIndex];
      const updatedInfo = await this.buildBankInfo(
        spnFile,
        project.getOutput(bankIndex),
        project.getExtraPrograms(bankIndex)
      );

      let cachedArray = this.projectCache.get(rootPath);
      if (!cachedArray) {
        cachedArray = Array.from({ length: BANK_COUNT }, () => ({
          status: BankStatus.Empty,
          spnFile: null,
          hexFile: null,
          spnTime: null,
          hexTime: null,
          extraFiles: []
        }));
      }

      cachedArray[bankIndex] = updatedInfo;
      this.projectCache.set(rootPath, cachedArray);

      this.onDidChangeProjectEmitter.fire();
    } catch (error) {
      Logs.log(LogType.ERROR, `Error refreshing bank ${bankIndex}: ${(error as Error).message}`);
    }
  }

  /** Rescans programs on disk and rebuilds the full cache entry. Dedupes in-flight. */
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
      // If a Project already existed, the on-disk structure may have changed
      // since its last scan, so we rescan. First-time creation runs buildSetup
      // which scans, so we skip the extra pass there.
      const alreadyHadProject = this.projects.has(rootPath);

      const project = await this.getProject(rootPath);
      if (!project) {
        return;
      }

      if (alreadyHadProject) {
        await project.scanPrograms();
      }

      const programs = project.getAllPrograms();
      const tasks = Array.from({ length: BANK_COUNT }, (_, i) =>
        this.buildBankInfo(programs[i], project.getOutput(i), project.getExtraPrograms(i))
      );

      const updatedBanks = await Promise.all(tasks);
      this.projectCache.set(rootPath, updatedBanks);
      this.onDidChangeProjectEmitter.fire();
    } catch (error) {
      Logs.log(LogType.ERROR, `Failed to fully refresh project at ${rootPath}: ${(error as Error).message}`);
    }
  }

  private async buildBankInfo(
    spnFile: string | null,
    hexFile: string | null,
    extraFiles: string[] = []
  ): Promise<CachedBankInfo> {
    if (!spnFile) {
      return {
        status: BankStatus.Empty,
        spnFile: null,
        hexFile: null,
        spnTime: null,
        hexTime: null,
        extraFiles
      };
    }

    if (!hexFile) {
      return {
        status: BankStatus.NotCompiled,
        spnFile,
        hexFile: null,
        spnTime: null,
        hexTime: null,
        extraFiles
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
        hexTime,
        extraFiles
      };
    } catch {
      return {
        status: BankStatus.NotCompiled,
        spnFile,
        hexFile: hexFile,
        spnTime: null,
        hexTime: null,
        extraFiles
      };
    }
  }

  /** Returns the bank index from a path like `.../bank_3/...`, or -1 if unmatched. */
  public getBankIndexFromPath(filePath: string): number {
    const match = /[\\/]bank_([0-7])[\\/]/i.exec(filePath);
    if (match) {
      return parseInt(match[1], 10);
    }
    return -1;
  }
}
