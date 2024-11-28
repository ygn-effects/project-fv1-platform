import * as fs from "fs";
import * as sp from "serialport";
import { ByteLengthParser } from "@serialport/parser-byte-length";
import Logs, { LogType } from "./logs";

/**
 * @class Programmer
 * @brief Handles interaction with the programmer and EEPROM via a USB adapter.
 */
export default class Programmer {
  private serialPortPort: string;
  private serialPortBaud: number;
  private serialPort: sp.SerialPort;
  private readonly startMarker = 0x1e;
  private readonly endMarker = 0x1f;

  constructor(port: string, baud: number) {
    this.serialPortPort = port;
    this.serialPortBaud = baud;
    this.serialPort = new sp.SerialPort({ path: port, baudRate: baud, autoOpen: false });
  }

  /**
   * @brief Recreates the serial port object if the configuration changes.
   * @param port Serial port name.
   * @param baud Baud rate.
   */
  public programmerSetup(port: string, baud: number): void {
    if (this.serialPortPort !== port || this.serialPortBaud !== baud) {
      Logs.log(LogType.INFO, "Programmer configuration change, recreating port.");

      this.serialPortPort = port;
      this.serialPortBaud = baud;
      this.serialPort = new sp.SerialPort({ path: port, baudRate: baud, autoOpen: false });
    }
    else {
      Logs.log(LogType.INFO, "No programmer configuration change.");
    }

    Logs.log(LogType.INFO, `Programmer port: ${this.serialPortPort}`);
    Logs.log(LogType.INFO, `Programmer baud rate: ${this.serialPortBaud}`);
  }

  /**
   * @brief Checks if the programmer is connected and ready.
   */
  public async checkProgrammer(): Promise<void> {
    Logs.log(LogType.INFO, "Opening serial port...");

    await this.openSerialPort();
    const response = await this.sendOrder(0x01); // Order: ruthere

    if (response[0] === 99) {
      Logs.log(LogType.INFO, "Programmer connected and EEPROM ready.");
    }
    else if (response[0] === 98) {
      Logs.log(LogType.ERROR, "Programmer connected but EEPROM is not ready.");
    }
    else {
      Logs.log(LogType.ERROR, `Invalid response: ${response[0]}`);
    }

    await this.disconnectProgrammer();
  }

  /**
   * @brief Opens the serial port.
   */
  public async connectProgrammer(): Promise<void> {
    Logs.log(LogType.INFO, "Connecting programmer...");

    await this.openSerialPort();
    const response = await this.sendOrder(0x01); // Order: ruthere

    if (response[0] !== 99) {
      throw new Error("Programmer connected but EEPROM is not ready.");
    }
  }

  /**
   * @brief Disconnects the programmer.
   */
  public async disconnectProgrammer(): Promise<void> {
    if (this.serialPort.isOpen) {
      await this.closeSerialPort();

      Logs.log(LogType.INFO, `Programmer disconnected on port: ${this.serialPortPort}`);
    }
    else {
      Logs.log(LogType.INFO, "Serial port already closed.");
    }
  }

  /**
   * @brief Sends a write order to the programmer.
   */
  public async sendWriteOrder(): Promise<boolean> {
    const response = await this.sendOrder(0x04); // Order: write
    return response[0] === 99;
  }

  /**
   * @brief Sends a read order to the programmer.
   */
  public async sendReadOrder(): Promise<boolean> {
    const response = await this.sendOrder(0x03); // Order: read
    return response[0] === 99;
  }

  /**
   * @brief Sends an end order to the programmer.
   */
  public async sendEndOrder(): Promise<boolean> {
    const response = await this.sendOrder(0x05); // Order: end
    return response[0] === 99;
  }

  /**
   * @brief Sends a specific order to the programmer and waits for a response.
   * @param order The order code to send.
   * @returns The response buffer.
   */
  private async sendOrder(order: number): Promise<Buffer> {
    const data = Buffer.from([this.startMarker, order, this.endMarker]);
    return this.writeBufferReadBuffer1(data);
  }

  /**
   * @brief Sends an address to the programmer.
   * @param address Address to send.
   */
  public async sendAddress(address: number): Promise<boolean> {
    const data = Buffer.from([
      this.startMarker,
      (address >> 8) & 0xff, // High byte
      address & 0xff, // Low byte
      this.endMarker,
    ]);

    const response = await this.writeBufferReadBuffer1(data);
    return response[0] === 99;
  }

