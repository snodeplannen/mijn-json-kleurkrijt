import colored_json

def main():
    print("=== colored_json: Advanced Value-Dependent Styling ===")
    
    # Advanced styling is purely value/path dependent and done via StyleRules
    data = {
        "status": "success",
        "nested": {
            "status": "error",
            "message": "Resource not found"
        },
        "flags": [True, False, True],
        "system": "online",
        "metrics": [10, 20, 30, 40, 50]
    }
    
    # Let's create a custom style
    style = colored_json.Style()
    
    # 1. Boilerplate rules: boolean coloring is already enabled by default in presets.
    # You can also manually add rules!
    print("\n1. Key-based styling:")
    # Provide special colors to specific keys regardless of location
    error_key_color = colored_json.Color(255, 85, 85) # bright red
    error_key_color.bold = True
    
    style.set_key_color("status", error_key_color)
    
    # Make nested message yellow
    msg_key_color = colored_json.Color(255, 255, 85)
    msg_key_color.italic = True
    style.set_key_color("message", msg_key_color)

    colored_json.print(data, style)
    
    print("\n2. Path-based Custom Styling:")
    # Style values specifically by their path
    path_color = colored_json.colors.bright_blue
    path_color.underline = True
    
    style.set_value_color("system", path_color)
        
    colored_json.print(data, style)
    
    print("\n3. Booleans natively handled by themes:")
    # All preset themes natively style boolean strings correctly via core C++ rules!
    colored_json.print(data, colored_json.Style.get_preset("default"))

if __name__ == "__main__":
    main()
