import * as vscode from "vscode";
import Utils from "./utils";

/**
 * @class Config
 * @brief Static access to SpinASM VSCode configuration.
 */
export default class Config {

  /**
   * @brief Retrieves the compiler executable path from VSCode settings.
   * @returns Sanitized path string.
   */
  public static getCompilerPath(): string {
    const config = vscode.workspace.getConfiguration("spinasm");
    const pathStr = config.get<string>("compiler.path", "");

    return Utils.sanitizePath(pathStr);
  }

  /**
   * @brief Retrieves compiler arguments.
   * @returns Array of string arguments.
   */
  public static getCompilerArgs(): string[] {
    const config = vscode.workspace.getConfiguration("spinasm");

    return config.get<string[]>("compiler.args", ["-s"]);
  }

  /**
   * @brief Retrieves the configured serial port.
   */
  public static getSerialPort(): string {
    const config = vscode.workspace.getConfiguration("spinasm");

    return config.get<string>("programmer.serialPort", "");
  }

  /**
   * @brief Retrieves the configured baud rate.
   */
  public static getBaudRate(): number {
    const config = vscode.workspace.getConfiguration("spinasm");

    return config.get<number>("programmer.baudRate", 57600);
  }

  /**
   * @brief Checks if the essential configuration is missing.
   * @returns True if compiler path or serial port is unset.
   */
  public static isConfigMissing(): boolean {
    const path = this.getCompilerPath();
    const port = this.getSerialPort();

    return !path || !port;
  }
}
