import * as assert from "assert";
import { describeStaleOutputs, OutputState } from "../staleOutputGuard";

function states(entries: [number, OutputState][]): Map<number, OutputState> {
  return new Map(entries);
}

describe("describeStaleOutputs", () => {
  it("returns nothing when every bank's output is current", () => {
    assert.strictEqual(describeStaleOutputs(states([[0, "current"], [3, "current"]])), undefined);
  });

  it("offers Upload Anyway when outputs exist but are outdated", () => {
    const prompt = describeStaleOutputs(states([[0, "current"], [2, "outdated"]]));
    assert.ok(prompt);
    assert.strictEqual(prompt.allowUploadAnyway, true);
    assert.match(prompt.detail, /Source changed since the last compile: bank 2\./);
  });

  it("does not offer Upload Anyway when a bank has no output", () => {
    const prompt = describeStaleOutputs(states([[1, "missing"], [2, "outdated"]]));
    assert.ok(prompt);
    assert.strictEqual(prompt.allowUploadAnyway, false);
    assert.match(prompt.detail, /Not compiled yet: bank 1\./);
    assert.match(prompt.detail, /bank 2/);
  });

  it("lists several banks in order", () => {
    const prompt = describeStaleOutputs(states([[5, "outdated"], [1, "outdated"], [3, "outdated"]]));
    assert.ok(prompt);
    assert.match(prompt.detail, /banks 1, 3, 5\./);
  });
});
