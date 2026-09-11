import * as assert from "assert";
import * as path from "path";
import { findDirtyProgramPaths, OpenDocumentState } from "../dirtyProgramGuard";

function document(fsPath: string, isDirty: boolean): OpenDocumentState {
  return { fsPath, isDirty };
}

describe("findDirtyProgramPaths", () => {
  const bank0 = path.resolve("/project/bank_0/chorus.spn");
  const bank1 = path.resolve("/project/bank_1/delay.spn");

  it("returns a project program with unsaved editor changes", () => {
    assert.deepStrictEqual(
      findDirtyProgramPaths([bank0, bank1], [document(bank0, true)]),
      [bank0],
    );
  });

  it("ignores saved documents and dirty files outside the project programs", () => {
    const unrelated = path.resolve("/other/scratch.spn");

    assert.deepStrictEqual(
      findDirtyProgramPaths(
        [bank0, bank1],
        [document(bank0, false), document(unrelated, true)],
      ),
      [],
    );
  });

  it("checks every populated bank before a batch compilation", () => {
    assert.deepStrictEqual(
      findDirtyProgramPaths(
        [bank0, null, bank1],
        [document(bank1, true), document(bank0, true)],
      ),
      [bank0, bank1],
    );
  });

  it("normalizes equivalent document and project paths", () => {
    const equivalent = path.join("/project", "bank_2", "..", "bank_0", "chorus.spn");

    assert.deepStrictEqual(
      findDirtyProgramPaths([bank0], [document(equivalent, true)]),
      [bank0],
    );
  });

  it("does not report the same program more than once", () => {
    assert.deepStrictEqual(
      findDirtyProgramPaths([bank0, bank0], [document(bank0, true)]),
      [bank0],
    );
  });
});
