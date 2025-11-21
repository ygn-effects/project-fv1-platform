import { DelimiterParser, SerialPort } from "serialport";
import Logs, { LogType } from "./logs";
import * as fs from 'fs/promises';

interface IntelHexData {
  address: number;   // Start address (Base address of the firmware)
  data: Buffer;      // The program data
}

/**
 * @enum OrderCode
 * @brief Codes representing commands sent to the programmer.
 */
enum OrderCode {
  RuThere = 0x01,
  RuReady = 0x02,
  Read    = 0x03,
  Write   = 0x04,
  End     = 0x05,
}

/**
 * @enum ResponseCode
 * @brief Codes representing possible programmer responses.
 */
enum ResponseCode {
  Nok          = 0x06,
  Ok           = 0x07,
  Timeout      = 0x08,
  WriteError   = 0x09,
  ReadError    = 0x0A,
  ComError     = 0x0B,
  FramingError = 0x0C,
}

/**
 * @class Programmer
 * @brief Manages communication with EEPROM programmer hardware via serial port.
 */
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

        // Allow 100 ms to discard initial noise
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
    let program = Buffer.alloc(512);

    for (let offset = 0; offset < 512; offset += 32) {
      const currentAddress = address + offset;

      if (! (await this.sendReadOrder())) {
        throw new Error("Failed to send READ order.");
      }

      if (! (await this.sendAddress(currentAddress))) {
        throw new Error("Failed to send address.");
      }

      let data = await this.readData();
      data.copy(program, offset, 0, 32);
    }

    return program;
  }

  public async writeProgram(address: number, program: Buffer): Promise<void> {
    // Pad buffer to ensure we have full pages
    if (program.length < 512) {
        const padding = Buffer.alloc(512 - program.length, 0xFF); // 0xFF is standard EEPROM blank state
        program = Buffer.concat([program, padding]);
    }

    for (let offset = 0; offset < 512; offset += 32) {
      const currentAddress = address + offset;

      let data = Buffer.alloc(32);
      program.copy(data, 0, offset, offset + 32);

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
  }

  /**
   * @brief Parses a standard Intel HEX file asynchronously.
   * Validates checksums and handles variable record lengths.
   */
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

    const memoryMap = new Map<number, number>(); // Address -> Byte
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

        // Parse Record Structure: :LLAAAATT[DD...]CC
        const byteCount = parseInt(line.substr(1, 2), 16);
        const address = parseInt(line.substr(3, 4), 16);
        const recordType = parseInt(line.substr(7, 2), 16);
        const checksum = parseInt(line.substr(line.length - 2, 2), 16);

        // 1. Checksum Validation
        let calculatedChecksum = byteCount + (address >> 8) + (address & 0xFF) + recordType;

        for (let i = 0; i < byteCount; i++) {
          const byte = parseInt(line.substr(9 + (i * 2), 2), 16);
          calculatedChecksum += byte;
        }
        // Checksum is two's complement of the LSB of the sum
        if (((calculatedChecksum + checksum) & 0xFF) !== 0) {
          throw new Error(`Checksum mismatch at line ${lineNo}`);
        }

        // 2. Handle Record Types
        if (recordType === 0x00) { // Data Record
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

    // 3. Construct Buffer
    // FV-1 Banks are typically fixed size, but we support variable for safety.
    const size = maxAddress - minAddress + 1;
    // Enforce 512 byte minimum for FV-1 bank size
    const bufferSize = Math.max(512, size);
    const buffer = Buffer.alloc(bufferSize, 0xFF); // Fill with 0xFF (Empty)

    memoryMap.forEach((byte, addr) => {
      buffer[addr - minAddress] = byte;
    });

    Logs.log(LogType.INFO, `Parsed HEX. Base Address: 0x${minAddress.toString(16)} | Size: ${size} bytes`);

    return {
      address: minAddress,
      data: buffer
    };
  }

  // ... Serial Communication Helpers (SendWriteOrder, SendAddress, etc.) ...

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

  private async readData(): Promise<Buffer> {
    return await this.sendMessage(Buffer.from([OrderCode.Read]), 32);
  }

  private async sendData(data: Buffer): Promise<boolean> {
    const response = await this.sendMessage(data, 1);
    return response[0] === ResponseCode.Ok;
  }

  private async sendMessage(payload: Buffer, expectedResponseSize: number, timeoutMs = 500): Promise<Buffer> {
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
