/**
 * Human-readable reference for FV-1 instructions and built-in symbols. Single
 * source of truth shared by the hover and completion providers. Pure data — no
 * vscode dependency — so it can be unit-tested and kept in sync with the
 * INSTRUCTIONS / BUILT_IN_SYMBOLS sets in spinasmLanguage.ts.
 *
 * Summaries are concise functional descriptions; ACC = accumulator,
 * PACC = previous ACC, C/D = coefficients, REG = register, ADDR = delay address.
 */

export interface DocEntry {
  /** Operand shape, e.g. "RDAX  REG, C". */
  signature: string;
  /** One-line summary. */
  summary: string;
}

/** Keyed by UPPERCASE mnemonic. */
export const INSTRUCTION_DOCS: ReadonlyMap<string, DocEntry> = new Map<string, DocEntry>([
  ["SOF",  { signature: "SOF   C, D",       summary: "Scale ACC by C and add constant D → ACC = C·ACC + D." }],
  ["AND",  { signature: "AND   MASK",       summary: "Bitwise AND of ACC with a 24-bit mask." }],
  ["OR",   { signature: "OR    MASK",       summary: "Bitwise OR of ACC with a 24-bit mask." }],
  ["XOR",  { signature: "XOR   MASK",       summary: "Bitwise XOR of ACC with a 24-bit mask." }],
  ["LOG",  { signature: "LOG   C, D",       summary: "ACC = C·log2(|ACC|) + D (base-2 logarithm)." }],
  ["EXP",  { signature: "EXP   C, D",       summary: "ACC = C·2^ACC + D (base-2 exponential)." }],
  ["SKP",  { signature: "SKP   CMASK, N",   summary: "Skip N instructions (or to a label) if all flagged conditions hold (RUN/ZRC/ZRO/GEZ/NEG)." }],
  ["RDAX", { signature: "RDAX  REG, C",     summary: "Read register into ACC → ACC = ACC + REG·C." }],
  ["WRAX", { signature: "WRAX  REG, C",     summary: "Write ACC to register, then scale → REG = ACC; ACC = ACC·C." }],
  ["MAXX", { signature: "MAXX  REG, C",     summary: "ACC = max(|ACC|, |REG·C|)." }],
  ["MULX", { signature: "MULX  REG",        summary: "Multiply ACC by register → ACC = ACC·REG." }],
  ["RDFX", { signature: "RDFX  REG, C",     summary: "Read register through a 1-pole filter → ACC = (ACC − REG)·C + REG." }],
  ["WRLX", { signature: "WRLX  REG, C",     summary: "Write ACC to register; shelving low → REG = ACC; ACC = (PACC − ACC)·C + PACC." }],
  ["WRHX", { signature: "WRHX  REG, C",     summary: "Write ACC to register; shelving high → REG = ACC; ACC = ACC·C + PACC." }],
  ["RDA",  { signature: "RDA   ADDR, C",    summary: "Read delay RAM into ACC → ACC = ACC + delay[ADDR]·C." }],
  ["RMPA", { signature: "RMPA  C",          summary: "Read delay RAM via ADDR_PTR → ACC = ACC + delay[ADDR_PTR]·C." }],
  ["WRA",  { signature: "WRA   ADDR, C",    summary: "Write ACC to delay RAM, then scale → delay[ADDR] = ACC; ACC = ACC·C." }],
  ["WRAP", { signature: "WRAP  ADDR, C",    summary: "Write ACC to delay RAM, scale and add last delay read (all-pass) → delay[ADDR] = ACC; ACC = ACC·C + LR." }],
  ["LDAX", { signature: "LDAX  REG",        summary: "Load ACC from register → ACC = REG." }],
  ["WLDS", { signature: "WLDS  SINn, F, A", summary: "Load a sine LFO (SIN0/SIN1) with frequency F and amplitude A." }],
  ["WLDR", { signature: "WLDR  RMPn, F, A", summary: "Load a ramp LFO (RMP0/RMP1) with frequency F and amplitude A." }],
  ["JAM",  { signature: "JAM   RMPn",       summary: "Reset a ramp LFO (RMP0/RMP1) to its start position." }],
  ["CHO",  { signature: "CHO   RDA, N, C, ADDR", summary: "LFO-modulated delay operation (CHO RDA / CHO SOF / CHO RDAL) for chorus, flange, pitch." }],
  ["NOT",  { signature: "NOT",              summary: "One's complement of ACC (bitwise NOT)." }],
  ["CLR",  { signature: "CLR",              summary: "Clear ACC to 0." }],
  ["ABSA", { signature: "ABSA",             summary: "ACC = |ACC| (absolute value)." }],
  ["NOP",  { signature: "NOP",              summary: "No operation (equivalent to SKP 0, 0)." }],
  ["JMP",  { signature: "JMP   N",          summary: "Unconditional forward skip to a label / by N (asfv1; assembles as SKP 0, N)." }],
  ["RAW",  { signature: "RAW   U32",        summary: "Emit a raw 32-bit machine word (asfv1 extension)." }],
]);