  /**
   * @brief Writes a program to the EEPROM in 32-byte pages.
   * @param programData 512-byte buffer representing the program.
   * @param startAddress Starting address in the EEPROM.
   */
  public async writeProgram(programData: Buffer, startAddress: number): Promise<void> {
    if (programData.length !== 512) {
      throw new Error("Program data must be 512 bytes.");
    }

    await this.sendWriteOrder();

    for (let pageStart = 0; pageStart < 512; pageStart += 32) {
      const pageData = programData.slice(pageStart, pageStart + 32);

      if (await this.sendAddress(startAddress + pageStart)) {
        const pageBuffer = Buffer.concat([Buffer.from([this.startMarker]), pageData, Buffer.from([this.endMarker])]);
        const response = await this.writeBufferReadBuffer1(pageBuffer);

        if (response[0] !== 99) {
          throw new Error(`Error writing page starting at address ${startAddress + pageStart}`);
        }
      }
      else {
        throw new Error("Error sending address.");
      }
    }

    if (!(await this.sendEndOrder())) {
      throw new Error("Error sending end order.");
    }
  }

  /**
   * @brief Reads a program from the EEPROM into a buffer.
   * @param startAddress Starting address in the EEPROM.
   * @returns A 512-byte buffer containing the program data.
   */
  public async readProgram(startAddress: number): Promise<Buffer> {
    const buffer = Buffer.alloc(512);
    await this.sendReadOrder();

    for (let pageStart = 0; pageStart < 512; pageStart += 32) {
      const pageData = await this.triggerReadPage(startAddress + pageStart);
      pageData.copy(buffer, pageStart);
    }

    if (!(await this.sendEndOrder())) {
      throw new Error("Error sending end order.");
    }

    return buffer;
  }

  /**
   * @brief Reads a 32-byte page from the EEPROM.
   * @param address Address to read from.
   * @returns A 32-byte buffer containing the page data.
   */
  private async triggerReadPage(address: number): Promise<Buffer> {
    const data = Buffer.from([
      this.startMarker,
      (address >> 8) & 0xff, // High byte
      address & 0xff, // Low byte
      this.endMarker,
    ]);
    return this.writeBufferReadBuffer32(data);
  }
  private writeBufferReadBuffer1(data: Buffer): Promise<Buffer> {
    const parser = this.serialPort.pipe(new ByteLengthParser({ length: 1 }));
    return this.writeBufferWithTimeout(data, parser);
  }

  private writeBufferReadBuffer32(data: Buffer): Promise<Buffer> {
    const parser = this.serialPort.pipe(new ByteLengthParser({ length: 32 }));
    return this.writeBufferWithTimeout(data, parser);
  }

  private writeBufferWithTimeout(data: Buffer, parser: ByteLengthParser): Promise<Buffer> {
    const timeout = new Promise<never>((_, reject) => {
      const tm = setTimeout(() => reject(new Error("Timeout while communicating with the EEPROM.")), 500);
      clearTimeout(tm);
    });

    const response = new Promise<Buffer>((resolve, reject) => {
      this.serialPort.write(data);
      parser.once("data", (response) => resolve(response));
      this.serialPort.once("error", (err) => reject(err));
    });

    return Promise.race([response, timeout]);
  }

  private openSerialPort(): Promise<void> {
    return new Promise((resolve, reject) => {
      this.serialPort.open((err) => (err ? reject(err) : resolve()));
    });
  }

  private closeSerialPort(): Promise<void> {
    return new Promise((resolve, reject) => {
      this.serialPort.close((err) => (err ? reject(err) : resolve()));
    });
  }

  /**
  * @brief Reads a compiled program file and returns its content as a structured object.
  */
  public readIntelHexData(file: string): IntelHexData {
    if (!fs.existsSync(file)) {
      throw new Error(`Unable to open file: ${file}`);
    }

    try {
      const data = fs.readFileSync(file, { encoding: "utf8" });
      const lines = data.split(/\r\n|\r|\n/);

      const result: IntelHexData = {
        address: parseInt(lines[0].substr(3, 4), 16),
        offset: 0,
        data: Buffer.alloc(512),
      };

      lines.forEach((line) => {
        const startCode = line[0];
        const byteCount = parseInt(line.substr(1, 2), 16);
        const recordType = parseInt(line.substr(7, 2), 16);

        if (startCode === ":" && byteCount === 4 && recordType === 0) {
          for (let i = 9; i < 9 + byteCount * 2; i += 2) {
            result.data[result.offset] = parseInt(line.substr(i, 2), 16);
            result.offset++;
          }
        }
      });

      return result;
    } catch (error) {
      throw new Error(`Error reading Intel HEX file: ${(error as Error).message}`);
    }
  }
}

/**
* @interface IntelHexData
* @brief Represents the structure of parsed Intel HEX file data.
*/
interface IntelHexData {
  address: number;
  offset: number;
  data: Buffer;
}
