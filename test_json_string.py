#!/usr/bin/env python3
"""Test JSON string input functionaliteit"""
import colored_json

print("=== Test JSON String Input ===")

# Test 1: format_from_json
json_str = '{"name": "Alice", "age": 30}'
print("\n1. Test format_from_json:")
result = colored_json.format_from_json(json_str)
print(result)
print("✓ format_from_json werkt!")

# Test 2: Dual mode - format() met JSON string
print("\n2. Test dual mode - format() met JSON string:")
result2 = colored_json.format(json_str)
print(result2)
print("✓ Dual mode werkt!")

# Test 3: to_html_from_json
print("\n3. Test to_html_from_json:")
html = colored_json.to_html_from_json(json_str)
print(f"HTML length: {len(html)} characters")
print("✓ to_html_from_json werkt!")

# Test 4: Dual mode - to_html() met JSON string
print("\n4. Test dual mode - to_html() met JSON string:")
html2 = colored_json.to_html(json_str)
print(f"HTML length: {len(html2)} characters")
print("✓ Dual mode voor HTML werkt!")

print("\n=== Alle tests geslaagd! ===")

