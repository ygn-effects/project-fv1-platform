import * as vscode from "vscode";
import Config from "./config";

function getFormattedDate(): string {
  const d = new Date();
  return `${d.getFullYear()}-${('0' + (d.getMonth() + 1)).slice(-2)}-${('0' + d.getDate()).slice(-2)} ` +
         `${('0' + d.getHours()).slice(-2)}:${('0' + d.getMinutes()).slice(-2)}:${('0' + d.getSeconds()).slice(-2)}` +
         `.${('00' + d.getMilliseconds()).slice(-3)}`;
}

export enum LogType {
  DEBUG = 0,
  INFO = 1,
  WARNING = 2,
  ERROR = 3,
}

export default class Logs {
  private static logChannel: vscode.OutputChannel | null = null;

  public static createChannel(): void {
    if (!this.logChannel) {
      this.logChannel = vscode.window.createOutputChannel("SpinASM");
    }
  }

  public static log(type: LogType, message: string): void {
    // DEBUG is high-volume (every serial byte) — drop unless explicitly opted in.
    if (type === LogType.DEBUG && !Config.getVerboseLogging()) {
      return;
    }

    // Lazy-create so loggers that fire before activate() called createChannel
    // don't crash.
    if (!this.logChannel) {
      this.createChannel();
    }
    const channel = this.logChannel!;

    const timestamp = getFormattedDate();

    switch (type) {
      case LogType.DEBUG:
        channel.appendLine(`${timestamp} | DEBUG | ${message}`);
        break;

      case LogType.INFO:
        channel.appendLine(`${timestamp} | INFO  | ${message}`);
        break;

      case LogType.WARNING:
        channel.appendLine(`${timestamp} | WARN  | ${message}`);
        break;

      case LogType.ERROR:
        const sanitizedMessage = message.replace(/^Error:\s*/, "");
        channel.appendLine(`${timestamp} | ERROR | ${sanitizedMessage}`);
        break;

      default:
        channel.appendLine(`${timestamp} | UNKNOWN | ${message}`);
        break;
    }
  }

  public static show(): void {
    if (this.logChannel) {
      this.logChannel.show(true); // preserveFocus
    }
  }

  public static disposeChannel(): void {
    if (this.logChannel) {
      this.logChannel.dispose();
      this.logChannel = null;
    }
  }
}
