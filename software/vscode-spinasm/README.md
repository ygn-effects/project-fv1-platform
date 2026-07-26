# SpinASM for VS Code

Build Spin Semiconductor FV-1 effects without leaving VS Code. SpinASM for VS Code adds language support for `.spn` programs, compiles them with [`asfv1`](https://pypi.org/project/asfv1/) and programs an on-board EEPROM through the optional YGN FV-1 EEPROM Programmer.

This extension is part of the [YGN Effects Framework](https://ygn-effects.com/framework/), an open-source platform that gives hobbyists the hardware, firmware and tools needed to build guitar and bass effects pedals. It provides the software side of the Framework's FV-1 platform, from writing DSP code to loading it onto a pedal.

## Features

- Syntax and semantic highlighting for SpinASM `.spn` files
- Completion and hover reference for FV-1 instructions, registers, flags and user-defined symbols
- Go to Definition for `EQU`, `MEM` and label declarations
- Live diagnostics for symbols, declarations, instruction syntax and FV-1 resource limits
- Instruction, register and delay-memory usage in the VS Code status bar
- Compiler warnings and errors shown as editor diagnostics
- Snippets for common FV-1 instructions and program structures
- Eight-bank project management with stale-output and ambiguous-bank detection
- Single-bank, all-bank and combined EEPROM image compilation through `asfv1`
- Optional compile on save
- Serial-port selection and automatic YGN programmer detection
- EEPROM programming with read-back verification
- Multi-root workspace support

## Requirements

- [Visual Studio Code](https://code.visualstudio.com/) 1.95 or newer
- [`asfv1`](https://pypi.org/project/asfv1/), installed separately and available as an executable
- Optional compatible programmer hardware for loading compiled programs directly from VS Code

Language support, diagnostics and compilation work without the programmer.

## Quick Start

1. Install the assembler by following the [`asfv1` system-specific installation instructions](https://github.com/ndf-zz/asfv1#system-specific-installation).
2. Install **SpinASM for VS Code** from the VS Code Extensions view.
3. Locate the installed assembler. Run `whereis asfv1` on Linux or macOS. On Windows, run `gcm asfv1.exe` from a PowerShell prompt.
4. Open VS Code Settings, search for `SpinASM` and set **Compiler: Path** to the full path reported for the `asfv1` executable.
5. Open a folder containing an FV-1 project, or run **SpinASM: Create Project** from the Command Palette to create one.
6. Open a `.spn` file and run **SpinASM: Compile current program**. The editor title also provides compile and compile-upload buttons.

## Documentation

For complete project setup, compilation, EEPROM programming and troubleshooting, see the [SpinASM for VS Code usage guide](https://ygn-effects.com/docs/vscode-spinasm-extension-usage/).

## Project Layout

Each FV-1 EEPROM contains eight program slots. The extension maps those slots to `bank_0` through `bank_7` inside the open workspace:

```text
my-fv1-project/
├── bank_0/
│   └── chorus.spn
├── bank_1/
│   └── delay.spn
├── ...
├── bank_7/
│   └── reverb.spn
└── output/
    ├── bank_0.hex
    ├── bank_1.hex
    ├── ...
    ├── bank_7.hex
    └── output.bin
```

Keep no more than one `.spn` file in each bank folder. If a bank contains more than one program, the extension reports the ambiguity and identifies the file it selected. Banks may be left empty in a manually created project.

Individual compilations produce Intel HEX files under `output/`. The **Compile all programs to .bin** command builds a single 4 KB `output.bin` image containing all eight banks.

## Compiling and Programming

The main workflows are available from the Command Palette. Commands for the current program are also available from the editor title and context menus.

| Command | What it does |
|---|---|
| **Compile current program** | Compiles the active bank to `output/bank_N.hex` |
| **Compile Specific Bank...** | Selects and compiles one bank |
| **Compile all programs** | Compiles every populated bank to Intel HEX |
| **Compile all programs to .bin** | Builds the combined 4 KB `output.bin` EEPROM image |
| **Upload current program** | Uploads the active bank's existing HEX output |
| **Upload Specific Bank...** | Selects and uploads one bank's existing HEX output |
| **Upload all programs** | Uploads the existing HEX output for every populated bank |
| **Compile & Upload** commands | Compile first, then upload and verify the selected bank or banks |

Upload-only commands use the last compiled output. Use a **Compile & Upload** command when the source has changed or when you want to guarantee that the EEPROM receives the current editor version.

Every upload is read back from the EEPROM and compared with the compiled program. A mismatch is reported as an error.

## Programming the EEPROM

There are three ways to load compiled programs onto an FV-1 EEPROM. Choose the one that best matches the hardware you already have and how closely you want programming to integrate with VS Code.

### YGN FV-1 EEPROM Programmer

The custom [YGN FV-1 EEPROM Programmer](https://ygn-effects.com/docs/fv1-programmer-flashing-and-setup/) provides the complete workflow. It connects VS Code to the 24LC32A-compatible EEPROM through the target board's dedicated header, so the chip does not need to be removed from the pedal.

The programmer contains an ATmega328PB running the YGN serial protocol and an FT230X USB-to-serial bridge. Follow the [FV-1 Programmer Flashing and Setup guide](https://ygn-effects.com/docs/fv1-programmer-flashing-and-setup/) to assemble and configure it.

Once the programmer is connected and the target board is powered, run **SpinASM: Auto-Detect Programmer**. You can also choose its serial port manually with **SpinASM: Select Serial Port...**. Use **SpinASM: Check Hardware Connection** to verify the compiler, programmer and target EEPROM before the first upload.

### Arduino Pro Mini 3.3V

An Arduino Pro Mini 3.3V can run the same programming protocol as the custom board. It provides the ATmega programming side of the design without the built-in FT230X, so it needs a separate USB-to-serial adapter to communicate with VS Code.

Follow the [Arduino Pro Mini FV-1 Programmer Setup guide](https://ygn-effects.com/docs/fv1-programmer-arduino-pro-mini-setup/) to flash the programmer firmware and connect it to the target EEPROM. Once configured, the extension can detect and use it through the same upload commands as the YGN programmer.

### External EEPROM Programmer

A standalone EEPROM programmer such as a PICkit, or any programmer compatible with the 24LC32A, can write the complete EEPROM image outside VS Code. This option does not support the extension's upload commands or read-back verification.

Run **SpinASM: Compile all programs to .bin** to create `output/output.bin`. The resulting 4 KB file contains all eight FV-1 banks and is intended to be loaded with the external programmer's own software.

## Configuration

All settings live under `spinasm` in VS Code Settings.

| Setting | Default | Purpose |
|---|---:|---|
| `spinasm.compiler.path` | Empty | Full path to the `asfv1` executable |
| `spinasm.compiler.args` | `["-s"]` | Arguments passed to `asfv1`; `-s` enables SpinASM-compatible literal handling |
| `spinasm.programmer.serialPort` | Empty | Serial port used by the YGN programmer |
| `spinasm.programmer.baudRate` | `57600` | Programmer serial baud rate |
| `spinasm.editor.compileOnSave` | `false` | Compile a bank whenever its `.spn` file is saved |
| `spinasm.statusBar.enabled` | `true` | Show the eight-bank compilation summary |
| `spinasm.logging.verbose` | `false` | Log detailed serial traffic for troubleshooting |

The extension writes compiler, project and programmer activity to the **SpinASM** output channel. Open **View → Output**, then select **SpinASM** from the channel list. When a command fails, the extension automatically reveals this channel. Enable `spinasm.logging.verbose` only when troubleshooting serial communication, as it also logs individual programmer messages.

The selected serial port is stored as a global machine setting so a hardware-specific port name is not committed with a workspace.

## Related FV-1 Platform Source

The extension is one part of the larger [YGN FV-1 platform repository](https://github.com/ygn-effects/project-fv1-platform). The related source lives here:

| Component | Source |
|---|---|
| VS Code extension | [`software/vscode-spinasm`](https://github.com/ygn-effects/project-fv1-platform/tree/main/software/vscode-spinasm) |
| Programmer firmware | [`firmware/vscode-spinasm-firmware`](https://github.com/ygn-effects/project-fv1-platform/tree/main/firmware/vscode-spinasm-firmware) |
| Programmer PCB and fabrication files | [`pcb/vscode-spinasm-programmer`](https://github.com/ygn-effects/project-fv1-platform/tree/main/pcb/vscode-spinasm-programmer) |
| Example guitar and bass programs | [`programs`](https://github.com/ygn-effects/project-fv1-platform/tree/main/programs) |

## Project Status

SpinASM for VS Code is stable and feature-complete for its intended scope. It is used as part of the active YGN FV-1 development workflow for editing, compiling and loading programs onto target hardware.

Bug reports and contributions are welcome through the [project issue tracker](https://github.com/ygn-effects/project-fv1-platform/issues).

## Licence

SpinASM for VS Code is open-source software released under the [Apache License 2.0](LICENSE).

## Acknowledgements

- [Nathan Fraser's `asfv1`](https://github.com/ndf-zz/asfv1) provides the cross-platform FV-1 assembler used by the extension.
- [Spin Semiconductor](https://www.spinsemi.com/) created the FV-1 DSP and SpinASM language.
- The extension uses the [VS Code Extension API](https://code.visualstudio.com/api) and [Node SerialPort](https://serialport.io/) projects.
