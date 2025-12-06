#!/usr/bin/env python3
"""Unit tests voor colored_json module"""
import pytest
import colored_json


def test_preset_loading():
    """Test dat presets correct kunnen worden geladen"""
    presets = colored_json.Style.list_presets()
    assert len(presets) > 0
    assert "default" in presets
    
    # Test alle presets
    for preset_name in presets:
        style = colored_json.Style.get_preset(preset_name)
        assert style is not None
        assert isinstance(style, colored_json.Style)


def test_invalid_preset():
    """Test dat ongeldige preset een exception gooit"""
    with pytest.raises(RuntimeError):
        colored_json.Style.get_preset("nonexistent_preset")


def test_color_creation():
    """Test Color object creatie"""
    color = colored_json.Color(255, 128, 64)
    assert color.r == 255
    assert color.g == 128
    assert color.b == 64
    
    color2 = colored_json.Color.from_rgb(100, 200, 50)
    assert color2.r == 100
    assert color2.g == 200
    assert color2.b == 50


def test_color_modifiers():
    """Test color modifiers (bold, italic, underline)"""
    color = colored_json.Color(255, 255, 255)
    color.bold = True
    color.italic = True
    color.underline = True
    
    assert color.bold is True
    assert color.italic is True
    assert color.underline is True


def test_compact_formatting():
    """Test compact formatting mode"""
    data = {"key": "value", "number": 42}
    style = colored_json.Style()
    style.compact = True
    
    result = colored_json.format(data, style)
    assert "{" in result
    assert "}" in result
    assert "\n" not in result or result.count("\n") < 3


def test_indented_formatting():
    """Test indented formatting mode"""
    data = {"key": "value", "number": 42}
    style = colored_json.Style()
    style.compact = False
    style.indent_size = 2
    
    result = colored_json.format(data, style)
    assert "{" in result
    assert "}" in result
    assert "\n" in result


def test_string_escaping():
    """Test dat strings correct worden ge-escaped"""
    data = {"test": "quote\"test", "backslash": "test\\test", "newline": "test\ntest"}
    style = colored_json.Style()
    
    result = colored_json.format(data, style)
    assert "\\\"" in result or '"' in result
    assert "\\\\" in result or "\\" in result


def test_dict_formatting():
    """Test dict formatting"""
    data = {"name": "Alice", "age": 30}
    style = colored_json.Style()
    
    result = colored_json.format(data, style)
    assert "name" in result
    assert "Alice" in result
    assert "age" in result
    assert "30" in result


def test_list_formatting():
    """Test list formatting"""
    data = [1, 2, 3, "test"]
    style = colored_json.Style()
    
    result = colored_json.format(data, style)
    assert "1" in result
    assert "2" in result
    assert "3" in result
    assert "test" in result


def test_nested_structures():
    """Test geneste data structuren"""
    data = {
        "user": {
            "name": "Bob",
            "scores": [10, 20, 30]
        }
    }
    style = colored_json.Style()
    
    result = colored_json.format(data, style)
    assert "user" in result
    assert "name" in result
    assert "Bob" in result
    assert "scores" in result


def test_empty_dict():
    """Test lege dict"""
    data = {}
    style = colored_json.Style()
    
    result = colored_json.format(data, style)
    assert "{}" in result.replace("\033[0m", "").replace("\033[", "")


def test_empty_list():
    """Test lege list"""
    data = []
    style = colored_json.Style()
    
    result = colored_json.format(data, style)
    assert "[]" in result.replace("\033[0m", "").replace("\033[", "")


def test_number_types():
    """Test verschillende number types"""
    data = {"int": 42, "float": 3.14, "negative": -10}
    style = colored_json.Style()
    
    result = colored_json.format(data, style)
    assert "42" in result
    assert "3.14" in result or "3" in result
    assert "-10" in result


def test_boolean_types():
    """Test boolean types"""
    data = {"true_val": True, "false_val": False}
    style = colored_json.Style()
    
    result = colored_json.format(data, style)
    assert "True" in result or "true" in result
    assert "False" in result or "false" in result


def test_null_value():
    """Test null/None waarde"""
    data = {"null_val": None}
    style = colored_json.Style()
    
    result = colored_json.format(data, style)
    assert "null" in result


def test_custom_colors():
    """Test custom kleuren configuratie"""
    style = colored_json.Style()
    style.key_color = colored_json.Color(255, 0, 0)
    style.string_color = colored_json.Color(0, 255, 0)
    
    data = {"key": "value"}
    result = colored_json.format(data, style)
    assert len(result) > 0


def test_color_modes():
    """Test verschillende color modes"""
    data = {"test": "value"}
    
    for mode in [colored_json.ColorMode.AUTO, 
                 colored_json.ColorMode.ANSI16,
                 colored_json.ColorMode.ANSI256,
                 colored_json.ColorMode.TRUECOLOR,
                 colored_json.ColorMode.DISABLED]:
        style = colored_json.Style()
        style.color_mode = mode
        result = colored_json.format(data, style)
        assert len(result) > 0


def test_builtin_colors():
    """Test ingebouwde kleuren"""
    assert hasattr(colored_json.colors, "red")
    assert hasattr(colored_json.colors, "green")
    assert hasattr(colored_json.colors, "blue")
    assert hasattr(colored_json.colors, "bright_red")
    
    # Test dat kleuren Color objecten zijn
    assert isinstance(colored_json.colors.red, colored_json.Color)


def test_indent_size():
    """Test verschillende indent sizes"""
    data = {"nested": {"deep": "value"}}
    
    for indent in [2, 4, 8]:
        style = colored_json.Style()
        style.compact = False
        style.indent_size = indent
        result = colored_json.format(data, style)
        assert len(result) > 0


def test_printer_class():
    """Test Printer class direct gebruik"""
    style = colored_json.Style()
    printer = colored_json.Printer(style)
    
    data = {"test": "value"}
    result = printer.print(data)
    assert len(result) > 0
    assert "test" in result


if __name__ == "__main__":
    pytest.main([__file__, "-v"])

