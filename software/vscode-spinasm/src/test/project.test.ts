import * as assert from "assert";
import * as fs from "fs/promises";
import * as os from "os";
import * as path from "path";
import Project from "../project";

describe("Project.scanPrograms", () => {
  let root: string;
  let project: Project;

  beforeEach(async () => {
    root = await fs.mkdtemp(path.join(os.tmpdir(), "spinasm-project-"));
    await fs.mkdir(path.join(root, "bank_0"));
    await fs.writeFile(path.join(root, "bank_0", "chorus.spn"), "");
    project = new Project(root);
  });

  afterEach(async () => {
    project.dispose();
    await fs.rm(root, { recursive: true, force: true });
  });

  it("maps each bank folder's program and output", async () => {
    await project.scanPrograms();

    assert.strictEqual(project.getAllPrograms()[0], path.join(root, "bank_0", "chorus.spn"));
    assert.strictEqual(project.getOutput(0), path.join(root, "output", "bank_0.hex"));
    assert.strictEqual(project.getAllPrograms()[1], null);
  });

  it("keeps the previous banks visible while a rescan is in flight", async () => {
    await project.scanPrograms();
    await fs.mkdir(path.join(root, "bank_1"));
    await fs.writeFile(path.join(root, "bank_1", "delay.spn"), "");

    const rescan = project.scanPrograms();
    assert.strictEqual(project.getProgramBankByPath(path.join(root, "bank_0", "chorus.spn")), 0);
    assert.strictEqual(project.getOutput(0), path.join(root, "output", "bank_0.hex"));

    await rescan;
    assert.strictEqual(project.getAllPrograms()[1], path.join(root, "bank_1", "delay.spn"));
  });
});
