#pragma once
#include <string>
#include <cstdint>
#include <sstream>
#include <cstdlib>

namespace colored_json {

enum class ColorMode {
    Auto,       // Detecteer automatisch
    Ansi16,     // 16 kleuren
    Ansi256,    // 256 kleuren
    TrueColor,  // 24-bit kleuren
    Disabled    // Geen kleuren
};

struct Color {
    uint8_t r, g, b;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    
    Color() : r(0), g(0), b(0) {}
    Color(uint8_t r_, uint8_t g_, uint8_t b_) : r(r_), g(g_), b(b_) {}
    
    std::string toAnsi(ColorMode mode = ColorMode::Auto) const;
    static Color fromRgb(uint8_t r, uint8_t g, uint8_t b) { return {r, g, b}; }
};

inline std::string Color::toAnsi(ColorMode mode) const {
    if (mode == ColorMode::Disabled) return "";
    
    // Auto-detectie
    if (mode == ColorMode::Auto) {
        const char* term = std::getenv("TERM");
        const char* colorterm = std::getenv("COLORTERM");
        
        if (colorterm && std::string(colorterm) == "truecolor") {
            mode = ColorMode::TrueColor;
        } else if (term && std::string(term).find("256") != std::string::npos) {
            mode = ColorMode::Ansi256;
        } else {
            mode = ColorMode::Ansi16;
        }
    }
    
    std::stringstream ss;
    ss << "\033[";
    
    if (mode == ColorMode::TrueColor) {
        ss << "38;2;" << static_cast<int>(r) << ';' 
           << static_cast<int>(g) << ';' << static_cast<int>(b);
    } else if (mode == ColorMode::Ansi256) {
        // Convert RGB naar 256-color palette
        uint8_t idx = 16 + (36 * (r / 51)) + (6 * (g / 51)) + (b / 51);
        ss << "38;5;" << static_cast<int>(idx);
    } else { // Ansi16
        // Simpelste mapping: gebruik brightness
        bool bright = (r + g + b) > 382; // > 1.5 * 255
        ss << (bright ? "9" : "3");
        
        // Kies kleur op basis van dominante component
        if (r > g && r > b) ss << "1";      // Rood
        else if (g > r && g > b) ss << "2"; // Groen
        else if (b > r && b > g) ss << "4"; // Blauw
        else if (r > 0 && g > 0 && b == 0) ss << "3"; // Geel
        else if (r > 0 && b > 0 && g == 0) ss << "5"; // Magenta
        else if (g > 0 && b > 0 && r == 0) ss << "6"; // Cyaan
        else ss << "7"; // Wit/grijs
    }
    
    if (bold) ss << ";1";
    if (italic) ss << ";3";
    if (underline) ss << ";4";
    
    ss << "m";
    return ss.str();
}

// Ingebouwde ANSI16 kleuren
namespace colors {
    const Color black{0, 0, 0};
    const Color red{170, 0, 0};
    const Color green{0, 170, 0};
    const Color yellow{170, 85, 0};
    const Color blue{0, 0, 170};
    const Color magenta{170, 0, 170};
    const Color cyan{0, 170, 170};
    const Color white{170, 170, 170};
    const Color bright_black{85, 85, 85};
    const Color bright_red{255, 85, 85};
    const Color bright_green{85, 255, 85};
    const Color bright_yellow{255, 255, 85};
    const Color bright_blue{85, 85, 255};
    const Color bright_magenta{255, 85, 255};
    const Color bright_cyan{85, 255, 255};
    const Color bright_white{255, 255, 255};
}

} // namespace colored_json

