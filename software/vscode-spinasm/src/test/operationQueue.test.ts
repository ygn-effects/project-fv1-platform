import * as assert from "assert";
import { OperationQueue } from "../operationQueue";

/** A promise the test resolves by hand, to hold an operation open. */
function deferred(): { promise: Promise<void>; resolve: () => void } {
  let resolve!: () => void;
  const promise = new Promise<void>(r => (resolve = r));
  return { promise, resolve };
}

describe("OperationQueue", () => {
  it("does not start an operation until the previous one has finished", async () => {
    const queue = new OperationQueue();
    const events: string[] = [];
    const firstGate = deferred();

    const first = queue.run(async () => {
      events.push("first:start");
      await firstGate.promise;
      events.push("first:end");
    });
    const second = queue.run(async () => {
      events.push("second:start");
    });

    await new Promise(r => setImmediate(r));
    assert.deepStrictEqual(events, ["first:start"], "second must wait while first is running");

    firstGate.resolve();
    await Promise.all([first, second]);
    assert.deepStrictEqual(events, ["first:start", "first:end", "second:start"]);
  });

  it("returns each operation's result", async () => {
    const queue = new OperationQueue();
    const [a, b] = await Promise.all([queue.run(async () => 1), queue.run(async () => "two")]);
    assert.strictEqual(a, 1);
    assert.strictEqual(b, "two");
  });

  it("rejects with a failed operation's error and still runs the next one", async () => {
    const queue = new OperationQueue();
    const failing = queue.run(async () => {
      throw new Error("compile failed");
    });
    const next = queue.run(async () => "ran");

    await assert.rejects(failing, /compile failed/);
    assert.strictEqual(await next, "ran");
  });

  it("reports busy while an operation is running or waiting", async () => {
    const queue = new OperationQueue();
    assert.strictEqual(queue.isBusy, false);

    const gate = deferred();
    const first = queue.run(() => gate.promise);
    const second = queue.run(async () => undefined);
    assert.strictEqual(queue.isBusy, true);

    gate.resolve();
    await Promise.all([first, second]);
    assert.strictEqual(queue.isBusy, false);
  });
});
