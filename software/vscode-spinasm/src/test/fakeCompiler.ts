import * as fs from "fs/promises";
import * as path from "path";
import { BANK_SIZE_BYTES } from "../fv1Constants";

/**
 * A stand-in for asfv1 so compile paths can be tested without it installed.
 *
 * It runs on every platform the tests run on: the compiler is the test host's
 * own binary, which the extension host starts with ELECTRON_RUN_AS_NODE=1 (so
 * its children run as plain Node), and the first compiler argument is a small
 * script. Called like asfv1 (`[...args] -p N source output`), the script:
 *
 * - records its arguments, one JSON array per line, in `invocations.log`;
 * - fails like asfv1 (parse error on stderr, exit 2) when the source contains
 *   `FAIL`, and warns on stderr when it contains `WARN`;
 * - writes `(N + 1) * BANK_SIZE_BYTES` bytes for a `.bin` output, zero-padded
 *   like asfv1, with bank N's slice filled with the byte N + 1;
 * - otherwise writes a placeholder text for a `.hex` output.
 */
export interface FakeCompiler {
  /** Value for `Project.configure`'s compiler. */
  compiler: string;
  /** Value for `Project.configure`'s arguments; extra asfv1 flags go after it. */
  args: string[];
  /** Arguments of each run so far, without the script path. */
  invocations(): Promise<string[][]>;
}

export async function createFakeCompiler(dir: string): Promise<FakeCompiler> {
  const script = path.join(dir, "fake-asfv1.js");
  const log = path.join(dir, "invocations.log");

  await fs.writeFile(script, `
const fs = require("fs");
const args = process.argv.slice(2);
fs.appendFileSync(${JSON.stringify(log)}, JSON.stringify(args) + "\\n");

const bank = Number(args[args.indexOf("-p") + 1]);
const [source, output] = args.slice(-2);
const text = fs.readFileSync(source, "utf8");

if (text.includes("FAIL")) {
  process.stderr.write("parse error: Unexpected FAIL on line 1\\n");
  process.exit(2);
}
if (text.includes("WARN")) {
  process.stderr.write("warning: Something odd on line 1\\n");
}

if (output.endsWith(".bin")) {
  const image = Buffer.alloc((bank + 1) * ${BANK_SIZE_BYTES}, 0);
  image.fill(bank + 1, bank * ${BANK_SIZE_BYTES});
  fs.writeFileSync(output, image);
} else {
  fs.writeFileSync(output, "fake hex for bank " + bank + "\\n");
}
`);

  return {
    compiler: process.execPath,
    args: [script],
    async invocations() {
      try {
        const lines = (await fs.readFile(log, "utf8")).trim().split("\n");
        return lines.map(line => JSON.parse(line) as string[]);
      }
      catch {
        return [];
      }
    },
  };
}
