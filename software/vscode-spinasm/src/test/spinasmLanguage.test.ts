import * as assert from "assert";
import { isInstruction, isBuiltInSymbol } from "../spinasmLanguage";

describe("isInstruction", () => {
  it("recognizes a plain instruction (any case, leading whitespace)", () => {
    assert.ok(isInstruction("SOF 0,0"));
    assert.ok(isInstruction("rdax adcl, 1.0"));
    assert.ok(isInstruction("  RDAX ADCL, 1.0"));
  });

  it("recognizes an instruction behind a label prefix", () => {
    assert.ok(isInstruction("start: SOF 0,0"));
  });

  it("recognizes single-token instructions", () => {
    assert.ok(isInstruction("NOP"));
    assert.ok(isInstruction("CLR"));
    assert.ok(isInstruction("JMP loop"));
    assert.ok(isInstruction("RAW $00000000"));
  });

  it("rejects equ / mem directives", () => {
    assert.ok(!isInstruction("equ tempo 32"));
    assert.ok(!isInstruction("mem delay 4096"));
    assert.ok(!isInstruction("MEM delay int(100)"));
  });

  it("rejects a bare label line", () => {
    assert.ok(!isInstruction("loop:"));
    assert.ok(!isInstruction("  loop:   "));
  });

  it("rejects blank and comment lines", () => {
    assert.ok(!isInstruction(""));
    assert.ok(!isInstruction("   "));
    assert.ok(!isInstruction("; just a comment"));
  });

  it("requires the mnemonic at the start (or after a label), not mid-line", () => {
    assert.ok(!isInstruction("x RDAX"));
  });

  it("respects word boundaries — no match inside a longer identifier", () => {
    assert.ok(!isInstruction("andy 1, 2"));   // not AND
    assert.ok(!isInstruction("software 1"));  // not SOF
  });
});

describe("isBuiltInSymbol", () => {
  it("recognizes numbered registers (any case)", () => {
    assert.ok(isBuiltInSymbol("REG0"));
    assert.ok(isBuiltInSymbol("REG31"));
    assert.ok(isBuiltInSymbol("reg5"));
  });

  it("recognizes hardware registers, LFOs, and condition flags", () => {
    assert.ok(isBuiltInSymbol("ADCL"));
    assert.ok(isBuiltInSymbol("DACR"));
    assert.ok(isBuiltInSymbol("POT0"));
    assert.ok(isBuiltInSymbol("SIN0"));
    assert.ok(isBuiltInSymbol("RUN"));
    assert.ok(isBuiltInSymbol("GEZ"));
    assert.ok(isBuiltInSymbol("adcl")); // case-insensitive
  });

  it("rejects user-defined names", () => {
    assert.ok(!isBuiltInSymbol("TEMPO"));
    assert.ok(!isBuiltInSymbol("myDelay"));
  });
});
