import * as assert from "assert";
import * as path from "path";
import { pathsEqual } from "../pathUtils";

describe("pathsEqual", () => {
  it("matches identical absolute paths", () => {
    const p = path.resolve("/proj/bank_0/x.spn");
    assert.ok(pathsEqual(p, p));
  });

  it("normalizes separators and . / .. segments", () => {
    assert.ok(pathsEqual(
      path.join("/proj", "bank_0", "x.spn"),
      path.join("/proj", "bank_1", "..", "bank_0", ".", "x.spn")
    ));
  });

  it("treats different files in different banks as unequal", () => {
    assert.ok(!pathsEqual(
      path.resolve("/proj/bank_0/x.spn"),
      path.resolve("/proj/bank_1/x.spn")
    ));
  });

  it("handles drive-letter case per platform", () => {
    if (process.platform === "win32") {
      // The actual bug being fixed: c:\ vs C:\ must compare equal on Windows.
      assert.ok(pathsEqual("c:\\proj\\bank_0\\x.spn", "C:\\proj\\bank_0\\x.spn"));
    } else {
      // POSIX is case-sensitive — distinct names must stay distinct.
      assert.ok(pathsEqual("/proj/x.spn", "/proj/x.spn"));
      assert.ok(!pathsEqual("/proj/X.spn", "/proj/x.spn"));
    }
  });
});
