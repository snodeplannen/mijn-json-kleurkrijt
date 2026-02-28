import colored_json

def main():
    print("=== colored_json: Themes and Custom Colors ===")
    
    data = {
        "server": "localhost",
        "port": 8080,
        "active": True,
        "tags": ["web", "api"]
    }
    
    print("\n1. Using Built-in Themes:")
    # colored_json ships with numerous preset themes.
    # You can list them via colored_json.Style.list_presets()
    presets = colored_json.Style.list_presets()
    print(f"Available presets: {', '.join(presets[:5])}...")
    
    # Let's try some presets
    for theme_name in ["dracula", "monokai", "solarized", "neon"]:
        print(f"\nTheme: {theme_name}")
        style = colored_json.Style.get_preset(theme_name)
        colored_json.print(data, style)
        
    print("\n2. Customizing Base Colors:")
    # You can modify any theme's base colors.
    # Using the built-in colored_json.colors for standard ANSI colors:
    custom_style = colored_json.Style()
    custom_style.key_color = colored_json.colors.bright_cyan
    custom_style.string_color = colored_json.colors.bright_yellow
    custom_style.number_color = colored_json.colors.bright_magenta
    custom_style.bool_color = colored_json.colors.green
    custom_style.null_color = colored_json.colors.bright_black
    
    colored_json.print(data, custom_style)

    print("\n3. Advanced Color Objects:")
    # Colors can be modified using explicit properties:
    style_with_effects = colored_json.Style()
    
    # Make keys bold and underlined
    key_color = colored_json.Color(85, 85, 255) # bright blue
    key_color.bold = True
    key_color.underline = True
    style_with_effects.key_color = key_color
    
    # Make strings italic
    string_color = colored_json.Color(0, 170, 0) # green
    string_color.italic = True
    style_with_effects.string_color = string_color
    
    colored_json.print(data, style_with_effects)

if __name__ == "__main__":
    main()
