#pragma once
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <string>

namespace colored_json {

enum class ColorMode {
  Auto,      // Detect automatically
  Ansi16,    // 16 colors
  Ansi256,   // 256 colors
  TrueColor, // 24-bit colors
  Disabled   // No colors
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

  // Fluent interface for styling
  Color with_bold(bool b = true) const {
    Color c = *this;
    c.bold = b;
    return c;
  }

  Color with_italic(bool i = true) const {
    Color c = *this;
    c.italic = i;
    return c;
  }

  Color with_underline(bool u = true) const {
    Color c = *this;
    c.underline = u;
    return c;
  }

  // Blend with another color
  Color blend(const Color &other, float ratio = 0.5f) const {
    Color result;
    result.r = static_cast<uint8_t>(r * (1 - ratio) + other.r * ratio);
    result.g = static_cast<uint8_t>(g * (1 - ratio) + other.g * ratio);
    result.b = static_cast<uint8_t>(b * (1 - ratio) + other.b * ratio);
    result.bold = bold || other.bold;
    result.italic = italic || other.italic;
    result.underline = underline || other.underline;
    return result;
  }

  // Darken/lighten
  Color darken(float amount = 0.2f) const {
    Color result;
    result.r = static_cast<uint8_t>(r * (1 - amount));
    result.g = static_cast<uint8_t>(g * (1 - amount));
    result.b = static_cast<uint8_t>(b * (1 - amount));
    result.bold = bold;
    result.italic = italic;
    result.underline = underline;
    return result;
  }

  Color lighten(float amount = 0.2f) const {
    Color result;
    result.r = static_cast<uint8_t>(r + (255 - r) * amount);
    result.g = static_cast<uint8_t>(g + (255 - g) * amount);
    result.b = static_cast<uint8_t>(b + (255 - b) * amount);
    result.bold = bold;
    result.italic = italic;
    result.underline = underline;
    return result;
  }

  // Check if color is dark (for contrast calculations)
  bool is_dark() const { return (r + g + b) < 384; } // < 1.5 * 255

  // Get contrasting color (black or white)
  Color contrast() const { return is_dark() ? Color{255, 255, 255} : Color{0, 0, 0}; }

  bool operator==(const Color &other) const {
    return r == other.r && g == other.g && b == other.b && bold == other.bold &&
           italic == other.italic && underline == other.underline;
  }

  bool operator!=(const Color &other) const { return !(*this == other); }
};

inline std::string Color::toAnsi(ColorMode mode) const {
  if (mode == ColorMode::Disabled)
    return "";

  // Auto-detect
  if (mode == ColorMode::Auto) {
    const char *term = std::getenv("TERM");
    const char *colorterm = std::getenv("COLORTERM");

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
    ss << "38;2;" << static_cast<int>(r) << ';' << static_cast<int>(g) << ';'
       << static_cast<int>(b);
  } else if (mode == ColorMode::Ansi256) {
    // Convert RGB to 256-color palette
    uint8_t idx = 16 + (36 * (r / 51)) + (6 * (g / 51)) + (b / 51);
    ss << "38;5;" << static_cast<int>(idx);
  } else { // Ansi16
    // Simplest mapping: use brightness
    bool bright = (r + g + b) > 382; // > 1.5 * 255
    ss << (bright ? "9" : "3");

    // Choose color based on dominant component
    if (r > g && r > b)
      ss << "1"; // Red
    else if (g > r && g > b)
      ss << "2"; // Green
    else if (b > r && b > g)
      ss << "4"; // Blue
    else if (r > 0 && g > 0 && b == 0)
      ss << "3"; // Yellow
    else if (r > 0 && b > 0 && g == 0)
      ss << "5"; // Magenta
    else if (g > 0 && b > 0 && r == 0)
      ss << "6"; // Cyan
    else
      ss << "7"; // White/gray
  }

  if (bold)
    ss << ";1";
  if (italic)
    ss << ";3";
  if (underline)
    ss << ";4";

  ss << "m";
  return ss.str();
}

// Built-in ANSI16 colors
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
} // namespace colors

} // namespace colored_json
