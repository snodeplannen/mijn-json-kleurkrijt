#pragma once
#include "color.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

namespace colored_json {

// Type aliassen voor color maps
using KeyColorsMap = std::unordered_map<std::string, Color>;
using ValueColorsMap = std::unordered_map<std::string, Color>;

enum class ElementType {
    Key,
    String,
    Number,
    Boolean,
    Null,
    Brace,      // { }
    Bracket,    // [ ]
    Colon,
    Comma,
    Whitespace // Alleen voor indented mode
};

struct Style {
    Color key_color{93, 173, 226};      // Lichtblauw
    Color string_color{152, 224, 36};   // Limegroen
    Color number_color{255, 203, 107};  // Oranje
    Color bool_color{255, 121, 198};    // Roze
    Color null_color{137, 137, 137};    // Grijs
    Color brace_color{255, 255, 255};   // Wit
    Color bracket_color{255, 255, 255}; // Wit
    Color colon_color{255, 255, 255};   // Wit
    Color comma_color{255, 255, 255};   // Wit
    Color key_quote_color{255, 255, 255};   // Kleur voor aanhalingstekens rond keys (standaard: zelfde als key)
    Color string_quote_color{255, 255, 255}; // Kleur voor aanhalingstekens rond strings (standaard: zelfde als string)
    
    // Individuele kleuren per key
    KeyColorsMap key_colors;
    // Individuele kleuren per key voor values
    ValueColorsMap value_colors;
    
    ColorMode color_mode = ColorMode::Auto;
    bool compact = false;
    int indent_size = 2;
    
    // Helper functies voor individuele kleuren
    void setKeyColor(const std::string& key, const Color& color) {
        key_colors[key] = color;
    }
    
    void setValueColor(const std::string& path, const Color& color) {
        value_colors[path] = color;
    }
    
    static Style getPreset(const std::string& name);
    static std::vector<std::string> listPresets();
};

inline Style Style::getPreset(const std::string& name) {
    Style s;
    
    if (name == "default" || name == "dracula") {
        s.key_color = Color{93, 173, 226};
        s.string_color = Color{152, 224, 36};
        s.number_color = Color{255, 203, 107};
        s.bool_color = Color{255, 121, 198};
        s.null_color = Color{137, 137, 137};
        s.key_quote_color = Color{93, 173, 226};  // Zelfde als key
        s.string_quote_color = Color{152, 224, 36}; // Zelfde als string
        
    } else if (name == "solarized") {
        s.key_color = Color{38, 139, 210};
        s.string_color = Color{42, 161, 152};
        s.number_color = Color{181, 137, 0};
        s.bool_color = Color{211, 54, 130};
        s.null_color = Color{131, 148, 150};
        s.brace_color = Color{88, 110, 117};
        s.key_quote_color = Color{88, 110, 117};  // Subtieler grijs
        s.string_quote_color = Color{88, 110, 117};
        
    } else if (name == "monokai") {
        s.key_color = Color{156, 220, 254};
        s.string_color = Color{206, 145, 120};
        s.number_color = Color{181, 206, 168};
        s.bool_color = Color{190, 132, 255};
        s.null_color = Color{156, 156, 156};
        s.brace_color = Color{248, 248, 242};
        s.key_quote_color = Color{248, 248, 242};  // Wit/grijs
        s.string_quote_color = Color{248, 248, 242};
        
    } else if (name == "github") {
        s.key_color = Color{3, 47, 98};
        s.string_color = Color{3, 106, 7};
        s.number_color = Color{28, 0, 207};
        s.bool_color = Color{0, 16, 128};
        s.null_color = Color{128, 128, 128};
        s.brace_color = Color{36, 41, 46};
        s.key_quote_color = Color{36, 41, 46};  // Donkergrijs
        s.string_quote_color = Color{36, 41, 46};
        
    } else if (name == "minimal") {
        s.key_color = Color::fromRgb(200, 200, 200);
        s.string_color = Color::fromRgb(200, 200, 200);
        s.number_color = Color::fromRgb(200, 200, 200);
        s.bool_color = Color::fromRgb(200, 200, 200);
        s.null_color = Color::fromRgb(100, 100, 100);
        s.brace_color = Color::fromRgb(150, 150, 150);
        s.bracket_color = Color::fromRgb(150, 150, 150);
        s.key_quote_color = Color::fromRgb(150, 150, 150);  // Subtieler
        s.string_quote_color = Color::fromRgb(150, 150, 150);
        
    } else if (name == "neon") {
        s.key_color = Color{255, 0, 255};
        s.string_color = Color{0, 255, 255};
        s.number_color = Color{255, 255, 0};
        s.bool_color = Color{255, 100, 255};
        s.null_color = Color{150, 150, 150};
        s.brace_color = Color{255, 255, 255};
        s.key_quote_color = Color{255, 255, 255};  // Wit voor contrast
        s.string_quote_color = Color{255, 255, 255};
        s.key_color.bold = true;
        s.string_color.bold = true;
        
    } else {
        throw std::runtime_error("Unknown preset: " + name);
    }
    
    return s;
}

inline std::vector<std::string> Style::listPresets() {
    return {
        "default", "dracula", "solarized", "monokai", 
        "github", "minimal", "neon"
    };
}

} // namespace colored_json

