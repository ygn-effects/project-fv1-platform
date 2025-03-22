import { ByteLengthParser, SerialPort } from "serialport";
import Logs, { LogType } from "./logs";

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
  Nok          = 0x06, ///< Operation successful
  Ok           = 0x07, ///< Operation failed
  Timeout      = 0x08, ///< Operation timed out
  WriteError   = 0x09, ///< Write error
  ReadError    = 0x0A, ///< Read error
  ComError     = 0x0B, ///< Communication error
  BracingError = 0x0C, ///< Bracing error
}

/**
 * @class Programmer
 * @brief Manages communication with EEPROM programmer hardware via serial port.
 */
export default class Programmer {
  private serialPort: SerialPort;
  private parser: ByteLengthParser;
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
    this.parser = this.serialPort.pipe(new ByteLengthParser({ length: 3 }));  }

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

        // Allow 500 ms to discard initial noise from the serial buffer
        const discardDuration = 500; // ms
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
      if (!this.serialPort.isOpen) return resolve();

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
    const response = await this.sendCommand(OrderCode.RuThere);
    if (response === ResponseCode.Ok) {
      Logs.log(LogType.INFO, "Programmer is connected.");
      return true;
    }

    Logs.log(LogType.ERROR, "Programmer not responding correctly.");
    return false;
  }

  /**
   * @brief Sends a "RuReady" command to check EEPROM readiness.
   * @returns True if EEPROM responds with OK (ready).
   */
  public async isEepromReady(): Promise<boolean> {
    const response = await this.sendCommand(OrderCode.RuReady);
    if (response === ResponseCode.Ok) {
      Logs.log(LogType.INFO, "EEPROM is ready.");
      return true;
    } else if (response === ResponseCode.Nok) {
      Logs.log(LogType.ERROR, "EEPROM is not ready.");
      return false;
    }

    Logs.log(LogType.ERROR, `Unexpected EEPROM response: ${response}`);
    return false;
  }

  /**
   * @brief Sends a command to the programmer and awaits response.
   * @param order - Order code to send.
   * @returns The response code received from the programmer.
   */
  private async sendCommand(order: OrderCode): Promise<number> {
    const now = Date.now();
    const elapsed = now - this.lastCommandTimestamp;
    const requiredDelay = 500; // Adjust as needed

    if (elapsed < requiredDelay) {
      await new Promise(resolve => setTimeout(resolve, requiredDelay - elapsed));
    }

    this.lastCommandTimestamp = Date.now();

    // Your existing sendCommand logic here...
    return new Promise((resolve, reject) => {
      const message = Buffer.from([this.startMarker, order, this.endMarker]);

      const timeout = setTimeout(() => {
        this.parser.removeAllListeners("data");
        Logs.log(LogType.ERROR, "Timeout waiting for programmer response.");
        reject(new Error("Timeout waiting for programmer response."));
      }, 2000);

      this.parser.once("data", (data: Buffer) => {
        clearTimeout(timeout);

        if (data.length !== 3 || data[0] !== this.startMarker || data[2] !== this.endMarker) {
          Logs.log(LogType.ERROR, `Invalid response format: ${data.toString('hex')}`);
          return reject(new Error("Invalid response format from programmer."));
        }

        resolve(data[1]);
      });

      this.serialPort.write(message, (err) => {
        if (err) {
          clearTimeout(timeout);
          this.parser.removeAllListeners("data");
          Logs.log(LogType.ERROR, `Failed to write command: ${err.message}`);
          reject(err);
        } else {
          Logs.log(LogType.INFO, `Command sent: 0x${order.toString(16)}`);
        }
      });
    });
  }
}
