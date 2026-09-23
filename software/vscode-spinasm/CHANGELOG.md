# Changelog

All notable changes to SpinASM for VS Code are documented here.

## 0.3.0

- Upload commands now check each bank's compiled output first: when it is missing or older than its source, they offer to compile it, or to upload the previous program as is
- Compile on save now runs only when a file is saved in the editor, no longer when files change outside VS Code (git pull, branch switch, another editor)
- Compile, upload and hardware operations now run one at a time; a command started during another waits for it to finish
- `spinasm.compiler.path` now accepts a command on PATH, such as `asfv1`, as well as a full path, with a clear error when it cannot be found
- The bank status bar now works before a compiler is configured, and upload-only commands no longer require a compiler path
- "Current program" commands in multi-root workspaces now use the folder of the file they act on instead of asking for one
- Added a live diagnostic for unknown instruction mnemonics
- Fixed MEM validation to match asfv1: sizes are limited to 0–32767 and each block uses one extra sample of delay memory
- Fixed false "Undefined symbol" errors on `$` hex literals starting with a letter and on `INT()` expressions
- Fixed Go to Definition and diagnostic positions for short symbol names, operands and coefficients
- Fixed bank detection for projects stored below a folder named `bank_N`
- Fixed commands briefly seeing missing banks while a project was being rescanned
- A missing or unconfigured compiler no longer deletes the last compiled output
- Compile all and upload all now report when a project has no programs, and compile all shows per-bank progress
- Added project, compiler and bank-status tests using a portable stand-in for asfv1, and consolidated the compile and upload commands

## 0.2.0

- Made programmer response framing safe for binary EEPROM data containing protocol marker bytes
- Prevented compile commands from using stale source when a relevant editor has unsaved changes
- Added strict Intel HEX validation before opening the programmer, including checksums, record types, EOF structure, selected-bank address and image size
- Added simulated programmer and upload-safety regression tests, including checks that invalid images issue no EEPROM operations
- Updated the extension icon, package contents and documentation links

## 0.1.0

- Initial public release
- SpinASM syntax and semantic highlighting, completion, hover information and Go to Definition
- Live diagnostics and FV-1 resource usage reporting
- Eight-bank project creation, compilation and combined EEPROM image generation through `asfv1`
- Optional YGN programmer detection, EEPROM upload and read-back verification
