import * as assert from "assert";
import * as fs from "fs/promises";
import * as os from "os";
import * as path from "path";
import Project from "../project";
import { BANK_COUNT, BANK_SIZE_BYTES, EEPROM_SIZE_BYTES } from "../fv1Constants";
import { createFakeCompiler, FakeCompiler } from "./fakeCompiler";

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

describe("Project scanning details", () => {
  let root: string;
  let project: Project;

  beforeEach(async () => {
    root = await fs.mkdtemp(path.join(os.tmpdir(), "spinasm-scan-"));
    project = new Project(root);
  });

  afterEach(async () => {
    project.dispose();
    await fs.rm(root, { recursive: true, force: true });
  });

  it("picks the first .spn in sorted order and reports the others", async () => {
    await fs.mkdir(path.join(root, "bank_2"));
    for (const name of ["zeta.spn", "alpha.spn", "notes.txt"]) {
      await fs.writeFile(path.join(root, "bank_2", name), "");
    }
    await project.scanPrograms();

    assert.strictEqual(project.getAllPrograms()[2], path.join(root, "bank_2", "alpha.spn"));
    assert.deepStrictEqual(project.getExtraPrograms(2), [path.join(root, "bank_2", "zeta.spn")]);
    assert.deepStrictEqual(project.getExtraPrograms(0), []);
  });

  it("accepts an upper-case .SPN extension", async () => {
    await fs.mkdir(path.join(root, "bank_0"));
    await fs.writeFile(path.join(root, "bank_0", "CHORUS.SPN"), "");
    await project.scanPrograms();
    assert.strictEqual(project.getAllPrograms()[0], path.join(root, "bank_0", "CHORUS.SPN"));
  });

  it("finds a program's bank from an equivalent path, and -1 otherwise", async () => {
    await fs.mkdir(path.join(root, "bank_4"));
    await fs.writeFile(path.join(root, "bank_4", "a.spn"), "");
    await project.scanPrograms();

    assert.strictEqual(project.getProgramBankByPath(path.join(root, "bank_0", "..", "bank_4", "a.spn")), 4);
    assert.strictEqual(project.getProgramBankByPath(path.join(root, "bank_4", "b.spn")), -1);
    assert.strictEqual(project.getProgramBankByPath(undefined), -1);
  });
});

describe("Project.createProjectStructure", () => {
  let root: string;
  let project: Project;

  beforeEach(async () => {
    root = await fs.mkdtemp(path.join(os.tmpdir(), "spinasm-create-"));
    project = new Project(root);
  });

  afterEach(async () => {
    project.dispose();
    await fs.rm(root, { recursive: true, force: true });
  });

  it("creates every bank folder with a program, plus output/, and announces it", async () => {
    let structureEvents = 0;
    project.onDidChangeStructure(() => structureEvents++);

    await project.createProjectStructure();

    for (let bank = 0; bank < BANK_COUNT; bank++) {
      await fs.access(path.join(root, `bank_${bank}`, "program.spn"));
    }
    await fs.access(path.join(root, "output"));
    assert.strictEqual(structureEvents, 1);
  });

  it("keeps programs that already exist", async () => {
    await fs.mkdir(path.join(root, "bank_1"));
    await fs.writeFile(path.join(root, "bank_1", "program.spn"), "my work\n");

    await project.createProjectStructure();

    assert.strictEqual(await fs.readFile(path.join(root, "bank_1", "program.spn"), "utf8"), "my work\n");
  });
});

