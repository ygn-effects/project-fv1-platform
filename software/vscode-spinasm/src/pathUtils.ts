import * as path from "path";

/**
 * Compares two filesystem paths, tolerant of separator and `.`/`..` differences
 * and — on Windows — drive-letter / casing differences. VS Code's `uri.fsPath`
 * and our `path.join`-built program paths can disagree on drive-letter case
 * (e.g. `c:\` vs `C:\`), which would make an exact string match miss.
 *
 * Case-sensitive off Windows, since POSIX filesystems are case-sensitive.
 */
export function pathsEqual(a: string, b: string): boolean {
  const na = path.resolve(a);
  const nb = path.resolve(b);
  return process.platform === "win32"
    ? na.toLowerCase() === nb.toLowerCase()
    : na === nb;
}
