import * as assert from "assert";
import * as fs from "fs/promises";
import * as os from "os";
import * as path from "path";
import { BankStatus, ProjectManager } from "../projectManager";
import Config from "../config";

describe("ProjectManager", () => {
  let root: string;

  beforeEach(async () => {
    root = await fs.mkdtemp(path.join(os.tmpdir(), "spinasm-manager-"));
    await fs.mkdir(path.join(root, "bank_0"));
    await fs.writeFile(path.join(root, "bank_0", "chorus.spn"), "clr\n");
  });

  afterEach(async () => {
    ProjectManager.getInstance().invalidate(root);
    await fs.rm(root, { recursive: true, force: true });
  });

  it("computes bank status without a configured compiler", async () => {
    assert.strictEqual(Config.getCompilerPath(), "", "test host must have no compiler configured");

    const manager = ProjectManager.getInstance();
    await manager.refreshProject(root);
    const banks = manager.getBanksSync(root);

    assert.strictEqual(banks[0].status, BankStatus.NotCompiled);
    assert.strictEqual(banks[0].spnFile, path.join(root, "bank_0", "chorus.spn"));
    assert.strictEqual(banks[1].status, BankStatus.Empty);
  });
});
