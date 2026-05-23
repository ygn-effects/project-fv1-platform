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
 * @brief Serial-port helpers for discovering and probing the FV-1 programmer.
 */
export default class Utils {
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