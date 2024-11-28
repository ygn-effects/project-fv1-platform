import * as fs from "fs";
import * as ini from "ini";
import Utils from "./utils";

/**
 * @class Config
 * @brief Handles reading and parsing the settings.ini file.
 */
export default class Config {
  private iniFile: string;
  private config: ConfigObject | null = null;

  constructor(file: string) {
    this.iniFile = file;
  }

  /**
   * @brief Reads the ini file and parses its contents into a ConfigObject.
   * @throws If the file doesn't exist or cannot be read.
   */
  public readConfigFile(): void {
    try {
      if (fs.existsSync(this.iniFile)) {
        this.config = ini.parse(fs.readFileSync(this.iniFile, "utf-8")) as ConfigObject;
      }
      else {
        throw new Error(`Ini file not found: ${this.iniFile}. Please recreate the project.`);
      }
    }
    catch (error) {
      throw new Error(`Failed to read ini file: ${(error as Error).message}`);
    }
  }

  /**
   * @brief Returns the sanitized compiler path from the ini file.
   * @returns The sanitized compiler path.
   * @throws If the compiler path is invalid or the ini file is missing.
   */
  public readCompilerCommand(): string {
    this.ensureConfigLoaded();

    const strComp = this.config?.asfv1?.path?.trim();
    if (!strComp || !fs.existsSync(strComp)) {
      throw new Error(`Invalid compiler path: ${strComp}`);
    }

    return Utils.sanitizePath(strComp);
  }

  /**
   * @brief Returns the compiler options as an array of arguments.
   * @returns The compiler arguments as a string array.
   * @throws If the compiler options are invalid or the ini file is missing.
   */
  public readCompilerArgs(): string[] {
    this.ensureConfigLoaded();

    const strCompOpt = this.config?.asfv1?.options?.trim();
    if (!strCompOpt || !strCompOpt.match(/(\ ?(-q|-c|-s))*/)) {
      throw new Error("Invalid compiler options");
    }

    return strCompOpt.split(" ");
  }

  /**
   * @brief Returns the serial port value from the ini file.
   * @returns The serial port as a string.
   * @throws If the serial port is not set or the ini file is missing.
   */
  public readSerialPort(): string {
    this.ensureConfigLoaded();

    const port = this.config?.serial?.port?.trim();
    if (!port) {
      throw new Error("No serial port set in the config file.");
    }

    return port;
  }

  /**
   * @brief Returns the baud rate value from the ini file.
   * @returns The baud rate as a number.
   * @throws If the baud rate is not set or the ini file is missing.
   */
  public readBaudRate(): number {
    this.ensureConfigLoaded();

    const rate = this.config?.serial?.baudrate?.trim();
    if (!rate) {
      throw new Error("No baud rate set in the config file.");
    }

    return parseInt(rate, 10);
  }

  /**
   * @brief Ensures the ini file has been read and the config object is loaded.
   * @throws If the config object is null.
   */
  private ensureConfigLoaded(): void {
    if (!this.config) {
      this.readConfigFile();
    }
  }
}

/**
 * @interface ConfigObject
 * @brief Represents the structure of the parsed ini file.
 */
interface ConfigObject {
  asfv1?: {
    path: string;
    options: string;
  };
  serial?: {
    port: string;
    baudrate: string;
  };
}
