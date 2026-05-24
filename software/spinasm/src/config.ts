import * as vscode from "vscode";

export default class Config {

  public static getCompilerPath(): string {
    // No shell quoting. The path goes to cp.spawn, which doesn't invoke a
    // shell — quoting would make spawn try to exec a path with literal `"`s.
    const config = vscode.workspace.getConfiguration("spinasm");
    return config.get<string>("compiler.path", "");
  }

  public static getCompilerArgs(): string[] {
    const config = vscode.workspace.getConfiguration("spinasm");
    return config.get<string[]>("compiler.args", ["-s"]);
  }

  public static getSerialPort(): string {
    const config = vscode.workspace.getConfiguration("spinasm");
    return config.get<string>("programmer.serialPort", "");
  }

  public static getBaudRate(): number {
    const config = vscode.workspace.getConfiguration("spinasm");
    return config.get<number>("programmer.baudRate", 57600);
  }

  public static getCompileOnSave(): boolean {
    const config = vscode.workspace.getConfiguration("spinasm");
    return config.get<boolean>("editor.compileOnSave", false);
  }

  public static getStatusBarEnabled(): boolean {
    const config = vscode.workspace.getConfiguration("spinasm");
    return config.get<boolean>("statusBar.enabled", true);
  }

  public static async setSerialPort(port: string, global: boolean = false): Promise<void> {
    const config = vscode.workspace.getConfiguration("spinasm");
    await config.update("programmer.serialPort", port, global);
  }

  public static isConfigMissing(): boolean {
    return !this.getCompilerPath() || !this.getSerialPort();
  }
}
