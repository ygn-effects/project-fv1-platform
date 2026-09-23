import * as assert from "assert";
import * as vscode from "vscode";
import { DocumentParser } from "../documentParser";

async function parse(content: string) {
  const doc = await vscode.workspace.openTextDocument({ content, language: "spinasm" });
  return DocumentParser.get(doc);
}

describe("DocumentParser", () => {
  it("registers a directive-first EQU (EQU NAME VALUE)", async () => {
    const parsed = await parse("equ tempo 32\n");
    assert.ok(parsed.symbols.has("TEMPO"));
    assert.strictEqual(parsed.symbols.get("TEMPO")!.type, "constant");
  });

  it("registers a name-first EQU (NAME EQU VALUE)", async () => {
    const parsed = await parse("tempo equ 32\n");
    assert.ok(parsed.symbols.has("TEMPO"), "conventional SpinASM name-first equ should be recognized");
  });

  it("counts an integer mem allocation plus asfv1's extra sample toward memory usage", async () => {
    const parsed = await parse("mem delay 4096\n");
    assert.ok(parsed.symbols.has("DELAY"));
    assert.strictEqual(parsed.resourceUsage.memory.used, 4097);
  });

  it("counts one extra sample per mem block", async () => {
    const parsed = await parse("mem a 16384\nmem b 16384\n");
    assert.strictEqual(parsed.resourceUsage.memory.used, 32770);
  });

  it("registers an expression-sized mem but leaves it out of the total", async () => {
    const parsed = await parse("mem delay int(32767*3/5)\n");
    assert.ok(parsed.symbols.has("DELAY"), "expression-sized mem should still define the symbol");
    assert.strictEqual(parsed.resourceUsage.memory.used, 0);
  });

  it("accepts a mem allocation of 32767, the largest asfv1 allows", async () => {
    const parsed = await parse("mem big 32767\n");
    assert.ok(!parsed.diagnostics.some(d => d.code === "invalid-memory-size"));
    assert.strictEqual(parsed.resourceUsage.memory.used, 32768);
  });

  it("accepts a zero-size mem allocation (asfv1 range is 0..32767)", async () => {
    const parsed = await parse("mem tap 0\n");
    assert.ok(!parsed.diagnostics.some(d => d.code === "invalid-memory-size"));
  });

  it("rejects a mem allocation of 32768, which asfv1 refuses", async () => {
    const parsed = await parse("mem toobig 32768\n");
    const diag = parsed.diagnostics.find(d => d.code === "invalid-memory-size");
    assert.ok(diag);
    assert.strictEqual(diag.range.start.character, 11, "range should cover the size operand");
  });

  it("warns on SpinASM name-first MEM order but still resolves the symbol", async () => {
    const parsed = await parse("Delay MEM 1024\nrdax Delay, 1.0\n");
    assert.ok(parsed.diagnostics.some(d => d.code === "mem-operand-order"),
      "name-first 'Delay MEM 1024' should warn (asfv1 needs directive-first)");
    assert.ok(parsed.symbols.has("DELAY"), "symbol should still be registered");
    assert.strictEqual(parsed.resourceUsage.memory.used, 1025);
  });

  it("does not warn on directive-first MEM order", async () => {
    const parsed = await parse("mem delay 1024\n");
    assert.ok(!parsed.diagnostics.some(d => d.code === "mem-operand-order"));
  });

  it("does not misread an instruction line containing 'mem' as name-first MEM", async () => {
    const parsed = await parse("rdax mem 1.0\n");
    assert.ok(!parsed.diagnostics.some(d => d.code === "mem-operand-order"));
  });

  it("flags a duplicate symbol", async () => {
    const parsed = await parse("equ a 1\nequ a 2\n");
    assert.ok(parsed.diagnostics.some(d => d.code === "duplicate-symbol"));
  });

  it("flags a duplicate label and keeps the first definition", async () => {
    const parsed = await parse("start:\n  nop\nstart:\n  nop\n");
    assert.ok(parsed.diagnostics.some(d => d.code === "duplicate-symbol"));
    assert.strictEqual(parsed.symbols.get("START")!.line, 0);
  });

  it("flags a label that collides with another symbol kind", async () => {
    const parsed = await parse("equ target 1\ntarget:\n");
    assert.ok(parsed.diagnostics.some(d => d.code === "duplicate-symbol"));
    assert.strictEqual(parsed.symbols.get("TARGET")!.type, "constant");
  });

  it("flags redefinition of a reserved symbol", async () => {
    const parsed = await parse("equ adcl 1\n");
    assert.ok(parsed.diagnostics.some(d => d.code === "reserved-symbol"));
  });

  it("counts instructions and direct register usage", async () => {
    const parsed = await parse("rdax reg0, 1.0\nwrax reg1, 0\n");
    assert.strictEqual(parsed.resourceUsage.instructions.count, 2);
    assert.ok(parsed.resourceUsage.registers.used.has(0));
    assert.ok(parsed.resourceUsage.registers.used.has(1));
  });

  it("counts a register reached through an equ alias", async () => {
    const parsed = await parse("equ vol reg5\nrdax vol, 1.0\n");
    assert.ok(parsed.resourceUsage.registers.used.has(5));
    assert.strictEqual(parsed.resourceUsage.registers.aliases.get("vol"), 5);
  });

  it("does not count a label-only line as an instruction", async () => {
    const parsed = await parse("start:\n  sof 0, 0\n");
    assert.ok(parsed.symbols.has("START"));
    assert.strictEqual(parsed.symbols.get("START")!.type, "label");
    assert.strictEqual(parsed.resourceUsage.instructions.count, 1);
  });

  it("locates a short symbol name after the directive, not inside it", async () => {
    const parsed = await parse("equ e 0.5\nmem m 100\nq equ 1\n");
    assert.strictEqual(parsed.symbols.get("E")!.character, 4);
    assert.strictEqual(parsed.symbols.get("M")!.character, 4);
    assert.strictEqual(parsed.symbols.get("Q")!.character, 0);
  });

  it("locates symbols on indented lines", async () => {
    const parsed = await parse("  equ e 0.5\n\tloop: sof 0, 0\n");
    assert.strictEqual(parsed.symbols.get("E")!.character, 6);
    assert.strictEqual(parsed.symbols.get("LOOP")!.character, 1);
  });

  it("records instruction lines with their mnemonic and operand position", async () => {
    const parsed = await parse("  start: rdax adcl, 1.0 ; in\nclr\n");
    assert.strictEqual(parsed.instructions.length, 2);
    const [rdax, clr] = parsed.instructions;
    assert.strictEqual(rdax.mnemonic, "RDAX");
    assert.strictEqual(rdax.operands, "adcl, 1.0 ");
    assert.strictEqual(rdax.operandsColumn, 14);
    assert.strictEqual(clr.mnemonic, "CLR");
    assert.strictEqual(clr.operands, undefined);
  });

  it("records only lines that are not directives, labels or instructions as unrecognized", async () => {
    const parsed = await parse("equ\nx equ\nloop:\n  sfo 0, 0\n1.0\n");
    assert.deepStrictEqual(parsed.unrecognized, [{ line: 3, word: "sfo", column: 2 }]);
  });
});
