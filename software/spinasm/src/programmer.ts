import { DelimiterParser, SerialPort } from "serialport";
import Logs, { LogType } from "./logs";
import { randomBytes } from "crypto";
import * as fs from 'fs';

interface IntelHexData {
  address: number;   // Start address from HEX file
  offset: number;    // Current offset (for internal use)
  data: Buffer;      // Data buffer (512 bytes)
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
  Nok          = 0x06, ///< Operation failed
  Ok           = 0x07, ///< Operation successful
  Timeout      = 0x08, ///< Operation timed out
  WriteError   = 0x09, ///< Write error
  ReadError    = 0x0A, ///< Read error
  ComError     = 0x0B, ///< Communication error
  FramingError = 0x0C, ///< Framing error
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

  /**
   * @brief Constructs a Programmer instance.
   * @param port - Serial port name.
   * @param baudRate - Communication baud rate.
   */
  constructor(port: string, baudRate: number) {
    this.serialPort = new SerialPort({ path: port, baudRate, autoOpen: false });
    this.parser = this.serialPort.pipe(new DelimiterParser({ delimiter: Buffer.from([this.endMarker]) }));
  }

    /**
   * @brief Opens the serial port connection to the programmer.
   */
  public async connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      this.serialPort.open((err) => {
        if (err) {
          Logs.log(LogType.ERROR, `Failed to open serial port: ${err.message}`);
          return reject(err);
        }

        Logs.log(LogType.INFO, `Serial port ${this.serialPort.path} opened successfully.`);

        // Allow 100 ms to discard initial noise from the serial buffer
        const discardDuration = 100; // ms
        const startTime = Date.now();

        const discardData = (data: Buffer) => {
          Logs.log(LogType.INFO, `Discarding junk data during init: ${data.toString('hex')}`);
        };

        // Attach temporary listener to discard incoming junk data
        this.serialPort.on("data", discardData);

        // After discard duration, remove the listener and proceed
        setTimeout(() => {
          this.serialPort.removeListener("data", discardData);
          Logs.log(LogType.INFO, `Initial junk data discard completed (${Date.now() - startTime} ms).`);
          resolve();
        }, discardDuration);
      });
    });
  }

  /**
   * @brief Closes the serial port connection.
   */
  public async disconnect(): Promise<void> {
    return new Promise((resolve, reject) => {
      if (!this.serialPort.isOpen) {
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

  /**
   * @brief Sends a "RuThere" command to verify programmer connection.
   * @returns True if programmer responds with OK.
   */
  public async isProgrammerConnected(): Promise<boolean> {
    const response = await this.sendMessage(Buffer.from([OrderCode.RuThere]), 1);

    switch (response[0]) {
      case ResponseCode.Ok:
        Logs.log(LogType.INFO, "Programmer is ready.");
        return true;
      case ResponseCode.Nok:
        Logs.log(LogType.ERROR, "Programmer is not ready.");
        return false;
      default:
        Logs.log(LogType.ERROR, `Unexpected response: ${response[0]}`);
        return false;
    }
  }

  /**
   * @brief Sends a "RuReady" command to check EEPROM readiness.
   * @returns True if EEPROM responds with OK (ready).
   */
  public async isEepromReady(): Promise<boolean> {
    const response = await this.sendMessage(Buffer.from([OrderCode.RuReady]), 1);

    switch (response[0]) {
      case ResponseCode.Ok:
        Logs.log(LogType.INFO, "EEPROM is ready.");
        return true;
      case ResponseCode.Nok:
        Logs.log(LogType.ERROR, "EEPROM is not ready.");
        return false;
      default:
        Logs.log(LogType.ERROR, `Unexpected response: ${response[0]}`);
        return false;
    }
  }

  /**
   * @brief Reads a 512 bytes program at the given address
   * @param address Program address
   * @returns A 512 bytes buffer containing the program
   */
  public async readProgram(address: number): Promise<Buffer> {
    let program = Buffer.alloc(512);

    // Process the read by 32 bytes pages increment.
    for (let offset = 0; offset < 512; offset += 32) {
      const currentAddress = address + offset;

      if (! (await this.sendReadOrder())) {
        throw new Error("Failed to send READ order to programmer.");
      }

      if (! (await this.sendAddress(currentAddress))) {
        throw new Error("Failed to send address 0 to programmer.");
      }

      let data = Buffer.alloc(32);
      data = await this.readData();
      data.copy(program, offset, 0, 32);
    }

    return program;
  }

  /**
   * @brief Writes a 512 bytes program at the given address
   * @param address Program address
   * @param program A 512 bytes buffer containing the program to write
   */
  public async writeProgram(address: number, program: Buffer): Promise<void> {
    // Process the write by 32 bytes pages increment.
    for (let offset = 0; offset < 512; offset += 32) {
      const currentAddress = address + offset;

      let data = Buffer.alloc(32);
      program.copy(data, 0, offset, offset + 32);

      if (! (await this.sendWriteOrder())) {
        throw new Error("Failed to send WRITE order to programmer.");
      }

      if (! (await this.sendAddress(currentAddress))) {
        throw new Error("Failed to send address 0 to programmer.");
      }

      if (! (await this.sendWriteOrder())) {
        throw new Error("Failed to send WRITE order to programmer.");
      }

      if (! (await this.sendData(data))) {
        throw new Error("Failed to send data.");
      }

      if (! (await this.sendWriteOrder())) {
        throw new Error("Failed to send WRITE order to programmer.");
      }
    }
  }

  /**
   * @brief Reads a compiled Intel HEX program file and returns a 512-byte buffer to write to the EEPROM
   * @param file Path of the HEX file to read
   * @returns IntelHexData containing start address and data buffer
   * @throws Error if file doesn't exist or parsing fails
   */
  public readIntelHexData(file: string): IntelHexData {
    if (!fs.existsSync(file)) {
      throw new Error(`Unable to open file: ${file}`);
    }

    try {
      const data = fs.readFileSync(file, { encoding: 'utf8' });
      const lines = data.split(/\r\n|\r|\n/); // Split file contents by lines

      const result: IntelHexData = {
        address: parseInt(lines[0].substr(3, 4), 16),
        offset: 0,
        data: Buffer.alloc(512),
      };

      lines.forEach((line: string) => {
        const startCode = line.charAt(0);
        const byteCount = parseInt(line.substr(1, 2), 16);
        const recordType = parseInt(line.substr(7, 2), 16);

        if (startCode === ':' && byteCount === 4 && recordType === 0) { // Data record
          for (let i = 9; i < 9 + byteCount * 2; i += 2) {
            result.data[result.offset] = parseInt(line.substr(i, 2), 16);
            result.offset++;
          }
        }
      });

      return result;
    }
    catch (error) {
      throw new Error(`Error reading Intel HEX file: ${(error as Error).message}`);
    }
  }

  /**
   * @brief Sends a "Write" command to initiate a write operation.
   * @returns True if the order is accepted.
   */
  private async sendWriteOrder(): Promise<boolean> {
    const response = await this.sendMessage(Buffer.from([OrderCode.Write]), 1);

    switch (response[0]) {
      case ResponseCode.Ok:
        Logs.log(LogType.INFO, "Write order accepted.");
        return true;
      case ResponseCode.Nok:
        Logs.log(LogType.ERROR, "Write order rejected.");
        return false;
      default:
        Logs.log(LogType.ERROR, `Unexpected response: ${response[0]}`);
        return false;
    }
  }

  /**
   * @brief Sends a "Read" command to initiate a read operation.
   * @returns True if the order is accepted.
   */
  private async sendReadOrder(): Promise<boolean> {
    const response = await this.sendMessage(Buffer.from([OrderCode.Read]), 1);

    switch (response[0]) {
      case ResponseCode.Ok:
        Logs.log(LogType.INFO, "Read order accepted.");
        return true;
      case ResponseCode.Nok:
        Logs.log(LogType.ERROR, "Read order rejected.");
        return false;
      default:
        Logs.log(LogType.ERROR, `Unexpected response: ${response[0]}`);
        return false;
    }
  }

  /**
   * @brief Sends the address to write to or read from.
   * @returns True if the address is accepted.
   */
  private async sendAddress(address: number): Promise<boolean> {
    const response = await this.sendMessage(Buffer.from([(address >> 8) & 0xFF, address & 0xFF]), 1);

    switch (response[0]) {
      case ResponseCode.Ok:
        Logs.log(LogType.INFO, "Address accepted.");
        return true;
      case ResponseCode.Nok:
        Logs.log(LogType.ERROR, "Address rejected.");
        return false;
      default:
        Logs.log(LogType.ERROR, `Unexpected response: ${response[0]}`);
        return false;
    }
  }

  /**
   * @brief Triggers a 32 bytes page read.
   * @returns A 32 bytes buffer containing the data read.
   */
  private async readData(): Promise<Buffer> {
    return await this.sendMessage(Buffer.from([OrderCode.Read]), 32);
  }

  /**
   * @brief Sends the data to write, typically a 32 bytes buffer
   * @returns True if the write suceeded.
   */
  private async sendData(data: Buffer): Promise<boolean> {
    const response = await this.sendMessage(data, 1);

    switch (response[0]) {
      case ResponseCode.Ok:
        Logs.log(LogType.INFO, "Data accepted.");
        return true;
      case ResponseCode.Nok:
        Logs.log(LogType.ERROR, "Data rejected.");
        return false;
      default:
        Logs.log(LogType.ERROR, `Unexpected response: ${response[0]}`);
        return false;
    }
  }

  private async sendEndOrder(): Promise<boolean> {
    const message = Buffer.from([OrderCode.End]);
    const response = await this.sendMessage(message, 1);

    if (response[0] === ResponseCode.Ok) {
      return true;
    }
    else if (response[0] === ResponseCode.Nok) {
      return false;
    }

    return false;
  }

  /**
 * @brief Sends an arbitrary message to the programmer and awaits the response.
 * @param payload - Message payload without framing markers.
 * @param expectedResponseSize - Expected size of the response payload (excluding framing markers).
 * @param timeoutMs - Timeout in milliseconds for the response (default: 500ms).
 * @returns A Buffer containing the response payload.
 */
  private async sendMessage(payload: Buffer, expectedResponseSize: number, timeoutMs = 500): Promise<Buffer> {
    // Ensure proper timing between commands
    const now = Date.now();
    const elapsed = now - this.lastCommandTimestamp;
    const requiredDelay = 20;

    if (elapsed < requiredDelay) {
      await new Promise((resolve) => setTimeout(resolve, requiredDelay - elapsed));
    }

    this.lastCommandTimestamp = Date.now();

    // Prepare the complete message with start/end markers
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

        // Note: DelimiterParser removes only the delimiter (endMarker),
        // so data includes startMarker + payload. Thus expected length = payload + 1.
        if (data.length !== expectedResponseSize + 1 || data[0] !== this.startMarker) {
          Logs.log(LogType.ERROR,`Invalid response format: ${data.toString("hex")}`);

          return reject(new Error("Invalid response format from programmer."));
        }
        else if (data.length === 2 && data[1] === ResponseCode.Timeout) {
          Logs.log(LogType.ERROR, "Programmer timed out during operation.");
          return reject(new Error("Programmer timed out during operation."));
        }
        else if (data.length === 2 && data[1] === ResponseCode.FramingError) {
          Logs.log(LogType.ERROR, "Programmer reported a framing error on received message.");
          return reject(new Error("Programmer reported a framing error on received message."));
        }
        else if (data.length === 2 && data[1] === ResponseCode.ComError) {
          Logs.log(LogType.ERROR, "Programmer reported a communication error.");
          return reject(new Error("Programmer reported a communication error."));
        }

        // Extract the response payload (excluding markers)
        const responsePayload = data.slice(1, data.length);
        resolve(responsePayload);
      });

      // Send message
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

  /**
   * @brief Generates a 512 bytes array with random bytes for testing purpose.
   * @returns A 512 bytes array containing a random program.
   */
  private generateRandomProgram(): Buffer {
      const buffer = randomBytes(512);

      // Iterate through the buffer and replace unwanted values
      for (let i = 0; i < buffer.length; i++) {
          while (buffer[i] === 30 || buffer[i] === 31) {
              buffer[i] = randomBytes(1)[0];
          }
      }

      return buffer;
  }

  /**
   * @brief Test method.
   */
  async test(): Promise<void> {
    const programWrite = this.generateRandomProgram();
    Logs.log(LogType.INFO, `Program to write : ${programWrite.toString("hex")}`);
    await this.writeProgram(0, programWrite);

    let programRead = Buffer.alloc(512);
    programRead = await this.readProgram(0);
    Logs.log(LogType.INFO, `Program read : ${programRead.toString("hex")}`);

    const result = Buffer.compare(programWrite, programRead);
    Logs.log(LogType.INFO, `Comparison result : ${result}`);
  }
}
