import * as fs from "fs";
import * as fsPromises from "fs/promises";
import * as path from "path";

export interface ExecutableLookupOptions {
  /** Defaults to `process.env.PATH`. */
  pathEnv?: string;
  /** Defaults to `process.platform`. */
  platform?: NodeJS.Platform;
}

/**
 * Resolves a compiler setting the way `child_process.spawn` will run it:
 * a value containing a path separator is used as a path, a bare name such as
 * `asfv1` is searched for on PATH. Returns the executable's path, or null when
 * nothing runnable is found.
 *
 * On Windows only `.exe` is tried for a bare name: pip installs console
 * scripts as `.exe` launchers, and a legacy `.com` of the same name earlier on
 * PATH shouldn't be picked instead.
 *
 * Takes PATH and platform as options so the lookup can be unit-tested.
 */
export async function findExecutable(
  command: string,
  options: ExecutableLookupOptions = {}
): Promise<string | null> {
  const platform = options.platform ?? process.platform;
  const isWindows = platform === "win32";

  if (!command) {
    return null;
  }

  const hasSeparator = command.includes("/") || (isWindows && command.includes("\\"));
  if (hasSeparator) {
    return (await isExecutable(command)) ? command : null;
  }

  const pathEnv = options.pathEnv ?? process.env.PATH ?? "";
  const directories = pathEnv.split(isWindows ? ";" : ":").filter(dir => dir.length > 0);

  const hasExtension = path.extname(command).length > 0;
  const fileName = isWindows && !hasExtension ? `${command}.exe` : command;

  for (const directory of directories) {
    const fullPath = path.join(directory, fileName);
    if (await isExecutable(fullPath)) {
      return fullPath;
    }
  }

  return null;
}

async function isExecutable(filePath: string): Promise<boolean> {
  try {
    const stats = await fsPromises.stat(filePath);
    if (!stats.isFile()) {
      return false;
    }
    // X_OK only checks existence on Windows, which is the best available there.
    await fsPromises.access(filePath, fs.constants.X_OK);
    return true;
  }
  catch {
    return false;
  }
}
