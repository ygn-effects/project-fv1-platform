import { SerialPort } from "serialport";
import Logs, { LogType } from "./logs";
import { BANK_SIZE_BYTES, EEPROM_BLOCK_SIZE_BYTES, EEPROM_BLANK_BYTE } from "./fv1Constants";
import SerialFrameParser from "./serialFrameParser";

enum OrderCode {
  RuThere = 0x01,
  RuReady = 0x02,
  Read    = 0x03,
  Write   = 0x04,
  End     = 0x05,
}

enum ResponseCode {
  Nok          = 0x06,
  Ok           = 0x07,
  Timeout      = 0x08,
  WriteError   = 0x09,
  ReadError    = 0x0A,
  ComError     = 0x0B,
  FramingError = 0x0C,
}

/** Narrow serial-port surface used by Programmer and its simulated transport tests. */
export interface SerialTransport {
  readonly path: string;
  readonly isOpen: boolean;
  open(callback: (error: Error | null) => void): void;
  close(callback: (error: Error | null) => void): void;
  on(event: "data", listener: (data: Buffer) => void): this;
  removeListener(event: "data", listener: (data: Buffer) => void): this;
  write(data: Buffer, callback: (error: Error | null | undefined) => void): boolean;
}

export default class Programmer {
  private serialPort: SerialTransport;
  private readonly startMarker = 0x1e;
  private readonly endMarker = 0x1f;
  private readonly parser = new SerialFrameParser(this.startMarker, this.endMarker);
  private lastCommandTimestamp = 0;
  private commandInProgress = false;
  private responseDataHandler: (() => void) | undefined;
  // Set when a command timed out: its reply may still arrive and must be
  // discarded, not consumed as the next command's response.
  private staleResponsePossible = false;

  private readonly collectSerialData = (data: Buffer): void => {
    this.parser.push(data);
    this.responseDataHandler?.();
  };

  constructor(port: string, baudRate: number, serialTransport?: SerialTransport) {
    this.serialPort = serialTransport ?? new SerialPort({ path: port, baudRate, autoOpen: false });
    this.serialPort.on("data", this.collectSerialData);
  }

