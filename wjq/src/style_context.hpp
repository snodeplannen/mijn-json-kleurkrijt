#pragma once
#include <string>
#include <vector>

namespace colored_json {

// Context information passed during styling decisions
struct StyleContext {
  // Current JSON path (e.g., "users.0.name")
  std::string path;

  // Path components for easy access
  std::vector<std::string> path_components;

  // Current depth in the JSON tree
  int depth = 0;

  // If inside an array: current index
  int array_index = -1;

  // Total items in current array (if known)
  int array_length = -1;

  // If inside an object: the key name
  std::string key_name;

  // Parent element type
  enum class ParentType { None, Object, Array, Root } parent_type = ParentType::None;

  // Value type (as string for easy matching)
  std::string value_type;

  // Original raw value (for matching)
  std::string raw_value;

  // Methods to manipulate context
  StyleContext enter_object(const std::string &key) const {
    StyleContext child = *this;
    child.depth++;
    child.key_name = key;
    child.parent_type = ParentType::Object;
    child.path = path.empty() ? key : path + "." + key;
    child.path_components.push_back(key);
    child.array_index = -1;
    child.array_length = -1;
    return child;
  }

  StyleContext enter_array(int index, int length = -1) const {
    StyleContext child = *this;
    child.depth++;
    child.array_index = index;
    child.array_length = length;
    child.parent_type = ParentType::Array;
    child.path = path + "[" + std::to_string(index) + "]";
    child.path_components.push_back(std::to_string(index));
    child.key_name = "";
    return child;
  }

  // Check if path matches a pattern
  bool path_matches(const std::string &pattern) const {
    // Simple glob matching
    if (pattern == "*")
      return true;
    if (pattern == path)
      return true;
    if (pattern.find('*') == std::string::npos)
      return false;

    // Convert glob to simple check
    size_t pos = 0;
    size_t pattern_pos = 0;

    while (pos < path.length() && pattern_pos < pattern.length()) {
      if (pattern[pattern_pos] == '*') {
        // Match any characters
        pattern_pos++;
        if (pattern_pos >= pattern.length())
          return true;

        // Find next match point
        char next_char = pattern[pattern_pos];
        while (pos < path.length() && path[pos] != next_char) {
          pos++;
        }
      } else if (pattern[pattern_pos] == path[pos]) {
        pos++;
        pattern_pos++;
      } else {
        return false;
      }
    }

    // Skip trailing wildcards
    while (pattern_pos < pattern.length() && pattern[pattern_pos] == '*') {
      pattern_pos++;
    }

    return pos == path.length() && pattern_pos == pattern.length();
  }

  // Check depth conditions
  bool depth_in_range(int min_depth, int max_depth = -1) const {
    if (depth < min_depth)
      return false;
    if (max_depth >= 0 && depth > max_depth)
      return false;
    return true;
  }

  // Check array position conditions
  bool is_first_array_item() const { return array_index == 0; }

  bool is_last_array_item() const {
    return array_index >= 0 && array_length > 0 && array_index == array_length - 1;
  }

  bool array_index_in_range(int min_idx, int max_idx = -1) const {
    if (array_index < 0)
      return false; // Not in an array
    if (array_index < min_idx)
      return false;
    if (max_idx >= 0 && array_index > max_idx)
      return false;
    return true;
  }

  // Check if array index matches pattern (even, odd, every N)
  bool array_index_matches(const std::string &pattern) const {
    if (array_index < 0)
      return false;

    if (pattern == "even")
      return array_index % 2 == 0;
    if (pattern == "odd")
      return array_index % 2 == 1;
    if (pattern == "first")
      return array_index == 0;
    if (pattern == "last")
      return is_last_array_item();

    // Pattern like "every:3" for every 3rd element
    if (pattern.find("every:") == 0) {
      int n = std::stoi(pattern.substr(6));
      if (n > 0)
        return array_index % n == 0;
    }

    return false;
  }
};

} // namespace colored_json
