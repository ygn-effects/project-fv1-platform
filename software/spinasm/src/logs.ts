import * as vscode from "vscode";
import Utils from "./utils";

/**
 * @class Logs
 * @brief Handles logging for the SpinASM extension, with support for info and error messages.
 */
export default class Logs {
  private static logChannel: vscode.OutputChannel;

  /**
   * @brief Creates the output channel for logging.
   */
  public static createChannel(): void {
    if (!this.logChannel) {
      this.logChannel = vscode.window.createOutputChannel("SpinASM");
    }
  }

  /**
   * @brief Logs a message to the output channel.
   * @param type - The type of log message. 0 = INFO, 1 = ERROR.
   * @param message - The message to log.
   */
  public static log(type: LogType, message: string): void {
    if (!this.logChannel) {
      throw new Error("Log channel is not created. Call createChannel() first.");
    }

    const timestamp = Utils.getFormattedDate();
    switch (type) {
      case LogType.INFO:
        this.logChannel.appendLine(`${timestamp} | INFO  | ${message}`);
        break;

      case LogType.ERROR:
        const sanitizedMessage = message.replace(/Error: /g, "");
        this.logChannel.appendLine(`${timestamp} | ERROR | ${sanitizedMessage}`);
        break;

      default:
        this.logChannel.appendLine(`${timestamp} | UNKNOWN | ${message}`);
        break;
    }
  }
}

/**
 * @enum LogType
 * @brief Enum for log message types.
 */
export enum LogType {
  INFO = 0,
  ERROR = 1,
}
