import colored_json
import os
import json
import re

sample = {
    "project": "mijn-json-kleurkrijt",
    "version": "0.9.0",
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

sample_json = json.dumps(sample)

presets = colored_json.Style.list_presets()
presets.sort()

html_content = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <style>
        body { margin: 0; padding: 0; background: #0d1117; color: white; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif; }
        .theme-wrapper {
            padding: 20px;
            border-bottom: 1px solid #30363d;
        }
        .theme-header {
            margin-bottom: 10px;
            font-size: 18px;
            font-weight: bold;
        }
        .theme-container { 
            display: inline-block;
            padding: 0;
            margin: 0;
            border-radius: 6px;
            overflow: hidden;
        }
        pre { margin: 0; padding: 20px; }
    </style>
</head>
<body>
"""

for name in presets:
    style = colored_json.Style.get_preset(name)
    bg = "#ffffff" if name in ["white", "github"] else "#1e1e1e"
    
    # Use to_html_from_json to get THEME-ONLY HTML block
    complete_html = colored_json.to_html_from_json(sample_json, style, title="", background_color=bg)
    
    # Extract only the <pre> block
    match = re.search(r'<pre.*</pre>', complete_html, re.DOTALL)
    if match:
        pre_content = match.group(0)
        html_content += f"""
        <div class="theme-wrapper">
            <div class="theme-header">Thèma: {name}</div>
            <div id="theme-{name}" class="theme-container">
                {pre_content}
            </div>
        </div>
        """

html_content += "</body></html>"

with open("theme_preview.html", "w", encoding="utf-8") as f:
    f.write(html_content)

print("Created fixed theme_preview.html")
