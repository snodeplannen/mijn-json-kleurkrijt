import colored_json
import sys

def main():
    print("=== colored_json: Basic Usage ===")
    
    # Sample data that highlights different types
    data = {
        "project": "mijn-json-kleurkrijt",
        "version": "1.0.0",
        "is_active": True,
        "features": ["fast", "pretty", "customizable"],
        "metadata": {
            "author": "Antigravity",
            "license": "MIT",
            "null_value": None,
            "stars": 42
        }
    }
    
    print("\n1. Direct Printing to Terminal:")
    # print() directly outputs to stdout with ANSI colors
    print(colored_json.format(data))
    
    print("\n2. Formatting as a String:")
    # format() returns the ANSI-colored string without printing
    formatted_str = colored_json.format(data)
    # We can then print it ourselves or process it further
    print(formatted_str)
    
    print("\n3. Compact vs Pretty Printing:")
    # We can control formatting via a Style object
    style = colored_json.Style()
    
    # Compact mode (no newlines/spaces)
    style.compact = True
    print("\nCompact mode:")
    print(colored_json.format(data, style))
    
    # Pretty mode with custom indent
    style.compact = False
    style.indent_size = 4
    print("\nPretty mode (4 spaces indent):")
    print(colored_json.format(data, style))

    print("\n4. Parsing from JSON String:")
    # For maximum performance, you can pass a JSON string directly.
    # This bypasses Python dictionary conversion and uses simdjson parsing.
    jsonStr = '{"direct_parse": true, "speed": "blazing fast"}'
    print(colored_json.format_from_json(jsonStr, style))

if __name__ == "__main__":
    main()
