import * as os from "os";
import { SerialPort } from "serialport";
import Programmer from "./programmer";

/**
 * @interface SerialPortInfo
 * @brief Information about a detected serial port.
 */
export interface SerialPortInfo {
  path: string;
  manufacturer?: string;
  friendlyName?: string;
  vendorId?: string;
  productId?: string;
}

/**
 * @class Utils
 * @brief Utility functions for path sanitization and date formatting.
 */
export default class Utils {
  /**
   * @brief Cleans up folder paths for different operating systems.
   * @param path - The input folder path.
   * @returns A sanitized folder path.
   */
  public static sanitizePath(path: string): string {
    let returnPath: string;

    if (os.type() === "Windows_NT") {
      // Path includes spaces and no quotes at the start or end
      if (path.includes(" ") && !path.startsWith('"') && !path.endsWith('"')) {
         // Add quotes
        returnPath = `"${path}"`;
      }
      else {
        returnPath = path;
      }
    }
    else if (os.type() === "Linux" || os.type() === "Darwin") {
      // Do nothing for Linux or macOS
      returnPath = path;
    }
    else {
      // Fallback for unknown OS
      returnPath = path;
    }

    return returnPath;
  }

  /**
   * @brief Returns the current date and time in a formatted string.
   * @returns A string in the format `YYYY-MM-DD HH:mm:ss`.
   */
  public static getFormattedDate(): string {
    const d = new Date();

    const date = `${d.getFullYear()}-${('0' + (d.getMonth() + 1)).slice(-2)}-${('0' + d.getDate()).slice(-2)} ` +
                 `${('0' + d.getHours()).slice(-2)}:${('0' + d.getMinutes()).slice(-2)}:${('0' + d.getSeconds()).slice(-2)}`;

    return date;
  }

  /**
   * @brief Lists all available serial ports on the system.
   * @returns Array of serial port information objects.
   */
  public static async listSerialPorts(): Promise<SerialPortInfo[]> {
    const ports = await SerialPort.list();

    return ports.map(port => ({
      path: port.path,
      manufacturer: port.manufacturer,
      vendorId: port.vendorId,
      productId: port.productId
    }));
  }

  /**
   * @brief Attempts to detect the FV-1 programmer by probing available serial ports.
   * @param baudRate - The baud rate to use for detection (default: 57600).
   * @returns The path of the detected programmer, or null if not found.
   */
  public static async detectProgrammer(baudRate: number = 57600): Promise<string | null> {
    const ports = await this.listSerialPorts();

    for (const portInfo of ports) {
      let programmer: Programmer | null = null;

      try {
        programmer = new Programmer(portInfo.path, baudRate);
        await programmer.connect();

        // Check if it responds to RuThere command
        if (await programmer.isProgrammerConnected()) {
          await programmer.disconnect();

          return portInfo.path; // Found it!
        }

        await programmer.disconnect();
      }
      catch (error) {
        // Not this port, continue to next
        if (programmer) {
          try {
            await programmer.disconnect();
          }
          catch {
            // Ignore disconnect errors
          }
        }
      }
    }

    return null; // Couldn't auto-detect
  }
}