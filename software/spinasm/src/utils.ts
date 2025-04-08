import * as os from "os";

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
}
