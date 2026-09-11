import * as assert from "assert";
import SerialFrameParser from "../serialFrameParser";

const START_MARKER = 0x1e;
const END_MARKER = 0x1f;

function frame(payload: Buffer): Buffer {
  return Buffer.concat([
    Buffer.from([START_MARKER]),
    payload,
    Buffer.from([END_MARKER]),
  ]);
}

describe("SerialFrameParser", () => {
  it("accepts every possible byte value as binary payload", () => {
    const parser = new SerialFrameParser(START_MARKER, END_MARKER);

    for (let value = 0; value <= 0xff; value++) {
      parser.push(frame(Buffer.from([value])));
      assert.deepStrictEqual(parser.readFrame(1), Buffer.from([value]));
    }
  });

  it("does not treat start and end markers inside a payload as boundaries", () => {
    const parser = new SerialFrameParser(START_MARKER, END_MARKER);
    const payload = Buffer.from([
      0x01, END_MARKER, 0x02, START_MARKER,
      0x03, END_MARKER, START_MARKER, 0x04,
    ]);

    parser.push(frame(payload));

    assert.deepStrictEqual(parser.readFrame(payload.length), payload);
  });

  it("assembles a response fragmented into individual bytes", () => {
    const parser = new SerialFrameParser(START_MARKER, END_MARKER);
    const payload = Buffer.from([0x10, END_MARKER, 0x20, START_MARKER]);
    const response = frame(payload);

    for (let index = 0; index < response.length - 1; index++) {
      parser.push(response.subarray(index, index + 1));
      assert.strictEqual(parser.readFrame(payload.length), undefined);
    }

    parser.push(response.subarray(response.length - 1));
    assert.deepStrictEqual(parser.readFrame(payload.length), payload);
  });

  it("retains a second frame that arrives in the same chunk", () => {
    const parser = new SerialFrameParser(START_MARKER, END_MARKER);
    const first = Buffer.from([0x11, END_MARKER, 0x22]);
    const second = Buffer.from([0x07]);

    parser.push(Buffer.concat([frame(first), frame(second)]));

    assert.deepStrictEqual(parser.readFrame(first.length), first);
    assert.deepStrictEqual(parser.readFrame(second.length), second);
  });

  it("discards leading junk and resynchronizes after a malformed candidate", () => {
    const parser = new SerialFrameParser(START_MARKER, END_MARKER);
    const validPayload = Buffer.from([0x42]);

    parser.push(Buffer.concat([
      Buffer.from([0xaa, 0xbb, START_MARKER, 0x11, 0x00]),
      frame(validPayload),
    ]));

    assert.deepStrictEqual(parser.readFrame(validPayload.length), validPayload);
  });

  it("drains an incomplete response without returning a frame", () => {
    const parser = new SerialFrameParser(START_MARKER, END_MARKER);
    const partial = Buffer.from([START_MARKER, 0x01, 0x02]);

    parser.push(partial);

    assert.strictEqual(parser.readFrame(3), undefined);
    assert.deepStrictEqual(parser.drain(), partial);
    assert.strictEqual(parser.readFrame(3), undefined);
  });
});
