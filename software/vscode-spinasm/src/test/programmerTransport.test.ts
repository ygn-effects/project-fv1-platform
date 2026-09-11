import * as assert from "assert";
import { EventEmitter } from "events";
import Programmer, { SerialTransport } from "../programmer";
import { BANK_SIZE_BYTES, EEPROM_BLOCK_SIZE_BYTES } from "../fv1Constants";

const START_MARKER = 0x1e;
const END_MARKER = 0x1f;
const READ_ORDER = 0x03;
const NOK_RESPONSE = 0x06;
const OK_RESPONSE = 0x07;

function frame(payload: Buffer): Buffer {
  return Buffer.concat([
    Buffer.from([START_MARKER]),
    payload,
    Buffer.from([END_MARKER]),
  ]);
}

class SimulatedSerialTransport extends EventEmitter implements SerialTransport {
  public readonly path = "/dev/simulated-fv1-programmer";
  public isOpen = true;
  public readonly writes: Buffer[] = [];

  constructor(private readonly responses: Buffer[][]) {
    super();
  }

  public open(callback: (error: Error | null) => void): void {
    this.isOpen = true;
    callback(null);
  }

  public close(callback: (error: Error | null) => void): void {
    this.isOpen = false;
    callback(null);
  }

  public write(data: Buffer, callback: (error: Error | null | undefined) => void): boolean {
    this.writes.push(Buffer.from(data));
    const chunks = this.responses.shift();
    callback(null);

    if (chunks === undefined) {
      throw new Error("Simulated transport has no response for this write.");
    }

    queueMicrotask(() => {
      for (const chunk of chunks) {
        this.emit("data", chunk);
      }
    });

    return true;
  }
}

describe("Programmer serial transport", () => {
  it("reads binary EEPROM pages containing frame markers from fragmented responses", async () => {
    const expected = Buffer.alloc(BANK_SIZE_BYTES);
    const responses: Buffer[][] = [];

    for (let offset = 0; offset < BANK_SIZE_BYTES; offset += EEPROM_BLOCK_SIZE_BYTES) {
      const page = Buffer.alloc(EEPROM_BLOCK_SIZE_BYTES);
      for (let index = 0; index < page.length; index++) {
        page[index] = (offset + index) & 0xff;
      }

      page[3] = START_MARKER;
      page[7] = END_MARKER;
      page.copy(expected, offset);

      const readOrderResponse = frame(Buffer.from([OK_RESPONSE]));
      responses.push(offset === 0
        ? [Buffer.concat([readOrderResponse, frame(Buffer.from([NOK_RESPONSE]))])]
        : [readOrderResponse]);
      responses.push([frame(Buffer.from([OK_RESPONSE]))]);

      const dataFrame = frame(page);
      responses.push([
        dataFrame.subarray(0, 2),
        dataFrame.subarray(2, 9),
        dataFrame.subarray(9),
      ]);
    }

    const transport = new SimulatedSerialTransport(responses);
    const programmer = new Programmer(transport.path, 57600, transport);

    const actual = await programmer.readProgram(0);

    assert.deepStrictEqual(actual, expected);
    assert.strictEqual(responses.length, 0, "every simulated response was consumed");
    assert.strictEqual(
      transport.writes.length,
      (BANK_SIZE_BYTES / EEPROM_BLOCK_SIZE_BYTES) * 3,
    );
    assert.ok(
      transport.writes.every(message =>
        message[0] === START_MARKER && message[message.length - 1] === END_MARKER,
      ),
      "the existing firmware wire format remains unchanged",
    );
    assert.ok(
      transport.writes.filter(message => message[1] === READ_ORDER).length >=
        BANK_SIZE_BYTES / EEPROM_BLOCK_SIZE_BYTES,
    );
  });
});
