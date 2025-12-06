#!/usr/bin/env python3
"""Demonstratie van alle features van colored_json module"""
import colored_json
import time

# Voorbeeld data
data = {
    "user": {
        "name": "Alice",
        "age": 30,
        "active": True,
        "scores": [95.5, 87.0, 92.0],
        "metadata": {
            "last_login": "2024-01-15",
            "preferences": {
                "theme": "dark",
                "notifications": False,
                "language": "nl"
            }
        }
    },
    "products": [
        {"id": 1, "name": "Laptop", "price": 999.99, "in_stock": True},
        {"id": 2, "name": "Muis", "price": 29.99, "in_stock": False},
        None
    ]
}

print("=" * 60)
print("GEKLEURDE JSON MODULE DEMONSTRATIE")
print("=" * 60)

print("\n=== Standaard preset (compact) ===")
style = colored_json.Style.get_preset("default")
style.compact = True
colored_json.print(data, style)

print("\n=== Solarized preset (geïndenteerd) ===")
style = colored_json.Style.get_preset("solarized")
style.compact = False
style.indent_size = 2
colored_json.print(data, style)

print("\n=== Monokai preset met custom kleur ===")
style = colored_json.Style.get_preset("monokai")
style.key_color = colored_json.Color.from_rgb(255, 100, 0)  # Oranje keys
style.key_color.bold = True
colored_json.print(data, style)

print("\n=== Minimal preset (subtiel) ===")
style = colored_json.Style.get_preset("minimal")
colored_json.print(data, style)

print("\n=== Neon preset (compact) ===")
style = colored_json.Style.get_preset("neon")
style.compact = True
colored_json.print(data, style)

print("\n=== Dracula preset ===")
style = colored_json.Style.get_preset("dracula")
colored_json.print(data, style)

print("\n=== GitHub preset ===")
style = colored_json.Style.get_preset("github")
colored_json.print(data, style)

print("\n=== Custom styling ===")
style = colored_json.Style()
style.key_color = colored_json.colors.bright_cyan
style.string_color = colored_json.colors.bright_green
style.number_color = colored_json.Color.from_rgb(255, 165, 0)
style.number_color.bold = True
style.bool_color = colored_json.colors.bright_magenta
style.null_color = colored_json.colors.bright_black
style.brace_color = colored_json.colors.bright_white
style.color_mode = colored_json.ColorMode.TRUECOLOR
colored_json.print(data, style)

print("\n=== Alle beschikbare presets ===")
presets = colored_json.Style.list_presets()
for preset_name in presets:
    print(f"  - {preset_name}")

print("\n=== Performance benchmark ===")
# Grote dataset genereren
large_data = {"items": [{"id": i, "data": f"item_{i}", "value": i * 1.5} for i in range(10000)]}

style = colored_json.Style()
style.compact = True

start = time.time()
result = colored_json.format(large_data, style)
duration = time.time() - start

print(f"Geformatteerd in {duration*1000:.2f} ms")
print(f"Output grootte: {len(result)//1024} KB")
print(f"Items per seconde: {10000/duration:.0f}")

print("\n=== Verschillende data types ===")
test_data = {
    "string": "Hello World",
    "integer": 42,
    "float": 3.14159,
    "boolean_true": True,
    "boolean_false": False,
    "null_value": None,
    "empty_dict": {},
    "empty_list": [],
    "nested": {
        "level1": {
            "level2": "deep"
        }
    },
    "array": [1, 2, 3, "mixed", True, None]
}

style = colored_json.Style.get_preset("default")
colored_json.print(test_data, style)

print("\n=== String escaping test ===")
escape_data = {
    "quotes": 'Text with "quotes"',
    "backslash": "Path\\to\\file",
    "newline": "Line1\nLine2",
    "tab": "Column1\tColumn2",
    "unicode": "Café 🎉"
}

colored_json.print(escape_data, style)

print("\n" + "=" * 60)
print("DEMONSTRATIE VOLTOOID")
print("=" * 60)

