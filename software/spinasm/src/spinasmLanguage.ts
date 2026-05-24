import { REGISTER_COUNT } from "./fv1Constants";

export const INSTRUCTIONS = new Set([
  'WRA', 'RDA', 'RDAX', 'WRAX', 'SOF', 'AND', 'OR', 'XOR',
  'SKP', 'LDAX', 'LOG', 'EXP', 'MULX', 'RDFX', 'WRLX', 'WRHX',
  'WRAP', 'CHO', 'WLDS', 'WLDR', 'JAM', 'NOT', 'CLR', 'ABSA',
  'RMPA', 'MAXX', 'NOP', 'JMP', 'RAW'
]);

// Accepts an optional `label:` prefix so `start: SOF 0,0` counts as an
// instruction just like `SOF 0,0` does.
const INSTRUCTION_REGEX = new RegExp(
  `^\\s*(?:[a-zA-Z_][a-zA-Z0-9_]*:\\s*)?\\b(${Array.from(INSTRUCTIONS).join('|')})\\b`, 'i'
);

export const BUILT_IN_SYMBOLS = new Set([
  'ADCL', 'ADCR', 'DACL', 'DACR',
  'POT0', 'POT1', 'POT2',
  'SIN0', 'SIN1', 'RMP0', 'RMP1',
  'SIN0_RATE', 'SIN0_RANGE', 'SIN1_RATE', 'SIN1_RANGE',
  'RMP0_RATE', 'RMP0_RANGE', 'RMP1_RATE', 'RMP1_RANGE',
  'ADDR_PTR',
  'SIN', 'COS', 'REG', 'COMPC', 'COMPA', 'RPTR2', 'NA',
  'RUN', 'ZRC', 'ZRO', 'GEZ', 'NEG',
  'RDA', 'SOF', 'RDAL'
]);

for (let i = 0; i < REGISTER_COUNT; i++) {
  BUILT_IN_SYMBOLS.add(`REG${i}`);
}

export function isInstruction(line: string): boolean {
  if (!line.trim()) { return false; }
  if (/^\s*(equ|mem)\b/i.test(line)) { return false; }
  if (/^\s*[a-zA-Z_][a-zA-Z0-9_]*:\s*$/.test(line)) { return false; }
  return INSTRUCTION_REGEX.test(line);
}

export function isBuiltInSymbol(symbol: string): boolean {
  if (/^REG\d+$/i.test(symbol)) { return true; }
  return BUILT_IN_SYMBOLS.has(symbol.toUpperCase());
}

