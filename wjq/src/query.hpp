#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace colored_json {

// Simple JSON query parser and executor
// Supports basic jq-style filters like: .key, .key.nested, .array[], .array[0],
// etc.

class SimpleQueryEngine {
public:
  // Execute a simple query on JSON string
  // Returns filtered JSON string or empty string on error
  static std::string execute(const std::string &json, const std::string &query);

  // Get last error message
  static std::string lastError();

private:
  static std::string error_;

  // Parse and execute query
  static std::string applyQuery(const std::string &json,
                                const std::string &query);

  // Query operations
  static std::string getProperty(const std::string &json,
                                 const std::string &prop);
  static std::string getArrayElement(const std::string &json, int index);
  static std::string iterateArray(const std::string &json);
  static std::string getAllValues(const std::string &json);
};

// Implementation
inline std::string SimpleQueryEngine::error_;

inline std::string SimpleQueryEngine::lastError() { return error_; }

inline std::string SimpleQueryEngine::execute(const std::string &json,
                                              const std::string &query) {
  error_.clear();
  if (query.empty() || query == ".") {
    return json;
  }
  return applyQuery(json, query);
}

inline std::string SimpleQueryEngine::applyQuery(const std::string &json,
                                                 const std::string &query) {
  // Very simple query parser for common patterns
  size_t pos = 0;
  std::string result = json;

  while (pos < query.size()) {
    // Skip whitespace
    while (pos < query.size() && std::isspace(query[pos]))
      pos++;
    if (pos >= query.size())
      break;

    // Handle dot (property access or identity)
    if (query[pos] == '.') {
      pos++;

      // Check for recursive descent ..
      if (pos < query.size() && query[pos] == '.') {
        pos++;
        // For now, just get all values recursively (simplified)
        if (pos < query.size() && std::isalpha(query[pos])) {
          std::string prop;
          while (pos < query.size() &&
                 (std::isalnum(query[pos]) || query[pos] == '_')) {
            prop += query[pos++];
          }
          // This would need a real JSON parser to do properly
          error_ = "Recursive descent not fully implemented";
          return result;
        }
        continue;
      }

      // Iterator .[]
      if (pos < query.size() && query[pos] == '[') {
        pos++;
        if (pos < query.size() && query[pos] == ']') {
          pos++;
          result = iterateArray(result);
          continue;
        }
        // Array index
        std::string idx;
        while (pos < query.size() &&
               (std::isdigit(query[pos]) || query[pos] == '-')) {
          idx += query[pos++];
        }
        if (pos < query.size() && query[pos] == ']') {
          pos++;
          result = getArrayElement(result, std::stoi(idx));
          continue;
        }
      }

      // Wildcard iterator .*
      if (pos < query.size() && query[pos] == '*') {
        pos++;
        result = getAllValues(result);
        continue;
      }

      // Property name
      std::string prop;
      while (pos < query.size() &&
             (std::isalnum(query[pos]) || query[pos] == '_')) {
        prop += query[pos++];
      }
      if (!prop.empty()) {
        result = getProperty(result, prop);
      }
    }
    // Handle direct array access [n] or []
    else if (query[pos] == '[') {
      pos++;
      if (pos < query.size() && query[pos] == ']') {
        pos++;
        result = iterateArray(result);
      } else {
        std::string idx;
        while (pos < query.size() &&
               (std::isdigit(query[pos]) || query[pos] == '-')) {
          idx += query[pos++];
        }
        if (pos < query.size() && query[pos] == ']') {
          pos++;
          result = getArrayElement(result, std::stoi(idx));
        }
      }
    } else {
      pos++;
    }
  }

  return result;
}