/** Keyed by UPPERCASE symbol. REG0..REG31 are resolved dynamically in getSymbolDoc. */
export const SYMBOL_DOCS: ReadonlyMap<string, DocEntry> = new Map<string, DocEntry>([
  ["ADCL", { signature: "ADCL", summary: "Left ADC audio input register (read)." }],
  ["ADCR", { signature: "ADCR", summary: "Right ADC audio input register (read)." }],
  ["DACL", { signature: "DACL", summary: "Left DAC audio output register (write)." }],
  ["DACR", { signature: "DACR", summary: "Right DAC audio output register (write)." }],

  ["POT0", { signature: "POT0", summary: "Potentiometer 0 input register (0…1, read)." }],
  ["POT1", { signature: "POT1", summary: "Potentiometer 1 input register (0…1, read)." }],
  ["POT2", { signature: "POT2", summary: "Potentiometer 2 input register (0…1, read)." }],

  ["ADDR_PTR", { signature: "ADDR_PTR", summary: "Delay-RAM address pointer register used by RMPA." }],

  ["SIN0", { signature: "SIN0", summary: "Sine LFO 0 selector (configure with WLDS)." }],
  ["SIN1", { signature: "SIN1", summary: "Sine LFO 1 selector (configure with WLDS)." }],
  ["RMP0", { signature: "RMP0", summary: "Ramp LFO 0 selector (configure with WLDR, reset with JAM)." }],
  ["RMP1", { signature: "RMP1", summary: "Ramp LFO 1 selector (configure with WLDR, reset with JAM)." }],

  ["SIN0_RATE",  { signature: "SIN0_RATE",  summary: "Sine LFO 0 rate (frequency) register." }],
  ["SIN0_RANGE", { signature: "SIN0_RANGE", summary: "Sine LFO 0 range (amplitude) register." }],
  ["SIN1_RATE",  { signature: "SIN1_RATE",  summary: "Sine LFO 1 rate (frequency) register." }],
  ["SIN1_RANGE", { signature: "SIN1_RANGE", summary: "Sine LFO 1 range (amplitude) register." }],
  ["RMP0_RATE",  { signature: "RMP0_RATE",  summary: "Ramp LFO 0 rate (frequency) register." }],
  ["RMP0_RANGE", { signature: "RMP0_RANGE", summary: "Ramp LFO 0 range (amplitude) register." }],
  ["RMP1_RATE",  { signature: "RMP1_RATE",  summary: "Ramp LFO 1 rate (frequency) register." }],
  ["RMP1_RANGE", { signature: "RMP1_RANGE", summary: "Ramp LFO 1 range (amplitude) register." }],

  ["RUN", { signature: "RUN", summary: "SKP condition: true on every pass except the first (set after the first sample)." }],
  ["ZRC", { signature: "ZRC", summary: "SKP condition: ACC sign differs from the previous sample (zero crossing)." }],
  ["ZRO", { signature: "ZRO", summary: "SKP condition: ACC is exactly zero." }],
  ["GEZ", { signature: "GEZ", summary: "SKP condition: ACC ≥ 0." }],
  ["NEG", { signature: "NEG", summary: "SKP condition: ACC < 0." }],

  ["SIN",   { signature: "SIN",   summary: "CHO flag: use the sine LFO output." }],
  ["COS",   { signature: "COS",   summary: "CHO flag: use the cosine (90°-shifted) LFO output." }],
  ["REG",   { signature: "REG",   summary: "CHO flag: register (hold) the LFO value for the next CHO." }],
  ["COMPC", { signature: "COMPC", summary: "CHO flag: use the complement of the interpolation coefficient (1 − C)." }],
  ["COMPA", { signature: "COMPA", summary: "CHO flag: complement the delay address." }],
  ["RPTR2", { signature: "RPTR2", summary: "CHO flag: add half the ramp period to the pointer (for the second tap)." }],
  ["NA",    { signature: "NA",    summary: "CHO flag: no address offset — read the raw LFO value." }],

  ["RDAL", { signature: "RDAL", summary: "Read the selected LFO value into ACC (CHO RDAL form)." }],
]);

export function getInstructionDoc(name: string): DocEntry | undefined {
  return INSTRUCTION_DOCS.get(name.toUpperCase());
}

export function getSymbolDoc(name: string): DocEntry | undefined {
  const upper = name.toUpperCase();
  const regMatch = /^REG(\d+)$/.exec(upper);
  if (regMatch) {
    return {
      signature: upper,
      summary: `General-purpose register ${regMatch[1]} (one of REG0…REG31).`,
    };
  }
  return SYMBOL_DOCS.get(upper);
}
