/**
 * Incrementally extracts fixed-length frames from a serial byte stream.
 *
 * The programmer protocol uses marker bytes around payloads, but EEPROM data is
 * binary and may contain either marker. The expected payload length therefore
 * defines the frame boundary; markers are only validated at those boundaries.
 */
export default class SerialFrameParser {
  private buffer = Buffer.alloc(0);

  constructor(
    private readonly startMarker: number,
    private readonly endMarker: number,
  ) {}

  public push(chunk: Buffer): void {
    if (chunk.length === 0) {
      return;
    }

    this.buffer = Buffer.concat([this.buffer, chunk]);
  }

  /**
   * Returns the next complete payload of the requested size, if available.
   * Leading junk and malformed frame candidates are discarded while looking
   * for the next start marker. Bytes after a complete frame remain buffered.
   */
  public readFrame(payloadLength: number): Buffer | undefined {
    if (!Number.isInteger(payloadLength) || payloadLength < 0) {
      throw new Error("Serial frame payload length must be a non-negative integer.");
    }

    const frameLength = payloadLength + 2;

    while (this.buffer.length > 0) {
      const startIndex = this.buffer.indexOf(this.startMarker);

      if (startIndex < 0) {
        this.buffer = Buffer.alloc(0);
        return undefined;
      }

      if (startIndex > 0) {
        this.buffer = this.buffer.subarray(startIndex);
      }

      if (this.buffer.length < frameLength) {
        return undefined;
      }

      if (this.buffer[frameLength - 1] === this.endMarker) {
        const payload = Buffer.from(this.buffer.subarray(1, frameLength - 1));
        this.buffer = this.buffer.subarray(frameLength);
        return payload;
      }

      // The candidate did not end where its declared size requires. Drop only
      // its start byte so a later start marker can be used for resynchronizing.
      this.buffer = this.buffer.subarray(1);
    }

    return undefined;
  }

  /** Clears and returns all bytes currently waiting in the parser. */
  public drain(): Buffer {
    const buffered = Buffer.from(this.buffer);
    this.buffer = Buffer.alloc(0);
    return buffered;
  }
}
