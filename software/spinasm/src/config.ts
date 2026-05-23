import * as vscode from "vscode";

/**
 * @class Config
 * @brief Static access to SpinASM VSCode configuration.
 */
export default class Config {

  /**
   * @brief Retrieves the compiler executable path from VSCode settings.
   *
   * Returned as-is — no shell quoting. The path is handed to cp.spawn, which
   * doesn't invoke a shell and handles spaces correctly on its own. Quoting
   * here would make spawn try to exec a path with literal `"` characters.
   */
  public static getCompilerPath(): string {
    const config = vscode.workspace.getConfiguration("spinasm");
    return config.get<string>("compiler.path", "");
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
   * @brief Retrieves the compile-on-save setting.
   */
  public static getCompileOnSave(): boolean {
    const config = vscode.workspace.getConfiguration("spinasm");

    return config.get<boolean>("editor.compileOnSave", false);
  }

  /**
   * @brief Retrieves the status bar enabled setting.
   */
  public static getStatusBarEnabled(): boolean {
    const config = vscode.workspace.getConfiguration("spinasm");

    return config.get<boolean>("statusBar.enabled", true);
  }

  /**
   * @brief Updates the serial port configuration.
   * @param port - The serial port path to save.
   * @param global - If true, saves to user settings; otherwise workspace settings.
   */
  public static async setSerialPort(port: string, global: boolean = false): Promise<void> {
    const config = vscode.workspace.getConfiguration("spinasm");
    await config.update("programmer.serialPort", port, global);
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