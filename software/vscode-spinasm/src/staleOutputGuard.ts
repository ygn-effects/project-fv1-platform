/** How a bank's compiled .hex compares with its .spn source on disk. */
export type OutputState = "current" | "outdated" | "missing";

export interface StaleOutputPrompt {
  message: string;
  detail: string;
  /** False when a bank has no output at all, so there is nothing to upload as is. */
  allowUploadAnyway: boolean;
}

/**
 * Describes the warning an upload-only command shows when a bank's output is
 * missing or older than its source, or returns undefined when every bank is
 * current. Pure so the wording and the offered actions can be unit-tested.
 */
export function describeStaleOutputs(states: ReadonlyMap<number, OutputState>): StaleOutputPrompt | undefined {
  const missing: number[] = [];
  const outdated: number[] = [];

  for (const [bank, state] of states) {
    if (state === "missing") {
      missing.push(bank);
    }
    else if (state === "outdated") {
      outdated.push(bank);
    }
  }

  if (missing.length === 0 && outdated.length === 0) {
    return undefined;
  }

  const details: string[] = [];
  if (missing.length > 0) {
    details.push(`Not compiled yet: ${formatBanks(missing)}.`);
  }
  if (outdated.length > 0) {
    details.push(
      `Source changed since the last compile: ${formatBanks(outdated)}. ` +
      `Uploading as is sends the previous program.`
    );
  }

  return {
    message: missing.length > 0
      ? "Some programs have no compiled output"
      : "Compiled output is older than the source",
    detail: details.join("\n"),
    allowUploadAnyway: missing.length === 0,
  };
}

function formatBanks(banks: readonly number[]): string {
  const sorted = [...banks].sort((a, b) => a - b);
  return sorted.length === 1 ? `bank ${sorted[0]}` : `banks ${sorted.join(", ")}`;
}
