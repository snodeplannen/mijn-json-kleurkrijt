# wjq - Windows JSON Query Tool

A powerful, colored JSON processor for Windows with extensive command-line options, advanced styling capabilities, streaming support, and a flexible callback system.

## Features

- **🎨 15+ Color Themes** - Including advanced themes like `debug`, `depth-aware`, and `data-analysis`
- **🔍 Conditional Styling** - Style based on value content, regex patterns, numeric ranges, and JSON paths
- **📊 Position-based Styling** - Style array elements by index (first, last, even, odd, every N)
- **🌊 Streaming Parser** - Handle extremely large JSON/JSONL files efficiently
- **🔄 Callback System** - Customize behavior with hooks for filtering, transformation, and color overrides
- **🔒 Security Features** - Hide sensitive fields like passwords and API keys
- **📈 Number Formatting** - Automatic thousand separators for better readability

## Installation

### Requirements
- CMake 3.15+
- C++17 compatible compiler (MSVC, GCC, or Clang)
- simdjson (included)

### Windows (Visual Studio)
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### MinGW64 (MSYS2)
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

## Usage

The program is built to `build/Release/wjq.exe` (Visual Studio) or `build/wjq.exe` (MinGW).

### Basic Commands

```bash
# Read file
wjq data.json

# Read from stdin
cat data.json | wjq

# Specific theme
wjq -t monokai data.json

# Compact output
wjq -c data.json

# Custom indentation
wjq -i 4 data.json

# Force color mode
wjq --color-mode 256 data.json
```

### Advanced Options

```bash
# Streaming mode for large files
wjq -s large_file.json

# Line-by-line JSONL parsing
wjq -l records.jsonl

# Hide sensitive fields
wjq --hide-sensitive config.json

# Truncate long strings
wjq --truncate 50 data.json

# Format numbers with thousand separators
wjq --format-numbers financial_data.json
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `-t, --theme THEME` | Choose color scheme (see themes below) |
| `-m, --color-mode MODE` | Color mode: `auto`, `16`, `256`, `truecolor`, `disabled` |
| `-c, --compact` | Compact output (minimal whitespace) |
| `-i, --indent N` | Indentation size (1-8, default: 2) |
| `-s, --streaming` | Use streaming parser for large files |
| `-l, --line-by-line` | Parse JSONL line by line (memory efficient) |
| `--hide-sensitive` | Hide sensitive fields (passwords, tokens) |
| `--truncate N` | Truncate strings longer than N characters |
| `--format-numbers` | Add thousand separators to numbers |
| `--themes` | List all available themes |
| `-h, --help` | Show help |
| `-v, --version` | Show version |

## Color Themes

### Basic Themes

| Theme | Description |
|-------|-------------|
| `default` | Dracula-like dark theme |
| `dracula` | Official Dracula colors |
| `solarized` | Solarized color scheme |
| `monokai` | Monokai theme |
| `github` | GitHub light theme (dark variant) |
| `minimal` | Subtle grayscale |
| `neon` | Neon colors with bold text |

### New Themes

| Theme | Description |
|-------|-------------|
| `ocean` | Deep blue ocean palette |
| `forest` | Natural green/brown tones |
| `cyberpunk` | Matrix green with hot pink |
| `sunset` | Warm orange/pink gradient |
| `high-contrast` | Maximum contrast for accessibility |

### White String Themes (Voor de anti-groenen 😄)

| Theme | Description |
|-------|-------------|
| `white` | Clean white strings with blue keys |
| `nord` | Arctic North-inspired, snow storm whites |
| `gruvbox` | Retro cream/beige strings |
| `one-dark` | VSCode-style white strings |
| `catppuccin` | Soft pastel white strings |
| `ice` | Cold blue whites |
| `coffee` | Warm cream whites |
| `minimal` | Subtle white/light gray strings |

### Advanced Themes

| Theme | Description |
|-------|-------------|
| `debug` | Highlights errors, warnings, and anomalies |
| `depth-aware` | Colors change based on nesting depth |
| `data-analysis` | Highlights array patterns (first/last/even/odd/every N) |

### Examples

```bash
# Debug theme highlights errors and warnings in red/yellow
wjq -t debug application_logs.json

