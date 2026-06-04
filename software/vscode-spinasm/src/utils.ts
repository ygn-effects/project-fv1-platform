import { SerialPort } from "serialport";
import Programmer from "./programmer";

export interface SerialPortInfo {
  path: string;
  manufacturer?: string;
  friendlyName?: string;
  vendorId?: string;
  productId?: string;
}

export default class Utils {
  public static async listSerialPorts(): Promise<SerialPortInfo[]> {
    const ports = await SerialPort.list();

    return ports.map(port => ({
      path: port.path,
      manufacturer: port.manufacturer,
      vendorId: port.vendorId,
      productId: port.productId
    }));
  }

  /** Probes each serial port with a RuThere command; returns the first that answers. */
  public static async detectProgrammer(baudRate: number = 57600): Promise<string | null> {
    const ports = await this.listSerialPorts();

    for (const portInfo of ports) {
      let programmer: Programmer | null = null;

      try {
        programmer = new Programmer(portInfo.path, baudRate);
        await programmer.connect();

        if (await programmer.isProgrammerConnected()) {
          await programmer.disconnect();
          return portInfo.path;
        }

        await programmer.disconnect();
      }
      catch {
        if (programmer) {
          try {
            await programmer.disconnect();
          }
          catch {
            // swallowed: not our port, nothing to clean up
          }
        }
      }
    }

    return null;
  }
}
