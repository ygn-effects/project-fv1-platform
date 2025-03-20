import * as fs from "fs";
import * as ini from "ini";
import Utils from "./utils";

/**
 * @interface ConfigObject
 * @brief Represents the structure of the parsed ini file.
 */
interface ConfigObject {
  asfv1: {
    path: string;      ///< Path to the compiler executable.
    options: string;   ///< Compiler command-line options.
  };
  serial: {
    port: string;      ///< Serial port for communication.
    baudrate: string;  ///< Baud rate for serial communication.
  };
}

/**
 * @class Config
 * @brief Manages reading and validation of configuration settings from an ini file.
 *
 * Responsible for parsing the project's configuration (`settings.ini`) and providing
 * access to compiler and serial port configurations in a type-safe manner.
 */
export default class Config {
  private iniFile: string;           ///< Path to the ini configuration file.
  private config: ConfigObject;      ///< Parsed configuration object.

  /**
   * @brief Constructs a new Config instance.
   * @param file - Path to the ini file to be loaded.
   * @throws Error if the ini file doesn't exist or contains invalid configurations.
   */
  constructor(file: string) {
    this.iniFile = file;
    this.config = this.readAndValidateConfig();
  }

  /**
   * @brief Reads, parses, and validates the configuration file.
   * @returns A validated ConfigObject.
   * @throws Error if the ini file is missing, unreadable, or contains invalid settings.
   */
  private readAndValidateConfig(): ConfigObject {
    if (!fs.existsSync(this.iniFile)) {
      throw new Error(`Ini file not found: ${this.iniFile}. Please recreate the project.`);
    }

    try {
      const config = ini.parse(fs.readFileSync(this.iniFile, "utf-8")) as ConfigObject;

      if (!fs.existsSync(config.asfv1.path)) {
        throw new Error(`Invalid compiler path in ini file: ${config.asfv1.path}`);
      }

      const allowedOptions = ['-q', '-c', '-s'];
      const options = config.asfv1.options.trim().split(" ");

      if (!options.every(opt => allowedOptions.includes(opt))) {
        throw new Error(`Invalid compiler options: ${config.asfv1.options}`);
      }

      if (!config.serial.port || !config.serial.baudrate) {
        throw new Error(`Incomplete serial settings in ini file.`);
      }

      return config;
    }
    catch (error) {
      throw new Error(`Failed to read ini file: ${(error as Error).message}`);
    }
  }

  /**
   * @brief Gets the compiler command from the configuration, sanitizing the path if necessary.
   * @returns A sanitized path to the compiler executable.
   */
  public readCompilerCommand(): string {
    return Utils.sanitizePath(this.config.asfv1.path);
  }

  /**
   * @brief Retrieves the compiler command-line arguments.
   * @returns An array of compiler options.
   */
  public readCompilerArgs(): string[] {
    return this.config.asfv1.options.trim().split(" ");
  }

  /**
   * @brief Retrieves the serial port from the configuration.
   * @returns The serial port identifier as a string.
   */
  public readSerialPort(): string {
    return this.config.serial.port.trim();
  }

  /**
   * @brief Retrieves the baud rate for serial communication from the configuration.
   * @returns The baud rate as a numeric value.
   */
  public readBaudRate(): number {
    return parseInt(this.config.serial.baudrate.trim(), 10);
  }
}