  public async connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      this.serialPort.open((err) => {
        if (err) {
          Logs.log(LogType.ERROR, `Failed to open serial port: ${err.message}`);
          return reject(err);
        }

        Logs.log(LogType.INFO, `Serial port ${this.serialPort.path} opened successfully.`);

        const discardDuration = 100;
        const discardData = (data: Buffer) => {
          Logs.log(LogType.INFO, `Discarding junk data: ${data.toString('hex')}`);
        };

        this.serialPort.on("data", discardData);

        setTimeout(() => {
          this.serialPort.removeListener("data", discardData);
          this.parser.drain();
          resolve();
        }, discardDuration);
      });
    });
  }

  public async disconnect(): Promise<void> {
    return new Promise((resolve, reject) => {
      if (! this.serialPort.isOpen) {
        return resolve();
      }

      this.serialPort.close((err) => {
        if (err) {
          Logs.log(LogType.ERROR, `Failed to close serial port: ${err.message}`);
          return reject(err);
        }

        Logs.log(LogType.INFO, `Serial port ${this.serialPort.path} closed successfully.`);
        resolve();
      });
    });
  }

  public async isProgrammerConnected(): Promise<boolean> {
    const response = await this.sendMessage(Buffer.from([OrderCode.RuThere]), 1);
    return response[0] === ResponseCode.Ok;
  }

  public async isEepromReady(): Promise<boolean> {
    const response = await this.sendMessage(Buffer.from([OrderCode.RuReady]), 1);
    return response[0] === ResponseCode.Ok;
  }

  public async readProgram(address: number): Promise<Buffer> {
    let program = Buffer.alloc(BANK_SIZE_BYTES);

    for (let offset = 0; offset < BANK_SIZE_BYTES; offset += EEPROM_BLOCK_SIZE_BYTES) {
      const currentAddress = address + offset;

      if (! (await this.sendReadOrder())) {
        throw new Error("Failed to send READ order.");
      }

      if (! (await this.sendAddress(currentAddress))) {
        throw new Error("Failed to send address.");
      }

      let data = await this.readData();
      data.copy(program, offset, 0, EEPROM_BLOCK_SIZE_BYTES);
    }

    return program;
  }

  public async writeProgram(address: number, program: Buffer): Promise<void> {
    // Pad short buffers so every write covers a full EEPROM page.
    let payload = program;
    if (payload.length < BANK_SIZE_BYTES) {
        const padding = Buffer.alloc(BANK_SIZE_BYTES - payload.length, EEPROM_BLANK_BYTE);
        payload = Buffer.concat([payload, padding]);
    }

    for (let offset = 0; offset < BANK_SIZE_BYTES; offset += EEPROM_BLOCK_SIZE_BYTES) {
      const currentAddress = address + offset;

      let data = Buffer.alloc(EEPROM_BLOCK_SIZE_BYTES);
      payload.copy(data, 0, offset, offset + EEPROM_BLOCK_SIZE_BYTES);

      if (! (await this.sendWriteOrder())) {
        throw new Error("Failed to send WRITE order.");
      }

      if (! (await this.sendAddress(currentAddress))) {
        throw new Error("Failed to send address.");
      }

      if (! (await this.sendWriteOrder())) {
        throw new Error("Failed to send WRITE order (2).");
      }

      if (! (await this.sendData(data))) {
        throw new Error("Failed to send data.");
      }

      if (! (await this.sendWriteOrder())) {
        throw new Error("Failed to send WRITE order (3).");
      }
    }

    if (! (await this.sendEndOrder())) {
      throw new Error("Failed to send END order.");
    }
  }

  private async sendWriteOrder(): Promise<boolean> {
    const response = await this.sendMessage(Buffer.from([OrderCode.Write]), 1);
    return response[0] === ResponseCode.Ok;
  }

  private async sendReadOrder(): Promise<boolean> {
    const response = await this.sendMessage(Buffer.from([OrderCode.Read]), 1);
    return response[0] === ResponseCode.Ok;
  }

  private async sendAddress(address: number): Promise<boolean> {
    const response = await this.sendMessage(Buffer.from([(address >> 8) & 0xFF, address & 0xFF]), 1);
    return response[0] === ResponseCode.Ok;
  }

  private async sendEndOrder(): Promise<boolean> {
    const response = await this.sendMessage(Buffer.from([OrderCode.End]), 1);
    return response[0] === ResponseCode.Ok;
  }

  private async readData(): Promise<Buffer> {
    return await this.sendMessage(Buffer.from([OrderCode.Read]), EEPROM_BLOCK_SIZE_BYTES);
  }

  private async sendData(data: Buffer): Promise<boolean> {
    const response = await this.sendMessage(data, 1);
    return response[0] === ResponseCode.Ok;
  }

  /** Briefly listens and discards parser output after a timeout, so a late
   *  reply to the timed-out command can't be mistaken for the next reply. */
  private drainStaleResponses(drainMs = 100): Promise<void> {
    return new Promise((resolve) => {
      setTimeout(() => {
        const stale = this.parser.drain();
        if (stale.length > 0) {
          Logs.log(LogType.DEBUG, `Discarding stale response: ${stale.toString("hex")}`);
        }
        resolve();
      }, drainMs);
    });
  }

  private async sendMessage(payload: Buffer, expectedResponseSize: number, timeoutMs = 500): Promise<Buffer> {
    if (this.commandInProgress) {
      throw new Error("Another programmer command is already in progress.");
    }

    this.commandInProgress = true;

    try {
      if (this.staleResponsePossible) {
        await this.drainStaleResponses();
        this.staleResponsePossible = false;
      }

      // Hardware needs ≥10 ms between commands or it drops the next message.
      const now = Date.now();
      const elapsed = now - this.lastCommandTimestamp;
      const requiredDelay = 10;

      if (elapsed < requiredDelay) {
        await new Promise((resolve) => setTimeout(resolve, requiredDelay - elapsed));
      }

      // The protocol permits only one response per command. If an additional
      // frame arrived in the same serial chunk, do not let it satisfy the next
      // command after that command is sent.
      const unexpected = this.parser.drain();
      if (unexpected.length > 0) {
        Logs.log(LogType.DEBUG, `Discarding unexpected buffered response: ${unexpected.toString("hex")}`);
      }

      this.lastCommandTimestamp = Date.now();

      const message = Buffer.concat([
        Buffer.from([this.startMarker]),
        payload,
        Buffer.from([this.endMarker]),
      ]);

      return await new Promise((resolve, reject) => {
        const onData = () => {
          const response = this.parser.readFrame(expectedResponseSize);

          if (response === undefined) {
            return;
          }

          clearTimeout(timeout);
          this.responseDataHandler = undefined;

          if (response.length === 1) {
            const code = response[0];
            if (code === ResponseCode.Timeout) {
              return reject(new Error("Programmer timed out."));
            }

            if (code === ResponseCode.FramingError) {
              return reject(new Error("Programmer framing error."));
            }

            if (code === ResponseCode.ComError) {
              return reject(new Error("Programmer communication error."));
            }
          }

          resolve(response);
        };

        const timeout = setTimeout(() => {
          this.responseDataHandler = undefined;
          this.staleResponsePossible = true;
          Logs.log(LogType.ERROR, "Timeout waiting for programmer response.");
          reject(new Error("Timeout waiting for programmer response."));
        }, timeoutMs);

        this.responseDataHandler = onData;

        this.serialPort.write(message, (err) => {
          if (err) {
            clearTimeout(timeout);
            this.responseDataHandler = undefined;
            Logs.log(LogType.ERROR, `Failed to write message: ${err.message}`);
            reject(err);
          } else {
            Logs.log(LogType.DEBUG, `Message sent: ${message.toString("hex")}`);
          }
        });
      });
    }
    finally {
      this.responseDataHandler = undefined;
      this.commandInProgress = false;
    }
  }
}
