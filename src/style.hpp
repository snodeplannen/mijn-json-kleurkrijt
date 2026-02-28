#pragma once
#include "color.hpp"
#include "matchers.hpp"
#include "style_context.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace colored_json {

// Type aliases for backwards compatibility
using KeyColorsMap = std::unordered_map<std::string, Color>;
using ValueColorsMap = std::unordered_map<std::string, Color>;

enum class ElementType {
  Key,
  String,
  Number,
  Boolean,
  Null,
  Brace,   // { }
  Bracket, // [ ]
  Colon,
  Comma,
  Whitespace, // For indented mode
};

// A styling rule with conditions
struct StyleRule {
  // What this rule applies to
  std::vector<ElementType> element_types;

  // Matchers for value-based styling
  std::unique_ptr<Matcher> value_matcher;

  // Path pattern (glob-style)
  std::string path_pattern;

  // Depth constraints
  std::optional<std::pair<int, int>> depth_range; // min, max (-1 for unlimited)

  // Array index constraints
  std::optional<std::string>
      array_index_pattern; // "even", "odd", "first", "last", "every:N"
  std::optional<std::pair<int, int>> array_index_range;

  // Priority (higher = applied later, can override)
  int priority = 0;

  // The color to apply when matched
  Color color;

  // Allow move semantics
  StyleRule() = default;
  StyleRule(StyleRule &&) = default;
  StyleRule &operator=(StyleRule &&) = default;

  // Delete copy to avoid issues with unique_ptr
  StyleRule(const StyleRule &) = delete;
  StyleRule &operator=(const StyleRule &) = delete;

  // Constructor helpers
  StyleRule &for_elements(std::vector<ElementType> types) {
    element_types = std::move(types);
    return *this;
  }

  StyleRule &when_value(std::unique_ptr<Matcher> matcher) {
    value_matcher = std::move(matcher);
    return *this;
  }

  StyleRule &at_path(const std::string &pattern) {
    path_pattern = pattern;
    return *this;
  }

  StyleRule &at_depth(int min_depth, int max_depth = -1) {
    depth_range = std::make_pair(min_depth, max_depth);
    return *this;
  }

  StyleRule &at_array_index(const std::string &pattern) {
    array_index_pattern = pattern;
    return *this;
  }

  StyleRule &at_array_index(int min_idx, int max_idx = -1) {
    array_index_range = std::make_pair(min_idx, max_idx);
    return *this;
  }

  StyleRule &with_priority(int p) {
    priority = p;
    return *this;
  }

  StyleRule &use_color(const Color &c) {
    color = c;
    return *this;
  }

  // Check if this rule applies to given context
  bool matches(ElementType type, const StyleContext &ctx) const {
    // Check element type
    if (!element_types.empty()) {
      bool type_matches = std::find(element_types.begin(), element_types.end(),
                                    type) != element_types.end();
      if (!type_matches)
        return false;
    }

    // Check path pattern
    if (!path_pattern.empty() && !ctx.path_matches(path_pattern)) {
      return false;
    }

    // Check depth range
    if (depth_range.has_value()) {
      auto [min_d, max_d] = depth_range.value();
      if (!ctx.depth_in_range(min_d, max_d)) {
        return false;
      }
    }

    // Check array index constraints
    if (array_index_pattern.has_value()) {
      if (!ctx.array_index_matches(array_index_pattern.value())) {
        return false;
      }
    }

    if (array_index_range.has_value()) {
      auto [min_idx, max_idx] = array_index_range.value();
      if (!ctx.array_index_in_range(min_idx, max_idx)) {
        return false;
      }
    }

    return true;
  }

  // Check if value matches (for string/number values)
  bool matches_value(const std::string &value, const StyleContext &ctx) const {
    if (!value_matcher)
      return true; // No value constraint means match all
    return value_matcher->match(value, ctx).matched;
  }

  // Clone for copying - returns unique_ptr for Style's copy constructor
  std::unique_ptr<StyleRule> clone() const {
    auto copy = std::make_unique<StyleRule>();
    copy->element_types = element_types;
    if (value_matcher) {
      copy->value_matcher = value_matcher->clone();
    }
    copy->path_pattern = path_pattern;
    copy->depth_range = depth_range;
    copy->array_index_pattern = array_index_pattern;
    copy->array_index_range = array_index_range;
    copy->priority = priority;
    copy->color = color;
    return copy;
  }

  // Legacy clone returning unique_ptr for compatibility
  std::unique_ptr<StyleRule> clone_unique() const { return clone(); }
};

// Main Style class
struct Style {
  // Base colors
  Color key_color{93, 173, 226};           // Light blue
  Color string_color{152, 224, 36};        // Lime green
  Color number_color{255, 203, 107};       // Orange
  Color bool_color{255, 121, 198};         // Pink
  Color null_color{137, 137, 137};         // Gray
  Color brace_color{255, 255, 255};        // White
  Color bracket_color{255, 255, 255};      // White
  Color colon_color{255, 255, 255};        // White
  Color comma_color{255, 255, 255};        // White
  Color key_quote_color{255, 255, 255};    // White
  Color string_quote_color{255, 255, 255}; // White