inline std::string SimpleQueryEngine::getProperty(const std::string &json,
                                                  const std::string &prop) {
  // Find "prop": value pattern
  std::string search = "\"" + prop + "\"";
  size_t pos = json.find(search);
  if (pos == std::string::npos) {
    error_ = "Property not found: " + prop;
    return "null";
  }

  pos += search.size();
  // Skip whitespace and colon
  while (pos < json.size() && std::isspace(json[pos]))
    pos++;
  if (pos < json.size() && json[pos] == ':')
    pos++;
  while (pos < json.size() && std::isspace(json[pos]))
    pos++;

  // Extract value
  size_t start = pos;
  if (json[pos] == '{') {
    // Object - find matching }
    int depth = 1;
    pos++;
    while (pos < json.size() && depth > 0) {
      if (json[pos] == '{')
        depth++;
      else if (json[pos] == '}')
        depth--;
      else if (json[pos] == '"') {
        pos++;
        while (pos < json.size() && json[pos] != '"') {
          if (json[pos] == '\\')
            pos++;
          pos++;
        }
      }
      pos++;
    }
    return json.substr(start, pos - start);
  } else if (json[pos] == '[') {
    // Array - find matching ]
    int depth = 1;
    pos++;
    while (pos < json.size() && depth > 0) {
      if (json[pos] == '[')
        depth++;
      else if (json[pos] == ']')
        depth--;
      else if (json[pos] == '"') {
        pos++;
        while (pos < json.size() && json[pos] != '"') {
          if (json[pos] == '\\')
            pos++;
          pos++;
        }
      }
      pos++;
    }
    return json.substr(start, pos - start);
  } else if (json[pos] == '"') {
    // String
    pos++;
    while (pos < json.size() && json[pos] != '"') {
      if (json[pos] == '\\')
        pos++;
      pos++;
    }
    pos++;
    return json.substr(start, pos - start);
  } else {
    // Number, true, false, null
    while (pos < json.size() && !std::isspace(json[pos]) && json[pos] != ',' &&
           json[pos] != '}' && json[pos] != ']') {
      pos++;
    }
    return json.substr(start, pos - start);
  }
}

inline std::string SimpleQueryEngine::getArrayElement(const std::string &json,
                                                      int index) {
  if (json.empty() || json[0] != '[') {
    error_ = "Not an array";
    return "null";
  }

  size_t pos = 1; // Skip [
  int currentIdx = 0;

  while (pos < json.size()) {
    // Skip whitespace and comma
    while (pos < json.size() && (std::isspace(json[pos]) || json[pos] == ','))
      pos++;
    if (pos >= json.size() || json[pos] == ']')
      break;

    if (currentIdx == index) {
      // Found the element
      size_t start = pos;
      if (json[pos] == '{') {
        int depth = 1;
        pos++;
        while (pos < json.size() && depth > 0) {
          if (json[pos] == '{')
            depth++;
          else if (json[pos] == '}')
            depth--;
          else if (json[pos] == '"') {
            pos++;
            while (pos < json.size() && json[pos] != '"') {
              if (json[pos] == '\\')
                pos++;
              pos++;
            }
          }
          pos++;
        }
        return json.substr(start, pos - start);
      } else if (json[pos] == '[') {
        int depth = 1;
        pos++;
        while (pos < json.size() && depth > 0) {
          if (json[pos] == '[')
            depth++;
          else if (json[pos] == ']')
            depth--;
          else if (json[pos] == '"') {
            pos++;
            while (pos < json.size() && json[pos] != '"') {
              if (json[pos] == '\\')
                pos++;
              pos++;
            }
          }
          pos++;
        }
        return json.substr(start, pos - start);
      } else if (json[pos] == '"') {
        pos++;
        while (pos < json.size() && json[pos] != '"') {
          if (json[pos] == '\\')
            pos++;
          pos++;
        }
        pos++;
        return json.substr(start, pos - start);
      } else {
        while (pos < json.size() && !std::isspace(json[pos]) &&
               json[pos] != ',' && json[pos] != ']') {
          pos++;
        }
        return json.substr(start, pos - start);
      }
    }

    // Skip this element
    if (json[pos] == '{') {
      int depth = 1;
      pos++;
      while (pos < json.size() && depth > 0) {
        if (json[pos] == '{')
          depth++;
        else if (json[pos] == '}')
          depth--;
        else if (json[pos] == '"') {
          pos++;
          while (pos < json.size() && json[pos] != '"') {
            if (json[pos] == '\\')
              pos++;
            pos++;
          }
        }
        pos++;
      }
    } else if (json[pos] == '[') {
      int depth = 1;
      pos++;
      while (pos < json.size() && depth > 0) {
        if (json[pos] == '[')
          depth++;
        else if (json[pos] == ']')
          depth--;
        else if (json[pos] == '"') {
          pos++;
          while (pos < json.size() && json[pos] != '"') {
            if (json[pos] == '\\')
              pos++;
            pos++;
          }
        }
        pos++;
      }
    } else if (json[pos] == '"') {
      pos++;
      while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\')
          pos++;
        pos++;
      }
      pos++;
    } else {
      while (pos < json.size() && !std::isspace(json[pos]) &&
             json[pos] != ',' && json[pos] != ']') {
        pos++;
      }
    }

    currentIdx++;
  }

  error_ = "Index out of bounds: " + std::to_string(index);
  return "null";
}

