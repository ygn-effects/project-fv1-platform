import * as assert from "assert";
import * as fs from "fs/promises";
import * as os from "os";
import * as path from "path";
import { findExecutable } from "../executableLookup";

describe("findExecutable", () => {
  let binA: string;
  let binB: string;

  beforeEach(async () => {
    const root = await fs.mkdtemp(path.join(os.tmpdir(), "spinasm-exe-"));
    binA = path.join(root, "a");
    binB = path.join(root, "b");
    await fs.mkdir(binA);
    await fs.mkdir(binB);
  });

  afterEach(async () => {
    await fs.rm(path.dirname(binA), { recursive: true, force: true });
  });

  async function writeFile(dir: string, name: string, mode: number): Promise<string> {
    const file = path.join(dir, name);
    await fs.writeFile(file, "#!/bin/sh\n");
    await fs.chmod(file, mode);
    return file;
  }

  it("finds a bare name on PATH", async () => {
    const exe = await writeFile(binB, "asfv1", 0o755);
    const found = await findExecutable("asfv1", { pathEnv: `${binA}:${binB}`, platform: "linux" });
    assert.strictEqual(found, exe);
  });

  it("returns the first match in PATH order", async () => {
    const first = await writeFile(binA, "asfv1", 0o755);
    await writeFile(binB, "asfv1", 0o755);
    const found = await findExecutable("asfv1", { pathEnv: `${binA}:${binB}`, platform: "linux" });
    assert.strictEqual(found, first);
  });

  it("returns null when the name is not on PATH", async () => {
    const found = await findExecutable("asfv1", { pathEnv: `${binA}:${binB}`, platform: "linux" });
    assert.strictEqual(found, null);
  });

  it("uses a value with a path separator as a path, not a PATH search", async () => {
    const exe = await writeFile(binA, "asfv1", 0o755);
    assert.strictEqual(await findExecutable(exe, { pathEnv: "", platform: "linux" }), exe);
    assert.strictEqual(await findExecutable(path.join(binB, "asfv1"), { pathEnv: binA, platform: "linux" }), null);
  });

  it("skips files without execute permission and directories", async () => {
    await writeFile(binA, "asfv1", 0o644);
    await fs.mkdir(path.join(binB, "asfv1"));
    const found = await findExecutable("asfv1", { pathEnv: `${binA}:${binB}`, platform: "linux" });
    assert.strictEqual(found, null);
  });

  it("returns null for an empty setting", async () => {
    assert.strictEqual(await findExecutable("", { pathEnv: binA, platform: "linux" }), null);
  });

  it("tries .exe for a bare name on Windows, using ';' as the PATH separator", async () => {
    const exe = await writeFile(binB, "asfv1.exe", 0o755);
    const found = await findExecutable("asfv1", { pathEnv: `${binA};${binB}`, platform: "win32" });
    assert.strictEqual(found, exe);
  });

  it("does not add an extension on Windows when one is given", async () => {
    await writeFile(binA, "asfv1.exe", 0o755);
    const found = await findExecutable("asfv1.bat", { pathEnv: binA, platform: "win32" });
    assert.strictEqual(found, null);
  });

  it("ignores a .com of the same name on Windows", async () => {
    await writeFile(binA, "asfv1.com", 0o755);
    const exe = await writeFile(binB, "asfv1.exe", 0o755);
    const found = await findExecutable("asfv1", { pathEnv: `${binA};${binB}`, platform: "win32" });
    assert.strictEqual(found, exe);
  });
});
