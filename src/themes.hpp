#pragma once
#include "style.hpp"

namespace colored_json {

// Helper to configure boolean value-dependent coloring (true=green, false=red)
inline void apply_boolean_rules(Style &s) {
  s.add_rule()
      .for_elements({ElementType::Boolean})
      .when_value(keyword({"true"}, false))
      .use_color(Color{50, 255, 50}) // Green
      .with_priority(50);

  s.add_rule()
      .for_elements({ElementType::Boolean})
      .when_value(keyword({"false"}, false))
      .use_color(Color{255, 50, 50}) // Red
      .with_priority(50);
}

// Helper to create the default/dracula theme
inline Style make_default_theme() {
  Style s;
  s.key_color = Color{93, 173, 226};
  s.string_color = Color{152, 224, 36};
  s.number_color = Color{255, 203, 107};
  s.bool_color = Color{255, 121, 198};
  s.null_color = Color{137, 137, 137};
  s.key_quote_color = Color{93, 173, 226};
  s.string_quote_color = Color{152, 224, 36};
  apply_boolean_rules(s);
  return s;
}

inline Style make_dracula_theme() {
  Style s;
  s.key_color = Color{241, 250, 140};    // Yellow
  s.string_color = Color{80, 250, 123};  // Green
  s.number_color = Color{189, 147, 249}; // Purple
  s.bool_color = Color{255, 121, 198};   // Pink
  s.null_color = Color{98, 114, 164};    // Comment gray
  s.brace_color = Color{248, 248, 242};  // Foreground
  s.bracket_color = Color{248, 248, 242};
  s.key_quote_color = Color{241, 250, 140};
  s.string_quote_color = Color{80, 250, 123};
  apply_boolean_rules(s);
  return s;
}

inline Style make_solarized_theme() {
  Style s;
  s.key_color = Color{38, 139, 210};    // Blue
  s.string_color = Color{42, 161, 152}; // Cyan
  s.number_color = Color{181, 137, 0};  // Yellow
  s.bool_color = Color{211, 54, 130};   // Magenta
  s.null_color = Color{131, 148, 150};  // Base0
  s.brace_color = Color{88, 110, 117};  // Base01
  s.bracket_color = Color{88, 110, 117};
  s.key_quote_color = Color{88, 110, 117};
  s.string_quote_color = Color{88, 110, 117};
  apply_boolean_rules(s);
  return s;
}

inline Style make_monokai_theme() {
  Style s;
  s.key_color = Color{156, 220, 254};    // Light blue
  s.string_color = Color{206, 145, 120}; // Orange
  s.number_color = Color{181, 206, 168}; // Light green
  s.bool_color = Color{190, 132, 255};   // Purple
  s.null_color = Color{156, 156, 156};   // Gray
  s.brace_color = Color{248, 248, 242};  // White
  s.bracket_color = Color{248, 248, 242};
  s.key_quote_color = Color{248, 248, 242};
  s.string_quote_color = Color{248, 248, 242};
  apply_boolean_rules(s);
  return s;
}

inline Style make_github_theme() {
  Style s;
  s.key_color = Color{3, 47, 98};      // Dark blue
  s.string_color = Color{3, 106, 7};   // Green
  s.number_color = Color{28, 0, 207};  // Blue
  s.bool_color = Color{0, 16, 128};    // Navy
  s.null_color = Color{128, 128, 128}; // Gray
  s.brace_color = Color{36, 41, 46};   // Dark gray
  s.bracket_color = Color{36, 41, 46};
  s.key_quote_color = Color{36, 41, 46};
  s.string_quote_color = Color{36, 41, 46};
  apply_boolean_rules(s);
  return s;
}

inline Style make_minimal_theme() {
  Style s;
  s.key_color = Color::fromRgb(200, 200, 200);
  s.string_color = Color::fromRgb(255, 255, 255); // White strings
  s.number_color = Color::fromRgb(255, 255, 255); // White numbers
  s.bool_color = Color::fromRgb(255, 255, 255);   // White booleans
  s.null_color = Color::fromRgb(128, 128, 128);
  s.brace_color = Color::fromRgb(180, 180, 180);
  s.bracket_color = Color::fromRgb(180, 180, 180);
  s.colon_color = Color::fromRgb(150, 150, 150);
  s.comma_color = Color::fromRgb(150, 150, 150);
  s.key_quote_color = Color::fromRgb(150, 150, 150);
  s.string_quote_color = Color::fromRgb(180, 180, 180);
  apply_boolean_rules(s);
  return s;
}

inline Style make_neon_theme() {
  Style s;
  s.key_color = Color{255, 0, 255};
  s.string_color = Color{0, 255, 255};
  s.number_color = Color{255, 255, 0};
  s.bool_color = Color{255, 100, 255};
  s.null_color = Color{150, 150, 150};
  s.brace_color = Color{255, 255, 255};
  s.bracket_color = Color{255, 255, 255};
  s.key_quote_color = Color{255, 255, 255};
  s.string_quote_color = Color{255, 255, 255};
  s.key_color.bold = true;
  s.string_color.bold = true;
  apply_boolean_rules(s);
  return s;
}

// NEW THEMES

inline Style make_ocean_theme() {
  Style s;
  s.key_color = Color{122, 162, 247};    // Blue
  s.string_color = Color{158, 206, 106}; // Green
  s.number_color = Color{255, 199, 119}; // Orange
  s.bool_color = Color{187, 154, 247};   // Purple
  s.null_color = Color{86, 95, 137};     // Dark blue-gray
  s.brace_color = Color{192, 202, 245};  // Light text
  s.bracket_color = Color{192, 202, 245};
  s.key_quote_color = Color{122, 162, 247};
  s.string_quote_color = Color{158, 206, 106};
  s.colon_color = Color{86, 95, 137};
  s.comma_color = Color{86, 95, 137};
  apply_boolean_rules(s);
  return s;
}

inline Style make_forest_theme() {
  Style s;
  s.key_color = Color{129, 178, 154};    // Sage green
  s.string_color = Color{224, 160, 109}; // Terracotta
  s.number_color = Color{242, 204, 107}; // Gold
  s.bool_color = Color{183, 110, 121};   // Dusty rose
  s.null_color = Color{120, 120, 120};   // Gray
  s.brace_color = Color{224, 224, 224};  // Light
  s.bracket_color = Color{224, 224, 224};
  s.key_quote_color = Color{129, 178, 154};
  s.string_quote_color = Color{224, 160, 109};
  apply_boolean_rules(s);
  return s;
}

inline Style make_cyberpunk_theme() {
  Style s;
  s.key_color = Color{0, 255, 136};     // Matrix green
  s.string_color = Color{255, 0, 85};   // Hot pink
  s.number_color = Color{0, 204, 255};  // Cyan
  s.bool_color = Color{255, 170, 0};    // Amber
  s.null_color = Color{85, 85, 85};     // Dark gray
  s.brace_color = Color{255, 255, 255}; // White
  s.bracket_color = Color{255, 255, 255};
  s.key_quote_color = Color{0, 255, 136};
  s.string_quote_color = Color{255, 0, 85};
  s.key_color.bold = true;
  s.string_color.bold = true;
  s.number_color.bold = true;
  apply_boolean_rules(s);
  return s;
}

inline Style make_sunset_theme() {
  Style s;
  s.key_color = Color{255, 154, 118};    // Coral
  s.string_color = Color{255, 206, 109}; // Yellow
  s.number_color = Color{255, 107, 107}; // Red
  s.bool_color = Color{162, 155, 254};   // Lavender
  s.null_color = Color{99, 99, 99};      // Dark gray
  s.brace_color = Color{255, 255, 255};
  s.bracket_color = Color{255, 255, 255};
  s.key_quote_color = Color{255, 154, 118};
  s.string_quote_color = Color{255, 206, 109};
  apply_boolean_rules(s);
  return s;
}

inline Style make_high_contrast_theme() {
  Style s;
  s.key_color = Color{0, 150, 255};    // Bright blue
  s.string_color = Color{0, 200, 0};   // Bright green
  s.number_color = Color{255, 140, 0}; // Orange
  s.bool_color = Color{255, 0, 255};   // Magenta
  s.null_color = Color{128, 128, 128}; // Gray
  s.brace_color = Color{255, 255, 0};  // Yellow
  s.bracket_color = Color{255, 255, 0};
  s.key_quote_color = Color{255, 255, 255};
  s.string_quote_color = Color{255, 255, 255};
  s.key_color.bold = true;
  s.string_color.bold = true;
  s.number_color.bold = true;
  s.bool_color.bold = true;
  apply_boolean_rules(s);
  return s;
}

// Advanced theme with conditional rules - highlights errors and warnings
inline Style make_debug_theme() {
  Style s;
  s.key_color = Color{200, 200, 200};
  s.string_color = Color{255, 255, 255};
  s.number_color = Color{180, 180, 180};
  s.bool_color = Color{255, 255, 0};
  s.null_color = Color{128, 128, 128};
  s.brace_color = Color{100, 100, 100};
  s.bracket_color = Color{100, 100, 100};

  // Highlight error values in red
  s.add_rule()
      .for_elements({ElementType::String, ElementType::Key})
      .when_value(keyword({"error", "failed", "exception", "fatal"}, false))
      .use_color(Color{255, 50, 50}.with_bold(true))
      .with_priority(10);

  // Highlight warning values in yellow
  s.add_rule()
      .for_elements({ElementType::String, ElementType::Key})
      .when_value(keyword({"warning", "warn", "caution", "deprecated"}, false))
      .use_color(Color{255, 200, 0}.with_bold(true))
      .with_priority(9);

  // Highlight success values in green
  s.add_rule()
      .for_elements({ElementType::String})
      .when_value(keyword({"success", "ok", "done", "passed", "true"}, false))
      .use_color(Color{50, 255, 50})
      .with_priority(8);

  // Highlight large numbers (>1000) in orange
  s.add_rule()
      .for_elements({ElementType::Number})
      .when_value(greater_than(1000))
      .use_color(Color{255, 150, 50})
      .with_priority(5);

  // Highlight negative numbers in red
  s.add_rule()
      .for_elements({ElementType::Number})
      .when_value(less_than(0))
      .use_color(Color{255, 100, 100})
      .with_priority(5);

  apply_boolean_rules(s);
  return s;
}

// Check if debug theme has rules - helper for tests
inline bool debug_theme_has_rules() {
  auto s = make_debug_theme();
  return !s.rules.empty();
}

// Theme that highlights nested structures by depth
inline Style make_depth_aware_theme() {
  Style s;

  // Root level - bright colors
  s.add_rule()
      .for_elements({ElementType::Key})
      .at_depth(0, 0)
      .use_color(Color{255, 100, 100})
      .with_priority(100);

  // Level 1 - warm colors
  s.add_rule()
      .for_elements({ElementType::Key})
      .at_depth(1, 1)
      .use_color(Color{255, 200, 100})
      .with_priority(100);

  // Level 2 - yellow/green
  s.add_rule()
      .for_elements({ElementType::Key})
      .at_depth(2, 2)
      .use_color(Color{200, 255, 100})
      .with_priority(100);

  // Level 3+ - cool colors fading to blue
  s.add_rule()
      .for_elements({ElementType::Key})
      .at_depth(3)
      .use_color(Color{100, 200, 255})
      .with_priority(100);

  // Strings get similar treatment
  s.string_color = Color{180, 180, 180};
  s.number_color = Color{255, 180, 120};
  s.bool_color = Color{200, 150, 255};
  s.null_color = Color{120, 120, 120};

  apply_boolean_rules(s);
  return s;
}

// Theme for data analysis - highlights patterns in arrays
inline Style make_data_analysis_theme() {
  Style s;
  s.key_color = Color{150, 150, 150};
  s.string_color = Color{200, 200, 200};
  s.number_color = Color{255, 255, 255};
  s.bool_color = Color{180, 180, 255};
  s.null_color = Color{100, 100, 100};

  // First array item in green
  s.add_rule()
      .for_elements({ElementType::Number, ElementType::String})
      .at_array_index("first")
      .use_color(Color{100, 255, 100})
      .with_priority(10);

  // Last array item in red
  s.add_rule()
      .for_elements({ElementType::Number, ElementType::String})
      .at_array_index("last")
      .use_color(Color{255, 100, 100})
      .with_priority(10);

  // Even indices in blue
  s.add_rule()
      .for_elements({ElementType::Number})
      .at_array_index("even")
      .use_color(Color{100, 150, 255})
      .with_priority(5);

  // Odd indices in purple
  s.add_rule()
      .for_elements({ElementType::Number})
      .at_array_index("odd")
      .use_color(Color{200, 100, 255})
      .with_priority(5);

  // Every 5th item highlighted
  s.add_rule()
      .for_elements({ElementType::Number})
      .at_array_index("every:5")
      .use_color(Color{255, 200, 100})
      .with_priority(8);

  apply_boolean_rules(s);
  return s;
}

// New themes with WHITE strings

inline Style make_white_theme() {
  Style s;
  s.key_color = Color{100, 170, 255};    // Light blue keys
  s.string_color = Color{255, 255, 255}; // WHITE strings
  s.number_color = Color{255, 200, 100}; // Orange numbers
  s.bool_color = Color{255, 100, 200};   // Pink booleans
  s.null_color = Color{128, 128, 128};   // Gray null
  s.brace_color = Color{200, 200, 200};  // Light gray braces
  s.bracket_color = Color{200, 200, 200};
  s.colon_color = Color{150, 150, 150};
  s.comma_color = Color{150, 150, 150};
  s.key_quote_color = Color{100, 170, 255};
  s.string_quote_color = Color{255, 255, 255};
  apply_boolean_rules(s);
  return s;
}

inline Style make_nord_theme() {
  // Nord-inspired theme with white strings
  Style s;
  s.key_color = Color{129, 161, 193};    // Nord blue
  s.string_color = Color{236, 239, 244}; // Snow storm (white-ish)
  s.number_color = Color{180, 142, 173}; // Nord purple
  s.bool_color = Color{163, 190, 140};   // Nord green
  s.null_color = Color{76, 86, 106};     // Dark gray
  s.brace_color = Color{216, 222, 233};  // Light gray
  s.bracket_color = Color{216, 222, 233};
  s.colon_color = Color{143, 188, 187}; // Nord teal
  s.comma_color = Color{143, 188, 187};
  s.key_quote_color = Color{129, 161, 193};
  s.string_quote_color = Color{236, 239, 244};
  apply_boolean_rules(s);
  return s;
}

inline Style make_gruvbox_theme() {
  // Gruvbox-inspired theme with cream/white strings
  Style s;
  s.key_color = Color{131, 165, 152};    // Aqua
  s.string_color = Color{251, 241, 199}; // Cream/white
  s.number_color = Color{254, 128, 25};  // Orange
  s.bool_color = Color{184, 187, 38};    // Yellow-green
  s.null_color = Color{146, 131, 116};   // Gray
  s.brace_color = Color{235, 219, 178};  // Light beige
  s.bracket_color = Color{235, 219, 178};
  s.colon_color = Color{168, 153, 132}; // Gray-brown
  s.comma_color = Color{168, 153, 132};
  s.key_quote_color = Color{131, 165, 152};
  s.string_quote_color = Color{251, 241, 199};
  apply_boolean_rules(s);
  return s;
}

inline Style make_one_dark_theme() {
  // One Dark-inspired theme with white strings
  Style s;
  s.key_color = Color{86, 156, 214};     // Blue
  s.string_color = Color{206, 210, 205}; // White/smoke
  s.number_color = Color{181, 206, 168}; // Green
  s.bool_color = Color{197, 134, 192};   // Purple
  s.null_color = Color{100, 100, 100};   // Gray
  s.brace_color = Color{220, 220, 220};  // Light gray
  s.bracket_color = Color{220, 220, 220};
  s.colon_color = Color{150, 150, 150};
  s.comma_color = Color{150, 150, 150};
  s.key_quote_color = Color{86, 156, 214};
  s.string_quote_color = Color{206, 210, 205};
  apply_boolean_rules(s);
  return s;
}

inline Style make_catppuccin_theme() {
  // Catppuccin-inspired theme with white strings
  Style s;
  s.key_color = Color{140, 170, 238};    // Lavender
  s.string_color = Color{205, 214, 244}; // Text (white)
  s.number_color = Color{250, 179, 135}; // Peach
  s.bool_color = Color{166, 218, 149};   // Green
  s.null_color = Color{108, 112, 134};   // Surface 2
  s.brace_color = Color{180, 190, 254};  // Lavender light
  s.bracket_color = Color{180, 190, 254};
  s.colon_color = Color{147, 153, 178}; // Overlay 0
  s.comma_color = Color{147, 153, 178};
  s.key_quote_color = Color{140, 170, 238};
  s.string_quote_color = Color{205, 214, 244};
  apply_boolean_rules(s);
  return s;
}

inline Style make_ice_theme() {
  // Ice/cold theme with white strings
  Style s;
  s.key_color = Color{100, 149, 237};    // Cornflower blue
  s.string_color = Color{255, 255, 255}; // WHITE
  s.number_color = Color{173, 216, 230}; // Light blue
  s.bool_color = Color{224, 255, 255};   // Cyan/white
  s.null_color = Color{112, 128, 144};   // Slate gray
  s.brace_color = Color{176, 196, 222};  // Light steel blue
  s.bracket_color = Color{176, 196, 222};
  s.colon_color = Color{135, 206, 250}; // Light sky blue
  s.comma_color = Color{135, 206, 250};
  s.key_quote_color = Color{100, 149, 237};
  s.string_quote_color = Color{255, 255, 255};
  apply_boolean_rules(s);
  return s;
}

inline Style make_coffee_theme() {
  // Coffee/warm theme with cream strings
  Style s;
  s.key_color = Color{205, 133, 63};     // Peru/coffee
  s.string_color = Color{255, 248, 220}; // Cornsilk (cream white)
  s.number_color = Color{222, 184, 135}; // Burlywood
  s.bool_color = Color{210, 180, 140};   // Tan
  s.null_color = Color{139, 119, 101};   // Dark brown gray
  s.brace_color = Color{245, 222, 179};  // Wheat
  s.bracket_color = Color{245, 222, 179};
  s.colon_color = Color{188, 143, 143}; // Rosy brown
  s.comma_color = Color{188, 143, 143};
  s.key_quote_color = Color{205, 133, 63};
  s.string_quote_color = Color{255, 248, 220};
  apply_boolean_rules(s);
  return s;
}

// Preset loader implementation
inline Style Style::getPreset(const std::string &name) {
  if (name == "default" || name == "dracula")
    return make_default_theme();
  if (name == "solarized")
    return make_solarized_theme();
  if (name == "monokai")
    return make_monokai_theme();
  if (name == "github")
    return make_github_theme();
  if (name == "minimal")
    return make_minimal_theme();
  if (name == "neon")
    return make_neon_theme();
  if (name == "ocean")
    return make_ocean_theme();
  if (name == "forest")
    return make_forest_theme();
  if (name == "cyberpunk")
    return make_cyberpunk_theme();
  if (name == "sunset")
    return make_sunset_theme();
  if (name == "high-contrast")
    return make_high_contrast_theme();
  if (name == "debug")
    return make_debug_theme();
  if (name == "depth-aware")
    return make_depth_aware_theme();
  if (name == "data-analysis")
    return make_data_analysis_theme();
  // New white-string themes
  if (name == "white")
    return make_white_theme();
  if (name == "nord")
    return make_nord_theme();
  if (name == "gruvbox")
    return make_gruvbox_theme();
  if (name == "one-dark")
    return make_one_dark_theme();
  if (name == "catppuccin")
    return make_catppuccin_theme();
  if (name == "ice")
    return make_ice_theme();
  if (name == "coffee")
    return make_coffee_theme();

  throw std::runtime_error("Unknown preset: " + name);
}

inline std::vector<std::string> Style::listPresets() {
  return {"default", "dracula",     "solarized",     "monokai",
          "github",  "minimal",     "neon",          "ocean",
          "forest",  "cyberpunk",   "sunset",        "high-contrast",
          "debug",   "depth-aware", "data-analysis", "white",
          "nord",    "gruvbox",     "one-dark",      "catppuccin",
          "ice",     "coffee"};
}

} // namespace colored_json
