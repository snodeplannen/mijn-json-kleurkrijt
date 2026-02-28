# colored_json Examples

This directory contains executable examples demonstrating the full suite of features in the `colored_json` Python module. You can run any of these files directly using Python.

## 01. Basic Usage (`01_basic_usage.py`)
Demonstrates how to import the module and start printing JSON dictionaries or strings with instant ANSI styling.
- `colored_json.print(data)`: Immediately print to terminal.
- `colored_json.format(data)`: Retrieve a styled string for custom logging.
- `style.compact = True`: Toggling pretty printing and compact modes.
- `colored_json.format_from_json(json_str)`: Using SIMDJSON parsing for maximal performance.

## 02. Themes & Colors (`02_themes.py`)
Shows how to change the overall look of the JSON output.
- Loading built-in presets: `dracula`, `monokai`, `solarized`, `neon`, etc.
- Manually overriding key colors with exact RGB mappings: `colored_json.Color(255, 0, 0)`.
- Adding formatting attributes: `.bold`, `.italic`, `.underline`.
- Utilizing the standard color constants: `colored_json.colors.bright_cyan`.

## 03. Advanced Styling (`03_advanced_styling.py`)
Showcases conditional styling features to color specific parts of the JSON uniquely based on their key or path.
- `style.set_key_color(...)`: Make a key (e.g., `status`) consistently red anywhere in the JSON.
- `style.set_value_color(...)`: Highlight a value based on its precise path.
- *Note:* Themes natively handle boolean highlighting (`true` = Green, `false` = Red) out of the box dynamically.

## 04. HTML & Markdown Exports (`04_html_markdown.py`)
A powerful feature of the library is translating the ANSI coloring logic into CSS and Markdown.
- `colored_json.to_markdown()`: Generates standard GitHub-flavored ` ```json ` blocks.
- `colored_json.to_markdown_html()`: Translates the color mapping to inline `<span>` elements so JSON is fully colored on platforms that support raw HTML in Markdown (e.g., GitHub, GitLab).
- `colored_json.to_html()`: Generates a complete standalone web page with beautiful dark-mode defaults.

By default, the `04_html_markdown.py` script automatically creates `output.md`, `output_colored.md`, and `output.html` in this directory so you can visually verify the outputs!
