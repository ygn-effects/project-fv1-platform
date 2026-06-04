import * as assert from "assert";
import { parseAsfv1Output } from "../asfv1Output";

describe("parseAsfv1Output", () => {
  it("parses a parse error with a line number", () => {
    const problems = parseAsfv1Output("parse error: Invalid register 0x40 for RDAX on line 5\n");
    assert.strictEqual(problems.length, 1);
    assert.strictEqual(problems[0].severity, "error");
    assert.strictEqual(problems[0].line, 5);
    assert.match(problems[0].message, /Invalid register 0x40 for RDAX/);
  });

  it("parses a scan error", () => {
    const problems = parseAsfv1Output("scan error: Invalid integer literal %xyz on line 3");
    assert.strictEqual(problems[0].severity, "error");
    assert.strictEqual(problems[0].line, 3);
  });

  it("parses a warning", () => {
    const problems = parseAsfv1Output("warning: S1_14 arg clamped to 0xffff for WRAX on line 7");
    assert.strictEqual(problems[0].severity, "warning");
    assert.strictEqual(problems[0].line, 7);
    assert.match(problems[0].message, /clamped/);
  });

  it("ignores info and unrelated output", () => {
    const problems = parseAsfv1Output("info: 8/128 instructions used\nsome random text\n");
    assert.deepStrictEqual(problems, []);
  });

  it("handles a prefixed message with no line number (file-level)", () => {
    const problems = parseAsfv1Output("parse error: unexpected end of file");
    assert.strictEqual(problems.length, 1);
    assert.strictEqual(problems[0].line, null);
    assert.strictEqual(problems[0].severity, "error");
  });

  it("parses multiple problems and preserves order", () => {
    const out = [
      "warning: S1_14 arg clamped to 0xffff for WRAX on line 7",
      "parse error: Invalid register on line 12",
    ].join("\n");
    const problems = parseAsfv1Output(out);
    assert.strictEqual(problems.length, 2);
    assert.strictEqual(problems[0].severity, "warning");
    assert.strictEqual(problems[0].line, 7);
    assert.strictEqual(problems[1].severity, "error");
    assert.strictEqual(problems[1].line, 12);
  });

  it("filters out the -s SpinASM real-literal notice (always emitted)", () => {
    const problems = parseAsfv1Output("warning: SpinASM compatibility - literals 2,1 read as 2.0,1.0");
    assert.deepStrictEqual(problems, []);
  });

  it("keeps real warnings alongside the filtered compat notice", () => {
    const out = [
      "warning: SpinASM compatibility - literals 2,1 read as 2.0,1.0",
      "warning: S1_14 arg clamped to 0xffff for WRAX on line 7",
    ].join("\n");
    const problems = parseAsfv1Output(out);
    assert.strictEqual(problems.length, 1);
    assert.match(problems[0].message, /clamped/);
    assert.strictEqual(problems[0].line, 7);
  });

  it("returns nothing for empty output", () => {
    assert.deepStrictEqual(parseAsfv1Output(""), []);
  });
});