inline std::string SimpleQueryEngine::iterateArray(const std::string &json) {
  if (json.empty() || json[0] != '[') {
    return json; // Not an array, return as-is
  }

  // Return the array for iteration (consumer will handle)
  return json;
}

inline std::string SimpleQueryEngine::getAllValues(const std::string &json) {
  // Simplified - return values of object
  if (json.empty() || json[0] != '{') {
    return json;
  }

  std::vector<std::string> values;
  size_t pos = 1;

  while (pos < json.size()) {
    while (pos < json.size() && (std::isspace(json[pos]) || json[pos] == ','))
      pos++;
    if (pos >= json.size() || json[pos] == '}')
      break;

    // Skip key
    if (json[pos] == '"') {
      pos++;
      while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\')
          pos++;
        pos++;
      }
      pos++;
    }

    // Skip colon
    while (pos < json.size() && std::isspace(json[pos]))
      pos++;
    if (pos < json.size() && json[pos] == ':')
      pos++;
    while (pos < json.size() && std::isspace(json[pos]))
      pos++;

    // Extract value
    size_t start = pos;
    if (json[pos] == '{') {
      int depth = 1;
      pos++;
      while (pos < json.size() && depth > 0) {
        if (json[pos] == '{')
          depth++;
        else if (json[pos] == '}')
          depth--;
        else if (json[pos] == '"') {
          pos++;
          while (pos < json.size() && json[pos] != '"') {
            if (json[pos] == '\\')
              pos++;
            pos++;
          }
        }
        pos++;
      }
      values.push_back(json.substr(start, pos - start));
    } else if (json[pos] == '[') {
      int depth = 1;
      pos++;
      while (pos < json.size() && depth > 0) {
        if (json[pos] == '[')
          depth++;
        else if (json[pos] == ']')
          depth--;
        else if (json[pos] == '"') {
          pos++;
          while (pos < json.size() && json[pos] != '"') {
            if (json[pos] == '\\')
              pos++;
            pos++;
          }
        }
        pos++;
      }
      values.push_back(json.substr(start, pos - start));
    } else if (json[pos] == '"') {
      pos++;
      while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\')
          pos++;
        pos++;
      }
      pos++;
      values.push_back(json.substr(start, pos - start));
    } else {
      while (pos < json.size() && !std::isspace(json[pos]) &&
             json[pos] != ',' && json[pos] != '}') {
        pos++;
      }
      values.push_back(json.substr(start, pos - start));
    }
  }

  // Return as JSON array
  std::string result = "[";
  for (size_t i = 0; i < values.size(); i++) {
    if (i > 0)
      result += ",";
    result += values[i];
  }
  result += "]";
  return result;
}

} // namespace colored_json
