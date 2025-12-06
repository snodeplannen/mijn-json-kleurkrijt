#!/usr/bin/env python3
"""Demonstratie van Markdown export functionaliteit"""
import colored_json
import os

# Voorbeeld data
data = {
    "user": {
        "name": "Alice",
        "age": 30,
        "email": "alice@example.com",
        "active": True,
        "scores": [95.5, 87.0, 92.0]
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
print("MARKDOWN EXPORT DEMONSTRATIE")
print("=" * 60)

# Output directory
output_dir = "markdown_output"
os.makedirs(output_dir, exist_ok=True)

print("\n=== Standaard Markdown (code block) ===")
style = colored_json.Style.get_preset("default")
markdown = colored_json.to_markdown(data, style, title="Example JSON Data")
output_file = os.path.join(output_dir, "example.md")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(markdown)
print(f"✓ Opgeslagen in: {output_file}")

print("\n=== Markdown met HTML (gekleurd) ===")
style = colored_json.Style.get_preset("dracula")
markdown = colored_json.to_markdown_html(data, style, title="Colored JSON Example", background_color="#282a36")
output_file = os.path.join(output_dir, "colored_example.md")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(markdown)
print(f"✓ Opgeslagen in: {output_file}")

print("\n=== Verschillende presets in Markdown ===")
presets = ["default", "solarized", "monokai", "github", "neon"]

for preset_name in presets:
    style = colored_json.Style.get_preset(preset_name)
    bg_color = "#1e1e1e"
    if preset_name == "github":
        bg_color = "#ffffff"
    elif preset_name == "solarized":
        bg_color = "#002b36"
    elif preset_name == "monokai":
        bg_color = "#272822"
    elif preset_name == "neon":
        bg_color = "#000000"
    
    markdown = colored_json.to_markdown_html(
        data, style, 
        title=f"{preset_name.capitalize()} Preset",
        background_color=bg_color
    )
    output_file = os.path.join(output_dir, f"{preset_name}.md")
    with open(output_file, "w", encoding="utf-8") as f:
        f.write(markdown)
    print(f"✓ {preset_name.capitalize()} preset opgeslagen in: {output_file}")

print("\n=== Custom styling in Markdown ===")
style = colored_json.Style.get_preset("default")
style.set_key_color("user", colored_json.Color(255, 0, 0))  # Rode "user" key
style.set_key_color("name", colored_json.Color(0, 255, 0))  # Groene "name" keys
style.set_value_color("user.name", colored_json.Color(255, 20, 147))  # Dieproze

markdown = colored_json.to_markdown_html(
    data, style,
    title="Custom Styled JSON",
    background_color="#1e1e1e"
)
output_file = os.path.join(output_dir, "custom_styled.md")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(markdown)
print(f"✓ Opgeslagen in: {output_file}")

print("\n=== Compact mode in Markdown ===")
style = colored_json.Style.get_preset("default")
style.compact = True
markdown = colored_json.to_markdown_html(data, style, title="Compact JSON")
output_file = os.path.join(output_dir, "compact.md")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(markdown)
print(f"✓ Opgeslagen in: {output_file}")

print("\n=== Complete document met meerdere voorbeelden ===")
complete_doc = """# Colored JSON Markdown Examples

Dit document toont verschillende voorbeelden van gekleurde JSON in Markdown.

## Standaard Code Block

"""
complete_doc += colored_json.to_markdown(data, colored_json.Style.get_preset("default"), title="")

complete_doc += "\n\n## Gekleurde JSON (HTML in Markdown)\n\n"
complete_doc += colored_json.to_markdown_html(
    data, 
    colored_json.Style.get_preset("dracula"),
    title="",
    background_color="#282a36"
)

complete_doc += "\n\n## Custom Styling\n\n"
style = colored_json.Style.get_preset("default")
style.set_key_color("user", colored_json.Color(255, 0, 0))
style.set_key_color("name", colored_json.Color(0, 255, 0))
complete_doc += colored_json.to_markdown_html(
    data, style, title="", background_color="#1e1e1e"
)

complete_doc += "\n\n## Compact Mode\n\n"
style.compact = True
complete_doc += colored_json.to_markdown_html(
    data, style, title="", background_color="#1e1e1e"
)

output_file = os.path.join(output_dir, "complete_examples.md")
with open(output_file, "w", encoding="utf-8") as f:
    f.write(complete_doc)
print(f"✓ Complete document opgeslagen in: {output_file}")

print("\n" + "=" * 60)
print(f"Alle Markdown bestanden zijn opgeslagen in: {output_dir}/")
print("=" * 60)

