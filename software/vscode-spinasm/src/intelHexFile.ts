import * as fs from "fs/promises";
import { IntelHexData, parseIntelHex } from "./intelHex";
import Logs, { LogType } from "./logs";

/** Reads and parses an Intel HEX file without loading the serial transport. */
export async function readIntelHexData(file: string): Promise<IntelHexData> {
  try {
    await fs.access(file);
  }
  catch {
    throw new Error(`Unable to open file: ${file}`);
  }

  Logs.log(LogType.INFO, `Reading HEX file: ${file}`);
  const content = await fs.readFile(file, { encoding: "utf8" });
  const parsed = parseIntelHex(content);

  Logs.log(
    LogType.INFO,
    `Parsed HEX. Base Address: 0x${parsed.address.toString(16)} | Size: ${parsed.data.length} bytes`
  );
  return parsed;
}
