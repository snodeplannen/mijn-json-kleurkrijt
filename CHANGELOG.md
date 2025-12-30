# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **wjq Tool**: New command-line JSON query tool (`wjq`) for Windows
  - Standalone C++ executable for fast JSON formatting
  - Supports JSONL (JSON Lines) format for multiple documents
  - Multiple color themes (default, dracula, solarized, monokai, github, minimal, neon)
  - Compact and indented output modes
  - Configurable color modes (auto, 16, 256, truecolor, disabled)
  - Built with simdjson for high-performance JSON parsing
  - Located in `wjq/` directory with independent CMake build system

### Changed
- **CMake Modernization**: Updated to use modern `FindPython` module instead of deprecated `FindPythonLibs`
  - Set `PYBIND11_FINDPYTHON ON` to suppress CMake policy CMP0148 warnings
  - Improved compatibility with newer CMake versions
- **Build System**: Improved CMake configuration for better IDE integration
  - Added `wjq` subdirectory to main CMakeLists.txt
  - Better handling of simdjson dependency (local vs global)

### Fixed
- **IntelliSense Configuration**: Resolved all C++ IntelliSense errors in VS Code
  - Fixed include order in `printer.hpp` (simdjson.h before json_parser.hpp)
  - Created `.clangd` configuration files for proper include path resolution
  - Added include paths for simdjson, pybind11, and Python headers
  - Removed duplicate simdjson paths from `c_cpp_properties.json`
- **Compiler Warnings**: Added default case to switch statement in `printer.hpp` to handle unknown JSON types
- **Git Repository**: Improved `.gitignore` patterns
  - Added `build*/` pattern to catch all build directory variations
  - Added Visual Studio specific patterns
  - Added clangd cache directories
  - Better organization with clear sections
  - Whitelisted essential VS Code configuration files

### Development
- **IDE Support**: Enhanced development experience
  - Configured clangd for accurate IntelliSense
  - Added browse paths for better symbol resolution
  - Cleaned up build artifacts from repository

## [Previous Versions]

### Initial Release
- Python module for colored JSON output
- Multiple export formats (console, HTML, Markdown)
- Preset color themes
- Custom color configuration
- simdjson integration for fast JSON parsing
- Type hints (.pyi stub files)
