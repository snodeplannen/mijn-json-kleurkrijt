# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.8.5] - 2026-02-28

### Fixed
- **Repair**: Restored `color.hpp` and `matchers.hpp` after accidental corruption.
- **Lints**: Cleaned up various unused system includes across the codebase.
- **Lints**: Removed unused `unordered_map`, `algorithm`, `cmath`, etc., as identified by linter.

## [0.8.4] - 2026-02-28

### Fixed
- **Compiler Warnings**: Fixed `tolower` conversion warning in `matchers.cpp`.
- **Compiler Warnings**: Removed unreachable code in `query_engine.cpp`.
- **Third-party Noise**: Suppressed MSVC warnings for `simdjson.h` in headers.

## [0.8.3] - 2026-02-28

### Fixed
- **Multiple Definitions**: Restored `#pragma once` in `color.hpp` to fix build failures
- **Compiler Warnings**: Resolved unreachable code warnings in `query_engine` files

## [0.8.2] - 2026-02-28

### Fixed
- **Linker Errors**: Fixed missing inline definitions by including `themes.hpp` in `style.hpp`
- **wjq Tests**: Made include paths more robust for CI environments
- **Compiler Warnings**: Resolved further warnings (shadowing in `color.hpp`, unreachable code in `query_engine.cpp`) to ensure perfect CI build

## [0.8.1] - 2026-02-28

### Fixed
- **wjq Build Pipeline**: Fixed missing include paths in `wjq_tests` CMake configuration
- **Compiler Warnings**: Resolved multiple warnings (unused parameters, uninitialized variables) to ensure clean CI build

## [0.8.0] - 2026-02-28

### Added
- **Value-Dependent Styling**: Full architecture port of `StyleRule` and `Matcher` engine from `wjq` into the core library
  - `StyleContext` for rule-based color matching on JSON values
  - Boolean coloring: `true` renders green, `false` renders red across all themes
  - `KeywordMatcher`, `RegexMatcher`, `RangeMatcher` for flexible value matching
  - Python bindings for `StyleRule` and `Matcher` API
- **Query Engine Enhancements** (jq parity):
  - Object construction: `{name: .first, age: .years}`
  - Array construction: `[.a, .b, .c]`
  - String interpolation: `"User \(.name) is \(.age) years old"`
  - Mutation operators: `.name = "x"`, `.age |= . + 1`
  - Regex functions: `test("pattern")`, `match("pattern")`, `sub("old", "new")`
  - Try/catch/optional: `try .x`, `.x?`, `try .x catch "fallback"`
  - Variable bindings: `.name as $n | {user: $n}`, with proper scoping/shadowing
- **CLI Features**:
  - `--raw-output` / `-r`: Strip JSON quotes from string output
  - `--slurp` / `-s`: Collect stream of JSON documents into a single array
  - `--arg name value`: Pass string variables into queries as `$name`
  - `--argjson name value`: Pass JSON variables into queries as `$name`
- **Streaming Parser**: Refactored to use `simdjson::iterate_many` for proper multi-line JSON and NDJSON support
- **Examples**: Python examples with README (`examples/01_basic_usage.py` through `examples/04_html_markdown.py`)

### Changed
- **CLI Flag Rename**: `-s` is now `--slurp` (was `--streaming`); streaming moved to `-S`
- **QueryEngine**: Refactored from static methods to instance-based class for variable state management
- **Core Library**: Moved `matchers.hpp`, `matchers.cpp`, `style_context.hpp`, `themes.hpp` from `wjq/src/` to `src/`
- **Positional Argument Detection**: Improved heuristic — single CLI args are treated as filters unless they look like file paths

### Fixed
- **Keyword Literal Parsing**: `true`, `false`, `null` in expression contexts (e.g., `{active: true}`) were incorrectly parsed as property accesses instead of literal values
- **Pipe Operator**: Removed dead `match_str("=")` branch that could consume `=` after `|`

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
