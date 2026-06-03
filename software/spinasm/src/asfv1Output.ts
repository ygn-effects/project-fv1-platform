/**
 * Parses asfv1 stderr into structured problems. asfv1 emits one problem per
 * line in these forms (1-based line number, no column, no filename):
 *
 *   parse error: <message> on line <N>
 *   scan error:  <message> on line <N>
 *   warning:     <message> on line <N>
 *
 * The trailing "on line N" is optional (a few messages aren't line-attributed).
 * Anything else on stderr/stdout (e.g. "info: ...") is ignored. Pure and
 * dependency-free so the format handling can be unit-tested.
 */

export type CompilerProblemSeverity = "error" | "warning";

export interface CompilerProblem {
  /** 1-based line from asfv1, or null when the message isn't line-attributed. */
  line: number | null;
  severity: CompilerProblemSeverity;
  message: string;
}

const PROBLEM_RE =
  /^(parse error|scan error|error|warning):\s*(.*?)(?:\s+on line\s+(\d+))?\.?\s*$/i;

// asfv1 prints this once on every run under -s (SpinASM real-literal mode),
// which the extension enables by default. It's an informational notice — e.g.
// "SpinASM compatibility - literals 2,1 read as 2.0,1.0" — not a problem.
const SPINASM_COMPAT_NOTICE = /^SpinASM compatibility\b.*\bliterals\b/i;

export function parseAsfv1Output(output: string): CompilerProblem[] {
  const problems: CompilerProblem[] = [];

  for (const rawLine of output.split(/\r?\n/)) {
    const line = rawLine.trim();
    if (!line) {
      continue;
    }

    const match = PROBLEM_RE.exec(line);
    if (!match) {
      continue;
    }

    const kind = match[1].toLowerCase();
    const message = match[2].trim();
    if (!message) {
      continue;
    }

    const severity: CompilerProblemSeverity = kind === "warning" ? "warning" : "error";

    if (severity === "warning" && SPINASM_COMPAT_NOTICE.test(message)) {
      continue;
    }

    problems.push({
      line: match[3] ? parseInt(match[3], 10) : null,
      severity,
      message,
    });
  }

  return problems;
}
