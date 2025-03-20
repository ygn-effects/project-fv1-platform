import * as vscode from "vscode";
import Utils from "./utils";

/**
 * @enum LogType
 * @brief Enumerates the supported types of log messages.
 */
export enum LogType {
  INFO = 0,   ///< Informational log messages.
  ERROR = 1,  ///< Error log messages.
}

/**
 * @class Logs
 * @brief Provides centralized logging for the SpinASM VSCode extension.
 *
 * Handles the creation and management of a dedicated VSCode output channel
 * for logging informational and error messages with timestamps.
 */
export default class Logs {
  private static logChannel: vscode.OutputChannel | null = null; ///< VSCode Output Channel for logging.

  /**
   * @brief Initializes the logging output channel.
   *
   * Creates a dedicated VSCode output channel named "SpinASM" if not already created.
   * Should be called once during the extension activation phase.
   */
  public static createChannel(): void {
    if (!this.logChannel) {
      this.logChannel = vscode.window.createOutputChannel("SpinASM");
    }
  }

  /**
   * @brief Writes a log message to the output channel with timestamp and severity.
   *
   * Sanitizes error messages to remove redundant "Error:" prefixes, enhancing readability.
   * Ensures the output channel exists before attempting to log.
   *
   * @param type - Severity level of the log entry (`INFO` or `ERROR`).
   * @param message - Message content to log.
   *
   * @throws Error if the logging channel has not been created yet.
   */
  public static log(type: LogType, message: string): void {
    if (!this.logChannel) {
      throw new Error("Log channel is not created. Ensure Logs.createChannel() is called first.");
    }

    const timestamp = Utils.getFormattedDate();

    switch (type) {
      case LogType.INFO:
        this.logChannel.appendLine(`${timestamp} | INFO  | ${message}`);
        break;

      case LogType.ERROR:
        // Remove redundant "Error: " prefix to keep logs clean.
        const sanitizedMessage = message.replace(/^Error:\s*/, "");
        this.logChannel.appendLine(`${timestamp} | ERROR | ${sanitizedMessage}`);
        break;

      default:
        // Fallback for unrecognized log types.
        this.logChannel.appendLine(`${timestamp} | UNKNOWN | ${message}`);
        break;
    }
  }

  /**
   * @brief Disposes the logging channel resources.
   *
   * Should be called during the extension deactivation phase to free resources.
   */
  public static disposeChannel(): void {
    if (this.logChannel) {
      this.logChannel.dispose();
      this.logChannel = null;
    }
  }
}
