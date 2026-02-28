#include "query_engine.hpp"
#include <iomanip>
#include <sstream>

namespace colored_json {

// ============================================================================
// QueryValue Implementation
// ============================================================================

QueryValue QueryValue::fromSimdjson(simdjson::ondemand::value val) {
  simdjson::ondemand::json_type type;
  auto error = val.type().get(type);
  if (error) {
    return QueryValue::null();
  }

  switch (type) {
  case simdjson::ondemand::json_type::null:
    return QueryValue::null();

  case simdjson::ondemand::json_type::boolean: {
    bool b = false;
    if (val.get_bool().get(b) == simdjson::SUCCESS) {
      return QueryValue(b);
    }
    return QueryValue::null();
  }

  case simdjson::ondemand::json_type::number: {
    double d = 0;
    if (val.get_double().get(d) == simdjson::SUCCESS) {
      return QueryValue(d);
    }
    return QueryValue::null();
  }

  case simdjson::ondemand::json_type::string: {
    std::string_view sv;
    if (val.get_string().get(sv) == simdjson::SUCCESS) {
      return QueryValue(std::string(sv));
    }
    return QueryValue::null();
  }

  case simdjson::ondemand::json_type::array: {
    ArrayType arr;
    for (auto elem : val.get_array()) {
      arr.push_back(fromSimdjson(elem.value()));
    }
    return QueryValue::array(std::move(arr));
  }

  case simdjson::ondemand::json_type::object: {
    ObjectType obj;
    for (auto field : val.get_object()) {
      std::string_view key = field.unescaped_key();
      obj.emplace_back(std::string(key), fromSimdjson(field.value()));
    }
    return QueryValue::object(std::move(obj));
  }

  default:
    return QueryValue::null();
  }
}

QueryValue QueryValue::fromSimdjson(simdjson::ondemand::document &doc) {
  simdjson::ondemand::value val;
  auto error = doc.get_value().get(val);
  if (error) {
    return QueryValue::null();
  }
  return fromSimdjson(val);
}

std::string QueryValue::toJson() const { return toJson(0, 0); }

std::string QueryValue::toJson(int indent, int currentIndent) const {
  std::ostringstream oss;

  auto indentStr = [indent, currentIndent]() -> std::string {
    if (indent <= 0)
      return "";
    return std::string(currentIndent, ' ');
  };

  auto newline = [indent]() -> std::string { return indent > 0 ? "\n" : ""; };

  std::visit(
      [&](const auto &val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, NullValue>) {
          oss << "null";
        } else if constexpr (std::is_same_v<T, bool>) {
          oss << (val ? "true" : "false");
        } else if constexpr (std::is_same_v<T, double>) {
          if (val == static_cast<int64_t>(val)) {
            oss << static_cast<int64_t>(val);
          } else {
            oss << std::setprecision(15) << val;
          }
        } else if constexpr (std::is_same_v<T, std::string>) {
          oss << "\"" << escapeJsonString(val) << "\"";
        } else if constexpr (std::is_same_v<T, ArrayType>) {
          oss << "[" << newline();
          for (size_t i = 0; i < val.size(); ++i) {
            if (i > 0) {
              oss << "," << newline();
            }
            oss << indentStr() << std::string(indent, ' ')
                << val[i].toJson(indent, currentIndent + indent);
          }
          oss << newline() << indentStr() << "]";
        } else if constexpr (std::is_same_v<T, ObjectType>) {
          oss << "{" << newline();
          for (size_t i = 0; i < val.size(); ++i) {
            if (i > 0) {
              oss << "," << newline();
            }
            oss << indentStr() << std::string(indent, ' ') << "\""
                << escapeJsonString(val[i].first) << "\": "
                << val[i].second.toJson(indent, currentIndent + indent);
          }
          oss << newline() << indentStr() << "}";
        }
      },
      data_);

  return oss.str();
}

bool QueryValue::operator==(const QueryValue &other) const {
  if (type() != other.type())
    return false;

  return std::visit(
      [&](const auto &val) -> bool {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, NullValue>) {
          return true;
        } else if constexpr (std::is_same_v<T, bool> ||
                             std::is_same_v<T, double>) {
          return val == std::get<T>(other.data_);
        } else if constexpr (std::is_same_v<T, std::string>) {
          return val == std::get<std::string>(other.data_);
        } else if constexpr (std::is_same_v<T, ArrayType>) {
          const auto &other_arr = std::get<ArrayType>(other.data_);
          if (val.size() != other_arr.size())
            return false;
          for (size_t i = 0; i < val.size(); ++i) {
            if (val[i] != other_arr[i])
              return false;
          }
          return true;
        } else if constexpr (std::is_same_v<T, ObjectType>) {
          // Simple comparison - not order-sensitive for values
          const auto &other_obj = std::get<ObjectType>(other.data_);
          if (val.size() != other_obj.size())
            return false;
          for (const auto &[k, v] : val) {
            bool found = false;
            for (const auto &[ok, ov] : other_obj) {
              if (k == ok) {
                if (v != ov)
                  return false;
                found = true;
                break;
              }
            }
            if (!found)
              return false;
          }
          return true;
        }
      },
      data_);
}

bool QueryValue::operator<(const QueryValue &other) const {
  if (type() != other.type()) {
    // Compare types for consistent ordering
    return static_cast<int>(type()) < static_cast<int>(other.type());
  }

  return std::visit(
      [&](const auto &val) -> bool {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, NullValue>) {
          return false;
        } else if constexpr (std::is_same_v<T, bool>) {
          return val < std::get<bool>(other.data_);
        } else if constexpr (std::is_same_v<T, double>) {
          return val < std::get<double>(other.data_);
        } else if constexpr (std::is_same_v<T, std::string>) {
          return val < std::get<std::string>(other.data_);
        } else if constexpr (std::is_same_v<T, ArrayType>) {
          const auto &other_arr = std::get<ArrayType>(other.data_);
          return val.size() < other_arr.size();
        } else if constexpr (std::is_same_v<T, ObjectType>) {
          const auto &other_obj = std::get<ObjectType>(other.data_);
          return val.size() < other_obj.size();
        }
      },
      data_);
}

std::string escapeJsonString(const std::string &s) {
  std::ostringstream oss;
  for (char c : s) {
    switch (c) {
    case '"':
      oss << "\\\"";
      break;
    case '\\':
      oss << "\\\\";
      break;
    case '\b':
      oss << "\\b";
      break;
    case '\f':
      oss << "\\f";
      break;
    case '\n':
      oss << "\\n";
      break;
    case '\r':
      oss << "\\r";
      break;
    case '\t':
      oss << "\\t";
      break;
    default:
      if (static_cast<unsigned char>(c) < 0x20) {
        oss << "\\u00" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(static_cast<unsigned char>(c)) << std::dec;
      } else {
        oss << c;
      }
    }
  }
  return oss.str();
}

} // namespace colored_json
