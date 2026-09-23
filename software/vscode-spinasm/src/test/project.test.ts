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

describe("Project compiler resolution", () => {
  let root: string;
  let project: Project;
  let hexFile: string;

  beforeEach(async () => {
    root = await fs.mkdtemp(path.join(os.tmpdir(), "spinasm-compiler-"));
    await fs.mkdir(path.join(root, "bank_0"));
    await fs.mkdir(path.join(root, "output"));
    await fs.writeFile(path.join(root, "bank_0", "chorus.spn"), "clr\n");
    hexFile = path.join(root, "output", "bank_0.hex");
    await fs.writeFile(hexFile, "last good output");
    project = new Project(root);
  });

  afterEach(async () => {
    project.dispose();
    await fs.rm(root, { recursive: true, force: true });
  });

  it("reports an unset compiler and keeps the previous .hex", async () => {
    await project.buildSetup("", []);
    await assert.rejects(project.compileProgramToHex(0), /Compiler path is not set/);
    assert.strictEqual(await fs.readFile(hexFile, "utf8"), "last good output");
  });

  it("reports a compiler that isn't on PATH and keeps the previous .hex", async () => {
    await project.buildSetup("spinasm-no-such-compiler", []);
    await assert.rejects(project.compileProgramToHex(0), /"spinasm-no-such-compiler" not found/);
    await assert.rejects(project.checkCompiler(), /not found/);
    assert.strictEqual(await fs.readFile(hexFile, "utf8"), "last good output");
  });
});

describe("Project.getOutputState", () => {
  let root: string;
  let project: Project;
  let source: string;
  let hex: string;

  beforeEach(async () => {
    root = await fs.mkdtemp(path.join(os.tmpdir(), "spinasm-output-"));
    await fs.mkdir(path.join(root, "bank_0"));
    await fs.mkdir(path.join(root, "output"));
    source = path.join(root, "bank_0", "chorus.spn");
    hex = path.join(root, "output", "bank_0.hex");
    await fs.writeFile(source, "clr\n");
    project = new Project(root);
    await project.scanPrograms();
  });

  afterEach(async () => {
    project.dispose();
    await fs.rm(root, { recursive: true, force: true });
  });

  it("is missing when the bank has no .hex", async () => {
    assert.strictEqual(await project.getOutputState(0), "missing");
  });

  it("is missing for an empty bank", async () => {
    assert.strictEqual(await project.getOutputState(1), "missing");
  });

  it("is current when the .hex is newer than the source", async () => {
    await fs.writeFile(hex, "");
    await fs.utimes(source, new Date(1000_000), new Date(1000_000));
    await fs.utimes(hex, new Date(2000_000), new Date(2000_000));
    assert.strictEqual(await project.getOutputState(0), "current");
  });

  it("is outdated when the source was modified after the .hex", async () => {
    await fs.writeFile(hex, "");
    await fs.utimes(hex, new Date(1000_000), new Date(1000_000));
    await fs.utimes(source, new Date(2000_000), new Date(2000_000));
    assert.strictEqual(await project.getOutputState(0), "outdated");
  });
});