  // Legacy maps for backwards compatibility
  KeyColorsMap key_colors;
  ValueColorsMap value_colors;

  // New rule-based styling - stored as unique_ptrs to allow copying of Style
  std::vector<std::unique_ptr<StyleRule>> rules;

  // Settings
  ColorMode color_mode = ColorMode::Auto;
  bool compact = false;
  int indent_size = 2;

  // Default constructor
  Style() = default;

  // Copy constructor - clones the rules
  Style(const Style &other)
      : key_color(other.key_color), string_color(other.string_color),
        number_color(other.number_color), bool_color(other.bool_color),
        null_color(other.null_color), brace_color(other.brace_color),
        bracket_color(other.bracket_color), colon_color(other.colon_color),
        comma_color(other.comma_color), key_quote_color(other.key_quote_color),
        string_quote_color(other.string_quote_color),
        key_colors(other.key_colors), value_colors(other.value_colors),
        color_mode(other.color_mode), compact(other.compact),
        indent_size(other.indent_size) {
    // Clone each rule
    for (const auto &rule_ptr : other.rules) {
      rules.push_back(rule_ptr->clone());
    }
  }

  // Copy assignment
  Style &operator=(const Style &other) {
    if (this != &other) {
      key_color = other.key_color;
      string_color = other.string_color;
      number_color = other.number_color;
      bool_color = other.bool_color;
      null_color = other.null_color;
      brace_color = other.brace_color;
      bracket_color = other.bracket_color;
      colon_color = other.colon_color;
      comma_color = other.comma_color;
      key_quote_color = other.key_quote_color;
      string_quote_color = other.string_quote_color;
      key_colors = other.key_colors;
      value_colors = other.value_colors;
      color_mode = other.color_mode;
      compact = other.compact;
      indent_size = other.indent_size;

      // Clone each rule
      rules.clear();
      for (const auto &rule_ptr : other.rules) {
        rules.push_back(rule_ptr->clone());
      }
    }
    return *this;
  }

  // Move constructor
  Style(Style &&) = default;

  // Move assignment
  Style &operator=(Style &&) = default;

  // Get base color for element type
  Color get_base_color(ElementType type) const {
    switch (type) {
    case ElementType::Key:
      return key_color;
    case ElementType::String:
      return string_color;
    case ElementType::Number:
      return number_color;
    case ElementType::Boolean:
      return bool_color;
    case ElementType::Null:
      return null_color;
    case ElementType::Brace:
      return brace_color;
    case ElementType::Bracket:
      return bracket_color;
    case ElementType::Colon:
      return colon_color;
    case ElementType::Comma:
      return comma_color;
    case ElementType::Whitespace:
      return Color{255, 255, 255};
    }
    return Color{255, 255, 255};
  }

  // Get color for element with full context
  Color get_color(ElementType type, const StyleContext &ctx,
                  const std::string &value = "") const {
    Color result = get_base_color(type);

    // Apply legacy key_colors for keys
    if (type == ElementType::Key && !key_colors.empty()) {
      auto it = key_colors.find(ctx.key_name);
      if (it != key_colors.end()) {
        result = it->second;
      }
    }

    // Apply legacy value_colors
    if (!value_colors.empty() && !ctx.path.empty()) {
      auto it = value_colors.find(ctx.path);
      if (it != value_colors.end()) {
        result = it->second;
      }
    }

    // Apply rules in priority order
    std::vector<const StyleRule *> matching_rules;
    for (const auto &rule_ptr : rules) {
      if (rule_ptr->matches(type, ctx)) {
        if (value.empty() || rule_ptr->matches_value(value, ctx)) {
          matching_rules.push_back(rule_ptr.get());
        }
      }
    }

    // Sort by priority and apply last (highest priority) match
    std::sort(matching_rules.begin(), matching_rules.end(),
              [](const StyleRule *a, const StyleRule *b) {
                return a->priority < b->priority;
              });

    if (!matching_rules.empty()) {
      result = matching_rules.back()->color;
    }

    return result;
  }

  // Get quote color for element type
  Color get_quote_color(ElementType type) const {
    if (type == ElementType::Key)
      return key_quote_color;
    if (type == ElementType::String)
      return string_quote_color;
    return Color{255, 255, 255};
  }

  // Helper to add a rule
  StyleRule &add_rule() {
    auto rule = std::make_unique<StyleRule>();
    StyleRule &ref = *rule;
    rules.push_back(std::move(rule));
    return ref;
  }

  // Legacy helpers
  void setKeyColor(const std::string &key, const Color &color) {
    key_colors[key] = color;
  }

  void setValueColor(const std::string &path, const Color &color) {
    value_colors[path] = color;
  }

  // Preset loading
  static Style getPreset(const std::string &name);
  static std::vector<std::string> listPresets();

  // Builder pattern for fluent interface
  Style &with_compact(bool c) {
    compact = c;
    return *this;
  }

  Style &with_indent(int size) {
    indent_size = size;
    return *this;
  }

  Style &with_color_mode(ColorMode mode) {
    color_mode = mode;
    return *this;
  }
};

} // namespace colored_json
