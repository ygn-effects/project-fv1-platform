# Changelog

All notable changes to SpinASM for VS Code are documented here.

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
