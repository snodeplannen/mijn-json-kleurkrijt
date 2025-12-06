#!/usr/bin/env python3
"""Demonstratie van quote kleuren styling"""
import colored_json

# Voorbeeld data
data = {
    "user": {
        "name": "Alice",
        "email": "alice@example.com"
    },
    "status": "active"
}

print("=" * 60)
print("QUOTE KLEUREN DEMONSTRATIE")
print("=" * 60)

print("\n=== Standaard (quotes zelfde kleur als content) ===")
style = colored_json.Style.get_preset("default")
colored_json.print(data, style)

print("\n=== Solarized preset (subtiele quote kleuren) ===")
style = colored_json.Style.get_preset("solarized")
colored_json.print(data, style)

print("\n=== Custom quote kleuren ===")
style = colored_json.Style.get_preset("default")
# Verschillende kleuren voor quotes
style.key_quote_color = colored_json.Color(255, 255, 0)  # Geel voor key quotes
style.string_quote_color = colored_json.Color(0, 255, 255)  # Cyaan voor string quotes
colored_json.print(data, style)

print("\n=== Subtiele quote kleuren (grijs) ===")
style = colored_json.Style.get_preset("default")
style.key_quote_color = colored_json.Color(128, 128, 128)  # Grijs
style.string_quote_color = colored_json.Color(128, 128, 128)  # Grijs
colored_json.print(data, style)

print("\n=== Bold quotes voor extra nadruk ===")
style = colored_json.Style.get_preset("default")
style.key_quote_color = colored_json.Color(255, 255, 255)
style.key_quote_color.bold = True
style.string_quote_color = colored_json.Color(255, 255, 255)
style.string_quote_color.bold = True
colored_json.print(data, style)

print("\n=== Monokai preset (witte quotes) ===")
style = colored_json.Style.get_preset("monokai")
colored_json.print(data, style)

print("\n=== GitHub preset (donkere quotes) ===")
style = colored_json.Style.get_preset("github")
colored_json.print(data, style)

print("\n=== Minimal preset (subtiele quotes) ===")
style = colored_json.Style.get_preset("minimal")
colored_json.print(data, style)

print("\n" + "=" * 60)
print("DEMONSTRATIE VOLTOOID")
print("=" * 60)

