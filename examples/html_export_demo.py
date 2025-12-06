#!/usr/bin/env python3
"""Demonstratie van HTML export functionaliteit"""
import colored_json
import os

# Voorbeeld data
data = {
    "user": {
        "name": "Alice",
        "age": 30,
        "email": "alice@example.com",
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
    ],
    "status": "success",
    "count": 42
}

print("=" * 60)
print("HTML EXPORT DEMONSTRATIE")
print("=" * 60)

# Output directory
output_dir = "html_output"
os.makedirs(output_dir, exist_ok=True)

print("\n=== Standaard preset HTML ===")
style = colored_json.Style.get_preset("default")
html = colored_json.to_html(data, style, title="Default Preset")
output_file = os.path.join(output_dir, "default.html")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(html)
print(f"✓ Opgeslagen in: {output_file}")

print("\n=== Dracula preset HTML ===")
style = colored_json.Style.get_preset("dracula")
html = colored_json.to_html(data, style, title="Dracula Preset", background_color="#282a36")
output_file = os.path.join(output_dir, "dracula.html")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(html)
print(f"✓ Opgeslagen in: {output_file}")

print("\n=== Solarized preset HTML ===")
style = colored_json.Style.get_preset("solarized")
html = colored_json.to_html(data, style, title="Solarized Preset", background_color="#002b36")
output_file = os.path.join(output_dir, "solarized.html")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(html)
print(f"✓ Opgeslagen in: {output_file}")

print("\n=== Monokai preset HTML ===")
style = colored_json.Style.get_preset("monokai")
html = colored_json.to_html(data, style, title="Monokai Preset", background_color="#272822")
output_file = os.path.join(output_dir, "monokai.html")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(html)
print(f"✓ Opgeslagen in: {output_file}")

print("\n=== GitHub preset HTML (lichte achtergrond) ===")
style = colored_json.Style.get_preset("github")
html = colored_json.to_html(data, style, title="GitHub Preset", background_color="#ffffff")
output_file = os.path.join(output_dir, "github.html")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(html)
print(f"✓ Opgeslagen in: {output_file}")

print("\n=== Custom styling met individuele kleuren ===")
style = colored_json.Style.get_preset("default")
style.set_key_color("user", colored_json.Color(255, 0, 0))  # Rode "user" key
style.set_key_color("name", colored_json.Color(0, 255, 0))  # Groene "name" keys
style.set_value_color("user.name", colored_json.Color(255, 20, 147))  # Dieproze
style.set_value_color("user.age", colored_json.Color(0, 191, 255))  # Diepblauw

html = colored_json.to_html(data, style, title="Custom Styling", background_color="#1e1e1e")
output_file = os.path.join(output_dir, "custom.html")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(html)
print(f"✓ Opgeslagen in: {output_file}")

print("\n=== Compact mode HTML ===")
style = colored_json.Style.get_preset("default")
style.compact = True
html = colored_json.to_html(data, style, title="Compact Mode", background_color="#1e1e1e")
output_file = os.path.join(output_dir, "compact.html")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(html)
print(f"✓ Opgeslagen in: {output_file}")

print("\n=== Met custom font ===")
style = colored_json.Style.get_preset("monokai")
html = colored_json.to_html(
    data, style, 
    title="Custom Font",
    background_color="#272822",
    font_family="'Fira Code', 'JetBrains Mono', Consolas, monospace"
)
output_file = os.path.join(output_dir, "custom_font.html")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(html)
print(f"✓ Opgeslagen in: {output_file}")

print("\n=== Index pagina maken ===")
index_html = """<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Colored JSON HTML Export Demo</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 50px auto;
            padding: 20px;
            background-color: #f5f5f5;
        }
        h1 {
            color: #333;
        }
        ul {
            list-style-type: none;
            padding: 0;
        }
        li {
            margin: 10px 0;
            padding: 10px;
            background-color: white;
            border-radius: 5px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        a {
            color: #0066cc;
            text-decoration: none;
            font-weight: bold;
        }
        a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <h1>Colored JSON HTML Export Demo</h1>
    <p>Klik op een link om de verschillende presets en stijlen te bekijken:</p>
    <ul>
        <li><a href="default.html">Default Preset</a></li>
        <li><a href="dracula.html">Dracula Preset</a></li>
        <li><a href="solarized.html">Solarized Preset</a></li>
        <li><a href="monokai.html">Monokai Preset</a></li>
        <li><a href="github.html">GitHub Preset</a></li>
        <li><a href="custom.html">Custom Styling</a></li>
        <li><a href="compact.html">Compact Mode</a></li>
        <li><a href="custom_font.html">Custom Font</a></li>
    </ul>
</body>
</html>"""

index_file = os.path.join(output_dir, "index.html")
with open(index_file, "w", encoding="utf-8") as f:
    f.write(index_html)
print(f"✓ Index pagina opgeslagen in: {index_file}")

print("\n" + "=" * 60)
print(f"Alle HTML bestanden zijn opgeslagen in: {output_dir}/")
print(f"Open {index_file} in je browser om alle voorbeelden te bekijken!")
print("=" * 60)