# Depth-aware theme shows nesting visually
wjq -t depth-aware deeply_nested.json

# Data-analysis theme highlights array patterns
wjq -t data-analysis --format-numbers sales_data.json
```

## Conditional Styling

Create custom styles that match:

### Value Matchers

- **Keywords**: Match specific strings (case-sensitive or not)
- **Regex**: Pattern matching with regular expressions
- **Numeric Range**: Match numbers within a range
- **Comparison**: Greater than, less than, equal to

### Position Matchers

- **Depth**: Style based on nesting depth
- **Array Index**: Style first, last, even, odd, or every N items
- **Path**: Match JSON path patterns with wildcards

### Example Custom Rules (C++)

```cpp
Style s;

// Highlight "error" values in red
s.add_rule()
    .for_elements({ElementType::String})
    .when_value(keyword({"error", "failed"}, false))
    .use_color(Color(255, 0, 0).with_bold(true))
    .with_priority(10);

// Highlight large numbers in orange
s.add_rule()
    .for_elements({ElementType::Number})
    .when_value(greater_than(1000))
    .use_color(Color(255, 150, 0));

// Color first array items differently
s.add_rule()
    .for_elements({ElementType::Number})
    .at_array_index("first")
    .use_color(Color(0, 255, 0));
```

## Callback System

Register callbacks to customize behavior:

### Hide Sensitive Data

```cpp
CallbackRegistry callbacks;
callbacks.on_key("password*", callbacks::hide_sensitive("***"));
callbacks.on_key("*token", callbacks::hide_sensitive("***"));
```

### Transform Values

```cpp
// Truncate long strings
callbacks.on_value_transform(callbacks::truncate_strings(100));

// Format numbers with separators
callbacks.on_value_transform(callbacks::format_numbers(','));
```

### Custom Color Overrides

```cpp
callbacks.on_color_override([](const JsonEvent& event, const Color& default) {
    if (event.string_value == "CRITICAL") {
        return Color(255, 0, 0).with_bold(true);
    }
    return std::nullopt;
});
```

## Streaming for Large Files

Process files that don't fit in memory:

```bash
# Streaming mode (memory-mapped when possible)
wjq -s huge_file.json

# Line-by-line JSONL (most memory efficient)
wjq -l large_dataset.jsonl
```

## Examples

See the `examples/` directory for sample JSON files:

| File | Purpose |
|------|---------|
| `simple.json` | Basic JSON structure |
| `debug_example.json` | Demonstrates debug theme |
| `data_analysis.json` | Array data for data-analysis theme |
| `deep_nested.json` | Deep nesting for depth-aware theme |
| `sensitive_data.json` | Shows sensitive field hiding |
| `records.jsonl` | JSON Lines format example |

```bash
# Try different themes
wjq -t debug examples/debug_example.json
wjq -t depth-aware examples/deep_nested.json
wjq -t data-analysis --format-numbers examples/data_analysis.json

# Hide sensitive data
wjq --hide-sensitive examples/sensitive_data.json

# Process JSONL
wjq -l examples/records.jsonl
```

## Building Tests

```bash
mkdir build && cd build
cmake -DBUILD_TESTS=ON ..
cmake --build . --config Release

# Run tests
ctest --output-on-failure
# or
./tests/Release/wjq_tests
```

## Performance

- Uses simdjson for fast parsing (GB/s on modern CPUs)
- Streaming mode handles files larger than available RAM
- Minimal memory allocation during printing
- Efficient path tracking for conditional styling

## Architecture

```
src/
├── color.hpp           # Color struct and ANSI conversion
├── style.hpp           # Style system with rules
├── style_context.hpp   # Context for styling decisions
├── matchers.hpp/cpp    # Pattern matching for values
├── themes.hpp          # Predefined color themes
├── callbacks.hpp       # Callback system for customization
├── printer.hpp         # JSON output formatting
├── json_parser.hpp     # simdjson wrapper
├── streaming_parser.hpp # Streaming JSON processing
└── main.cpp            # CLI entry point
```

## License

MIT License - See LICENSE file for details

## Contributing

Contributions welcome! Areas of interest:
- Additional color themes
- More matcher types
- Additional callback utilities
- Performance improvements
- Documentation improvements
