import colored_json
import os

def main():
    print("=== colored_json: HTML and Markdown Exports ===")
    
    data = {"name": "Test", "status": "success", "value": 42}
    
    style = colored_json.Style.get_preset("default")
    
    output_dir = os.path.dirname(__file__)
    
    print("\n1. Exporting to standard Markdown")
    # Pure Markdown block, usually drops ANSI colors but keeps JSON formatting
    md_output = colored_json.to_markdown(data, style, title="Standard JSON")
    
    md_file = os.path.join(output_dir, "output.md")
    with open(md_file, "w", encoding="utf-8") as f:
        f.write(md_output)
    print(f"-> Saved {md_file}")
    
    print("\n2. Exporting to Markdown with HTML (Colored)")
    # This uses a <pre> block with inline <span> colors.
    # GitHub, GitLab and many others render this perfectly with full colors!
    md_html_output = colored_json.to_markdown_html(data, style, title="Colored JSON HTML")
    
    md_html_file = os.path.join(output_dir, "output_colored.md")
    with open(md_html_file, "w", encoding="utf-8") as f:
        f.write(md_html_output)
    print(f"-> Saved {md_html_file}")
    
    print("\n3. Exporting to Full HTML Page")
    # Creates a full standalone HTML document with embedded styles
    html_output = colored_json.to_html(data, style, title="My JSON Dashboard", background_color="#101015")
    
    html_file = os.path.join(output_dir, "output.html")
    with open(html_file, "w", encoding="utf-8") as f:
        f.write(html_output)
    print(f"-> Saved {html_file}")
    
    print("\nOpen the generated files in your browser or Markdown viewer to see the results!")

if __name__ == "__main__":
    main()
