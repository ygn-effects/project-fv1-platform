import { DelimiterParser, SerialPort } from "serialport";
import Logs, { LogType } from "./logs";
import * as fs from 'fs/promises';
import { BANK_SIZE_BYTES, EEPROM_BLOCK_SIZE_BYTES, EEPROM_BLANK_BYTE } from "./fv1Constants";

interface IntelHexData {
  address: number;
  data: Buffer;
}

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

export default class Programmer {
  private serialPort: SerialPort;
  private parser: DelimiterParser;
  private readonly startMarker = 0x1e;
  private readonly endMarker = 0x1f;
  private lastCommandTimestamp = 0;

  constructor(port: string, baudRate: number) {
    this.serialPort = new SerialPort({ path: port, baudRate, autoOpen: false });
    this.parser = this.serialPort.pipe(new DelimiterParser({ delimiter: Buffer.from([this.endMarker]) }));
  }

  public async connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      this.serialPort.open((err) => {
        if (err) {
          Logs.log(LogType.ERROR, `Failed to open serial port: ${err.message}`);
          return reject(err);
        }

        Logs.log(LogType.INFO, `Serial port ${this.serialPort.path} opened successfully.`);

        // Some adapters emit junk bytes right after open(); discard for 100 ms
        // so the first real command doesn't get the leftovers as its response.
        const discardDuration = 100;
        const discardData = (data: Buffer) => {
          Logs.log(LogType.INFO, `Discarding junk data: ${data.toString('hex')}`);
        };

        this.serialPort.on("data", discardData);

        setTimeout(() => {
          this.serialPort.removeListener("data", discardData);
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
    if (program.length < BANK_SIZE_BYTES) {
        const padding = Buffer.alloc(BANK_SIZE_BYTES - program.length, EEPROM_BLANK_BYTE);
        program = Buffer.concat([program, padding]);
    }

    for (let offset = 0; offset < BANK_SIZE_BYTES; offset += EEPROM_BLOCK_SIZE_BYTES) {
      const currentAddress = address + offset;

      let data = Buffer.alloc(EEPROM_BLOCK_SIZE_BYTES);
      program.copy(data, 0, offset, offset + EEPROM_BLOCK_SIZE_BYTES);

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

  /** Parses an Intel HEX file, validates per-record checksums, returns base addr + bytes. */
  public async readIntelHexData(file: string): Promise<IntelHexData> {
    try {
      await fs.access(file);
    }
    catch {
      throw new Error(`Unable to open file: ${file}`);
    }

    Logs.log(LogType.INFO, `Reading HEX file: ${file}`);
    const content = await fs.readFile(file, { encoding: 'utf8' });
    const lines = content.split(/\r\n|\r|\n/);

    const memoryMap = new Map<number, number>();
    let minAddress = Infinity;
    let maxAddress = 0;
    let lineNo = 0;

    for (const line of lines) {
        lineNo++;

        if (line.trim().length === 0) {
          continue;
        }

        if (line[0] !== ':') {
          continue;
        }

        // Intel HEX record: `:LLAAAATT[DD...]CC` — min frame (no data) is 11 chars
        // (`:` + LL(2) + AAAA(4) + TT(2) + CC(2)). Data adds 2*LL between TT and CC.
        if (line.length < 11) {
          throw new Error(`Malformed HEX record at line ${lineNo}: too short (${line.length} chars)`);
        }

        const byteCount = parseInt(line.substr(1, 2), 16);
        const address = parseInt(line.substr(3, 4), 16);
        const recordType = parseInt(line.substr(7, 2), 16);

        if (isNaN(byteCount) || isNaN(address) || isNaN(recordType)) {
          throw new Error(`Malformed HEX record at line ${lineNo}: non-hex header`);
        }

        const expectedLength = 11 + 2 * byteCount;
        if (line.length < expectedLength) {
          throw new Error(`Malformed HEX record at line ${lineNo}: expected ${expectedLength} chars for byteCount=${byteCount}, got ${line.length}`);
        }

        const checksum = parseInt(line.substr(line.length - 2, 2), 16);
        if (isNaN(checksum)) {
          throw new Error(`Malformed HEX record at line ${lineNo}: non-hex checksum`);
        }

        let calculatedChecksum = byteCount + (address >> 8) + (address & 0xFF) + recordType;

        for (let i = 0; i < byteCount; i++) {
          const byte = parseInt(line.substr(9 + (i * 2), 2), 16);
          if (isNaN(byte)) {
            throw new Error(`Malformed HEX record at line ${lineNo}: non-hex data byte at offset ${i}`);
          }
          calculatedChecksum += byte;
        }
        // Intel HEX checksum is two's complement of the LSB of the running sum.
        if (((calculatedChecksum + checksum) & 0xFF) !== 0) {
          throw new Error(`Checksum mismatch at line ${lineNo}`);
        }

        if (recordType === 0x00) { // data
          for (let i = 0; i < byteCount; i++) {
            const byte = parseInt(line.substr(9 + (i * 2), 2), 16);
            const absoluteAddress = address + i;
            memoryMap.set(absoluteAddress, byte);

            if (absoluteAddress < minAddress) {
              minAddress = absoluteAddress;
            }

            if (absoluteAddress > maxAddress) {
              maxAddress = absoluteAddress;
            }
          }
        } else if (recordType === 0x01) { // EOF
            break;
        }
    }

    if (minAddress === Infinity) {
      throw new Error("HEX file contained no valid data records.");
    }

    // Bank size is fixed but we tolerate larger inputs in case of off-spec hex.
    const size = maxAddress - minAddress + 1;
    const bufferSize = Math.max(BANK_SIZE_BYTES, size);
    const buffer = Buffer.alloc(bufferSize, EEPROM_BLANK_BYTE);

    memoryMap.forEach((byte, addr) => {
      buffer[addr - minAddress] = byte;
    });

    Logs.log(LogType.INFO, `Parsed HEX. Base Address: 0x${minAddress.toString(16)} | Size: ${size} bytes`);

    return {
      address: minAddress,
      data: buffer
    };
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

  private async sendMessage(payload: Buffer, expectedResponseSize: number, timeoutMs = 500): Promise<Buffer> {
    // Hardware needs ≥10 ms between commands or it drops the next message.
    const now = Date.now();
    const elapsed = now - this.lastCommandTimestamp;
    const requiredDelay = 10;

    if (elapsed < requiredDelay) {
      await new Promise((resolve) => setTimeout(resolve, requiredDelay - elapsed));
    }

    this.lastCommandTimestamp = Date.now();

    const message = Buffer.concat([
      Buffer.from([this.startMarker]),
      payload,
      Buffer.from([this.endMarker]),
    ]);

    return new Promise((resolve, reject) => {
      const timeout = setTimeout(() => {
        this.parser.removeAllListeners("data");
        Logs.log(LogType.ERROR, "Timeout waiting for programmer response.");
        reject(new Error("Timeout waiting for programmer response."));
      }, timeoutMs);

      this.parser.once("data", (data: Buffer) => {
        clearTimeout(timeout);

        if (data.length !== expectedResponseSize + 1 || data[0] !== this.startMarker) {
          Logs.log(LogType.ERROR,`Invalid response format: ${data.toString("hex")}`);
          return reject(new Error("Invalid response format from programmer."));
        }

        if (data.length === 2) {
          const code = data[1];
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

        resolve(data.slice(1));
      });

      this.serialPort.write(message, (err) => {
        if (err) {
          clearTimeout(timeout);
          this.parser.removeAllListeners("data");
          Logs.log(LogType.ERROR, `Failed to write message: ${err.message}`);
          reject(err);
        } else {
          Logs.log(LogType.INFO, `Message sent: ${message.toString("hex")}`);
        }
      });
    });
  }
}
