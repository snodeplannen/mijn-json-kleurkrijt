import colored_json
import os

sample = {
    "project": "mijn-json-kleurkrijt",
    "version": "0.8.7",
    "status": {
        "active": True,
        "debug": False,
        "errors": None,
        "details": {
            "uptime": "99.99%",
            "region": "eu-central-1"
        }
    },
    "metrics": [42, 123.45, -7],
    "tags": ["simdjson", "cpp17", "theme"]
}

presets = colored_json.Style.list_presets()
# Sort for consistent order
presets.sort()

gallery_md = "## Kleurthema Gallery\n\nHieronder vind je voorbeelden van alle beschikbare thema's. Klik op een naam om de weergave te openen:\n\n"

# Create a grid or just a list of details
for name in presets:
    try:
        style = colored_json.Style.get_preset(name)
        # We use a custom background for certain themes if needed, 
        # but the default #1e1e1e is good for dark themes.
        # For 'white' or light themes, we might want a different background.
        bg = "#ffffff" if name in ["white", "github"] else "#1e1e1e"
        
        # MarkdownPrinter::print_html usually adds a header. Let's see.
        md_content = colored_json.to_markdown_html(sample, style, title="", background_color=bg)
        
        # Remove empty title header if generated (it usually adds ### \n)
        md_content = md_content.strip()
        if md_content.startswith("###"):
            # Skip the first line
            lines = md_content.split("\n")
            md_content = "\n".join(lines[1:]).strip()

        gallery_md += f"<details>\n<summary>Thèma: <b>{name}</b></summary>\n\n{md_content}\n\n</details>\n"
    except Exception as e:
        print(f"Error generating {name}: {e}")

with open("theme_gallery.md", "w", encoding="utf-8") as f:
    f.write(gallery_md)

print("Gallery generated to theme_gallery.md")
