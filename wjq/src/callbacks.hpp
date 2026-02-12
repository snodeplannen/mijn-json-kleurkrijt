#pragma once
#include "color.hpp"
#include "style_context.hpp"
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace colored_json {

// Forward declarations
struct Color;
struct Style;

// Event types for callbacks
enum class JsonEventType {
  BeginObject,
  EndObject,
  BeginArray,
  EndArray,
  Key,
  StringValue,
  NumberValue,
  BooleanValue,
  NullValue,
  BeginDocument,
  EndDocument
};

// Event structure passed to callbacks
struct JsonEvent {
  JsonEventType type;
  std::string path;
  std::string key;              // For Key events
  std::string string_value;     // For StringValue
  double number_value = 0;      // For NumberValue
  bool bool_value = false;      // For BooleanValue
  int depth = 0;
  int array_index = -1;
  StyleContext context;
};

// Callback result - allows modifying behavior
struct CallbackResult {
  bool skip_element = false;           // Skip this element entirely
  bool skip_children = false;          // Skip children but keep element
  bool custom_color = false;           // Use custom color
  Color color{255, 255, 255};          // Custom color if set
  std::string replacement_text;        // Replace value with this text
  bool replace_value = false;
};

// Callback types
using ElementCallback = std::function<CallbackResult(const JsonEvent &)>;
using ValueTransformCallback = std::function<std::string(const std::string &original,
                                                          const StyleContext &ctx)>;
using ColorOverrideCallback = std::function<std::optional<Color>(const JsonEvent &,
                                                                  const Color &default_color)>;
using ProgressCallback = std::function<void(size_t bytes_processed,
                                             size_t total_bytes,
                                             const std::string &current_path)>;

// Callback registry
class CallbackRegistry {
public:
  // Register callbacks for specific events
  void on_element(JsonEventType type, ElementCallback cb) {
    element_callbacks_[type].push_back(std::move(cb));
  }

  void on_key(const std::string &key_pattern, ElementCallback cb) {
    key_callbacks_[key_pattern].push_back(std::move(cb));
  }

  void on_path(const std::string &path_pattern, ElementCallback cb) {
    path_callbacks_[path_pattern].push_back(std::move(cb));
  }

  void on_value_transform(ValueTransformCallback cb) {
    value_transforms_.push_back(std::move(cb));
  }

  void on_color_override(ColorOverrideCallback cb) {
    color_overrides_.push_back(std::move(cb));
  }

  void on_progress(ProgressCallback cb) {
    progress_callback_ = std::move(cb);
  }

  // Invoke callbacks
  CallbackResult invoke_element_callbacks(const JsonEvent &event) const {
    CallbackResult result;

    // First invoke type-based callbacks
    auto it = element_callbacks_.find(event.type);
    if (it != element_callbacks_.end()) {
      for (const auto &cb : it->second) {
        auto r = cb(event);
        merge_result(result, r);
        if (result.skip_element)
          break;
      }
    }

    // Then invoke key-based callbacks
    if (!event.key.empty()) {
      for (const auto &[pattern, cbs] : key_callbacks_) {
        if (match_pattern(event.key, pattern)) {
          for (const auto &cb : cbs) {
            auto r = cb(event);
            merge_result(result, r);
            if (result.skip_element)
              break;
          }
        }
        if (result.skip_element)
          break;
      }
    }

    // Finally invoke path-based callbacks
    for (const auto &[pattern, cbs] : path_callbacks_) {
      if (match_pattern(event.path, pattern)) {
        for (const auto &cb : cbs) {
          auto r = cb(event);
          merge_result(result, r);
          if (result.skip_element)
            break;
        }
      }
      if (result.skip_element)
        break;
    }

    return result;
  }

  std::string invoke_value_transforms(const std::string &value,
                                      const StyleContext &ctx) const {
    std::string result = value;
    for (const auto &cb : value_transforms_) {
      result = cb(result, ctx);
    }
    return result;
  }

  std::optional<Color> invoke_color_overrides(const JsonEvent &event,
                                               const Color &default_color) const {
    for (const auto &cb : color_overrides_) {
      auto result = cb(event, default_color);
      if (result.has_value()) {
        return result;
      }
    }
    return std::nullopt;
  }

  void invoke_progress(size_t processed, size_t total,
                       const std::string &path) const {
    if (progress_callback_) {
      progress_callback_(processed, total, path);
    }
  }

  // Check if any callbacks are registered
  bool has_callbacks() const {
    return !element_callbacks_.empty() || !key_callbacks_.empty() ||
           !path_callbacks_.empty() || !value_transforms_.empty() ||
           !color_overrides_.empty() || progress_callback_ != nullptr;
  }

  bool has_progress_callback() const { return progress_callback_ != nullptr; }

private:
  std::unordered_map<JsonEventType, std::vector<ElementCallback>> element_callbacks_;
  std::unordered_map<std::string, std::vector<ElementCallback>> key_callbacks_;
  std::unordered_map<std::string, std::vector<ElementCallback>> path_callbacks_;
  std::vector<ValueTransformCallback> value_transforms_;
  std::vector<ColorOverrideCallback> color_overrides_;
  ProgressCallback progress_callback_;

  static void merge_result(CallbackResult &base, const CallbackResult &other) {
    if (other.skip_element)
      base.skip_element = true;
    if (other.skip_children)
      base.skip_children = true;
    if (other.custom_color) {
      base.custom_color = true;
      base.color = other.color;
    }
    if (other.replace_value) {
      base.replace_value = true;
      base.replacement_text = other.replacement_text;
    }
  }

  static bool match_pattern(const std::string &value, const std::string &pattern) {
    if (pattern == "*")
      return true;
    if (pattern == value)
      return true;

    // Simple glob matching
    if (pattern.find('*') == std::string::npos)
      return false;

    // Convert simple glob to check
    size_t pos = 0;
    size_t pattern_pos = 0;

    while (pos < value.length() && pattern_pos < pattern.length()) {
      if (pattern[pattern_pos] == '*') {
        pattern_pos++;
        if (pattern_pos >= pattern.length())
          return true;

        char next_char = pattern[pattern_pos];
        while (pos < value.length() && value[pos] != next_char) {
          pos++;
        }
      } else if (pattern[pattern_pos] == value[pos]) {
        pos++;
        pattern_pos++;
      } else {
        return false;
      }
    }

    while (pattern_pos < pattern.length() && pattern[pattern_pos] == '*') {
      pattern_pos++;
    }

    return pos == value.length() && pattern_pos == pattern.length();
  }
};

// Pre-built callback factories
namespace callbacks {

// Hide sensitive fields (e.g., passwords, tokens)
inline ElementCallback hide_sensitive(const std::string &replacement = "***") {
  return [replacement](const JsonEvent &event) {
    CallbackResult result;
    result.replace_value = true;
    result.replacement_text = "\"" + replacement + "\"";
    return result;
  };
}

// Truncate long strings
inline ValueTransformCallback truncate_strings(size_t max_length,
                                                const std::string &suffix = "...") {
  return [max_length, suffix](const std::string &value,
                              const StyleContext &ctx) -> std::string {
    (void)ctx;
    if (value.length() <= max_length)
      return value;
    return value.substr(0, max_length) + suffix;
  };
}

// Format numbers with separators
inline ValueTransformCallback format_numbers(char separator = ',') {
  return [separator](const std::string &value,
                     const StyleContext &ctx) -> std::string {
    (void)ctx;
    // Check if it's an integer
    bool is_int = true;
    for (char c : value) {
      if (!std::isdigit(c) && c != '-') {
        is_int = false;
        break;
      }
    }

    if (!is_int)
      return value;

    // Add separators
    std::string result;
    int count = 0;
    for (auto it = value.rbegin(); it != value.rend(); ++it) {
      if (std::isdigit(*it)) {
        if (count > 0 && count % 3 == 0) {
          result += separator;
        }
        count++;
      }
      result += *it;
    }
    std::reverse(result.begin(), result.end());

    // Handle negative sign
    if (!value.empty() && value[0] == '-') {
      if (result.back() == '-') {
        result.pop_back();
        result = "-" + result;
      }
    }

    return result;
  };
}

// Highlight specific values
inline ColorOverrideCallback highlight_value(const std::string &target_value,
                                               const Color &highlight_color) {
  return [target_value, highlight_color](const JsonEvent &event,
                                         const Color &default_color) -> std::optional<Color> {
    (void)default_color;
    if (event.type == JsonEventType::StringValue &&
        event.string_value == target_value) {
      return highlight_color;
    }
    if (event.type == JsonEventType::NumberValue &&
        std::to_string(event.number_value) == target_value) {
      return highlight_color;
    }
    return std::nullopt;
  };
}

} // namespace callbacks

} // namespace colored_json
