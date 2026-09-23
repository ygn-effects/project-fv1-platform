import * as assert from "assert";
import * as fs from "fs/promises";
import * as os from "os";
import * as path from "path";
import { BankStatus, ProjectManager } from "../projectManager";
import Config from "../config";
import { createFakeCompiler } from "./fakeCompiler";

describe("ProjectManager", () => {
  const manager = ProjectManager.getInstance();
  let root: string;
  const source = (bank: number, name = "program.spn") => path.join(root, `bank_${bank}`, name);
  const hex = (bank: number) => path.join(root, "output", `bank_${bank}.hex`);

  async function addProgram(bank: number, name = "program.spn"): Promise<void> {
    await fs.mkdir(path.join(root, `bank_${bank}`), { recursive: true });
    await fs.writeFile(source(bank, name), "clr\n");
  }

  async function addOutput(bank: number, secondsAfterSource: number): Promise<void> {
    await fs.mkdir(path.join(root, "output"), { recursive: true });
    await fs.writeFile(hex(bank), "");
    const sourceTime = new Date(1_000_000_000);
    const outputTime = new Date(1_000_000_000 + secondsAfterSource * 1000);
    await fs.utimes(source(bank), sourceTime, sourceTime);
    await fs.utimes(hex(bank), outputTime, outputTime);
  }

  /**
   * Resolves once `predicate` holds, re-checking on each change event. The
   * manager is shared by the whole test run, so waiting for "the next event"
   * could be satisfied by an unrelated one.
   */
  function until(predicate: () => boolean): Promise<void> {
    return new Promise(resolve => {
      if (predicate()) {
        resolve();
        return;
      }
      const subscription = manager.onDidChangeProject(() => {
        if (predicate()) {
          subscription.dispose();
          resolve();
        }
      });
    });
  }

  beforeEach(async () => {
    root = await fs.mkdtemp(path.join(os.tmpdir(), "spinasm-manager-"));
  });

  afterEach(async () => {
    manager.invalidate(root);
    await fs.rm(root, { recursive: true, force: true });
  });

  it("computes bank status without a configured compiler", async () => {
    assert.strictEqual(Config.getCompilerPath(), "", "test host must have no compiler configured");
    await addProgram(0);

    await manager.refreshProject(root);
    const banks = manager.getBanksSync(root);

    assert.strictEqual(banks[0].status, BankStatus.NotCompiled);
    assert.strictEqual(banks[0].spnFile, source(0));
    assert.strictEqual(banks[1].status, BankStatus.Empty);
  });

  it("tells up-to-date, out-of-date and ambiguous banks apart", async () => {
    await addProgram(0);
    await addOutput(0, 60);
    await addProgram(1);
    await addOutput(1, -60);
    await addProgram(2, "a.spn");
    await addProgram(2, "b.spn");

    await manager.refreshProject(root);
    const banks = manager.getBanksSync(root);

    assert.strictEqual(banks[0].status, BankStatus.UpToDate);
    assert.strictEqual(banks[0].hexFile, hex(0));
    assert.strictEqual(banks[1].status, BankStatus.OutOfDate);
    assert.strictEqual(banks[2].spnFile, source(2, "a.spn"));
    assert.deepStrictEqual(banks[2].extraFiles, [source(2, "b.spn")]);
  });

  it("returns placeholders on a cache miss, then fills them in and announces it", async () => {
    await addProgram(3);

    const placeholder = manager.getBanksSync(root);
    assert.ok(placeholder.every(bank => bank.status === BankStatus.Empty));

    await until(() => manager.getBanksSync(root)[3].status === BankStatus.NotCompiled);
  });

  it("shares one Project between concurrent callers and replaces it after invalidate", async () => {
    const [first, second] = await Promise.all([manager.getProject(root), manager.getProject(root)]);
    assert.strictEqual(first, second);

    manager.invalidate(root);
    assert.notStrictEqual(await manager.getProject(root), first);
  });

  it("refreshes a bank's status when its project compiles it", async () => {
    await addProgram(0);
    await manager.refreshProject(root);
    assert.strictEqual(manager.getBanksSync(root)[0].status, BankStatus.NotCompiled);

    const fake = await createFakeCompiler(root);
    const project = await manager.getProject(root);
    project.configure(fake.compiler, fake.args);

    await project.compileProgramToHex(0);

    await until(() => manager.getBanksSync(root)[0].status === BankStatus.UpToDate);
  });
});
