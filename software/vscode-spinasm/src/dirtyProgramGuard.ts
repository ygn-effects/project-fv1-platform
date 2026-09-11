import { pathsEqual } from "./pathUtils";

export interface OpenDocumentState {
  fsPath: string;
  isDirty: boolean;
}

/** Returns project program paths whose open editor contains unsaved changes. */
export function findDirtyProgramPaths(
  programPaths: readonly (string | null)[],
  openDocuments: readonly OpenDocumentState[],
): string[] {
  const dirtyDocuments = openDocuments.filter(document => document.isDirty);
  const dirtyPrograms: string[] = [];

  for (const programPath of programPaths) {
    if (programPath === null || dirtyPrograms.some(path => pathsEqual(path, programPath))) {
      continue;
    }

    if (dirtyDocuments.some(document => pathsEqual(document.fsPath, programPath))) {
      dirtyPrograms.push(programPath);
    }
  }

  return dirtyPrograms;
}