describe("Project compiling", () => {
  let root: string;
  let project: Project;
  let fake: FakeCompiler;
  const source = (bank: number) => path.join(root, `bank_${bank}`, "program.spn");
  const hex = (bank: number) => path.join(root, "output", `bank_${bank}.hex`);

  async function addProgram(bank: number, content: string): Promise<void> {
    await fs.mkdir(path.join(root, `bank_${bank}`), { recursive: true });
    await fs.writeFile(source(bank), content);
  }

  beforeEach(async () => {
    root = await fs.mkdtemp(path.join(os.tmpdir(), "spinasm-compile-"));
    fake = await createFakeCompiler(root);
    project = new Project(root);
  });

  afterEach(async () => {
    project.dispose();
    await fs.rm(root, { recursive: true, force: true });
  });

  it("runs the compiler with the configured arguments and the bank's -p, source and output", async () => {
    await addProgram(3, "clr\n");
    await project.buildSetup(fake.compiler, [...fake.args, "-s"]);

    await project.compileProgramToHex(3);

    const [invocation] = await fake.invocations();
    assert.deepStrictEqual(invocation, ["-s", "-p", "3", source(3), hex(3)]);
    assert.strictEqual(await fs.readFile(hex(3), "utf8"), "fake hex for bank 3\n");
  });

  it("creates output/ when a hand-made project lacks it", async () => {
    await addProgram(0, "clr\n");
    await project.buildSetup(fake.compiler, fake.args);
    await project.compileProgramToHex(0);
    await fs.access(hex(0));
  });

  it("reports the compiled bank and the compiler's warnings", async () => {
    await addProgram(1, "WARN\n");
    await project.buildSetup(fake.compiler, fake.args);
    const compiled: number[] = [];
    const outputs: { sourcePath: string; stderr: string }[] = [];
    project.onDidCompile(bank => compiled.push(bank));
    project.onCompilerOutput(output => outputs.push(output));

    await project.compileProgramToHex(1);

    assert.deepStrictEqual(compiled, [1]);
    assert.strictEqual(outputs.length, 1);
    assert.strictEqual(outputs[0].sourcePath, source(1));
    assert.match(outputs[0].stderr, /warning: Something odd on line 1/);
  });

  it("fails on a compiler error, reports its output and leaves no stale .hex", async () => {
    await addProgram(0, "FAIL\n");
    await fs.mkdir(path.join(root, "output"));
    await fs.writeFile(hex(0), "previous output");
    await project.buildSetup(fake.compiler, fake.args);
    const compiled: number[] = [];
    const outputs: string[] = [];
    project.onDidCompile(bank => compiled.push(bank));
    project.onCompilerOutput(output => outputs.push(output.stderr));

    await assert.rejects(project.compileProgramToHex(0), /return code: 2/);

    assert.deepStrictEqual(compiled, []);
    assert.match(outputs[0], /parse error: Unexpected FAIL on line 1/);
    await assert.rejects(fs.access(hex(0)), "a failed compile must not leave the previous .hex to be uploaded");
  });

  it("refuses to compile an empty bank", async () => {
    await project.buildSetup(fake.compiler, fake.args);
    await assert.rejects(project.compileProgramToHex(5), /does not exist/);
    assert.deepStrictEqual(await fake.invocations(), []);
  });

  it("builds output.bin from each bank's slice, zero-filling empty banks", async () => {
    await addProgram(0, "clr\n");
    await addProgram(2, "clr\n");
    await project.buildSetup(fake.compiler, fake.args);

    await project.compileAllProgramsToCombinedBin();

    const image = await fs.readFile(path.join(root, "output", "output.bin"));
    assert.strictEqual(image.length, EEPROM_SIZE_BYTES);
    const slice = (bank: number) => image.subarray(bank * BANK_SIZE_BYTES, (bank + 1) * BANK_SIZE_BYTES);
    assert.ok(slice(0).every(byte => byte === 1), "bank 0 comes from the -p 0 output");
    assert.ok(slice(1).every(byte => byte === 0), "an empty bank is zero-filled");
    assert.ok(slice(2).every(byte => byte === 3), "bank 2 comes from the -p 2 output");
    assert.deepStrictEqual(
      (await fs.readdir(path.join(root, "output"))).sort(), ["output.bin"],
      "per-bank temporary files are removed"
    );
  });

  it("writes no output.bin when a bank fails, and still removes temporary files", async () => {
    await addProgram(0, "clr\n");
    await addProgram(1, "FAIL\n");
    await project.buildSetup(fake.compiler, fake.args);

    await assert.rejects(project.compileAllProgramsToCombinedBin(), /bank 1/);

    assert.deepStrictEqual(await fs.readdir(path.join(root, "output")), []);
  });
});
