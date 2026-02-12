#pragma once
#include "callbacks.hpp"
#include "json_parser.hpp"
#include "style.hpp"
#include "themes.hpp"
#include <iomanip>
#include <simdjson.h>
#include <sstream>

namespace colored_json {
using namespace simdjson;

class Printer {
private:
  const Style &style;
  std::stringstream output;
  int indent_level = 0;
  StyleContext current_context;
  const CallbackRegistry *callbacks_ = nullptr;

  static constexpr const char *RESET = "\033[0m";

  void increaseIndent() { indent_level++; }
  void decreaseIndent() { indent_level--; }

  void addIndent() {
    if (!style.compact) {
      output << std::string(indent_level * style.indent_size, ' ');
    }
  }

  void addNewline() {
    if (!style.compact) {
      output << "\n";
    }
  }

  template <typename T> void addColored(const Color &color, const T &value) {
    output << color.toAnsi(style.color_mode) << value << RESET;
  }

  void emit_event(JsonEventType type, const std::string &key = "",
                  const std::string &string_val = "", double num_val = 0,
                  bool bool_val = false) {
    if (!callbacks_)
      return;

    JsonEvent event;
    event.type = type;
    event.path = current_context.path;
    event.key = key;
    event.string_value = string_val;
    event.number_value = num_val;
    event.bool_value = bool_val;
    event.depth = current_context.depth;
    event.array_index = current_context.array_index;
    event.context = current_context;

    callbacks_->invoke_element_callbacks(event);
  }

  std::string escape_string(const std::string &s) {
    std::stringstream escaped;

    for (char c : s) {
      if (c == '"')
        escaped << "\\\"";
      else if (c == '\\')
        escaped << "\\\\";
      else if (c == '\b')
        escaped << "\\b";
      else if (c == '\f')
        escaped << "\\f";
      else if (c == '\n')
        escaped << "\\n";
      else if (c == '\r')
        escaped << "\\r";
      else if (c == '\t')
        escaped << "\\t";
      else if (static_cast<unsigned char>(c) < 0x20) {
        escaped << "\\u00" << std::hex << std::setfill('0') << std::setw(2)
                << static_cast<int>(static_cast<unsigned char>(c)) << std::dec;
      } else
        escaped << c;
    }

    return escaped.str();
  }

  void printString(const std::string &s, const StyleContext &ctx) {
    Color str_color = style.get_color(ElementType::String, ctx, s);
    Color quote_color = style.get_quote_color(ElementType::String);

    // Apply callbacks if any
    std::string display_value = s;
    if (callbacks_) {
      JsonEvent event;
      event.type = JsonEventType::StringValue;
      event.path = ctx.path;
      event.string_value = s;
      event.depth = ctx.depth;
      event.context = ctx;

      auto result = callbacks_->invoke_element_callbacks(event);
      if (result.skip_element)
        return;
      if (result.replace_value) {
        display_value = result.replacement_text;
      }
      if (result.custom_color) {
        str_color = result.color;
      }

      // Apply value transforms
      display_value = callbacks_->invoke_value_transforms(display_value, ctx);
    }

    std::string escaped = escape_string(display_value);

    addColored(quote_color, "\"");
    addColored(str_color, escaped);
    addColored(quote_color, "\"");
  }

  void printKey(const std::string &s, const StyleContext &ctx) {
    Color key_col = style.get_color(ElementType::Key, ctx, s);
    Color quote_col = style.get_quote_color(ElementType::Key);

    // Apply callbacks
    if (callbacks_) {
      JsonEvent event;
      event.type = JsonEventType::Key;
      event.path = ctx.path;
      event.key = s;
      event.depth = ctx.depth;
      event.context = ctx;

      auto result = callbacks_->invoke_element_callbacks(event);
      if (result.skip_element)
        return;
      if (result.custom_color) {
        key_col = result.color;
      }
    }

    std::string escaped = escape_string(s);

    addColored(quote_col, "\"");
    addColored(key_col, escaped);
    addColored(quote_col, "\"");
  }

  void printNumber(const std::string &num_str, const StyleContext &ctx) {
    Color num_color = style.get_color(ElementType::Number, ctx, num_str);

    // Apply callbacks
    std::string display_value = num_str;
    if (callbacks_) {
      JsonEvent event;
      event.type = JsonEventType::NumberValue;
      event.path = ctx.path;
      try {
        event.number_value = std::stod(num_str);
      } catch (...) {
        event.number_value = 0;
      }
      event.depth = ctx.depth;
      event.context = ctx;

      auto result = callbacks_->invoke_element_callbacks(event);
      if (result.skip_element)
        return;
      if (result.custom_color) {
        num_color = result.color;
      }

      display_value = callbacks_->invoke_value_transforms(display_value, ctx);
    }

    addColored(num_color, display_value);
  }

  void printBoolean(bool b, const StyleContext &ctx) {
    Color bool_col = style.get_color(ElementType::Boolean, ctx, b ? "true" : "false");
    std::string bool_str = b ? "true" : "false";

    // Apply callbacks
    if (callbacks_) {
      JsonEvent event;
      event.type = JsonEventType::BooleanValue;
      event.path = ctx.path;
      event.bool_value = b;
      event.depth = ctx.depth;
      event.context = ctx;

      auto result = callbacks_->invoke_element_callbacks(event);
      if (result.skip_element)
        return;
      if (result.custom_color) {
        bool_col = result.color;
      }
    }

    addColored(bool_col, bool_str);
  }

  void printNull(const StyleContext &ctx) {
    Color null_col = style.get_color(ElementType::Null, ctx, "null");

    // Apply callbacks
    if (callbacks_) {
      JsonEvent event;
      event.type = JsonEventType::NullValue;
      event.path = ctx.path;
      event.depth = ctx.depth;
      event.context = ctx;

      auto result = callbacks_->invoke_element_callbacks(event);
      if (result.skip_element)
        return;
      if (result.custom_color) {
        null_col = result.color;
      }
    }

    addColored(null_col, "null");
  }

public:
  explicit Printer(const Style &s, const CallbackRegistry *callbacks = nullptr)
      : style(s), callbacks_(callbacks) {}

  std::string str() const { return output.str(); }

  void clear() {
    output.str("");
    output.clear();
    indent_level = 0;
    current_context = StyleContext{};
  }

  void setCallbacks(const CallbackRegistry *callbacks) { callbacks_ = callbacks; }

  void printDictJson(ondemand::object obj, const StyleContext &parent_ctx = {}) {
    current_context = parent_ctx;
    emit_event(JsonEventType::BeginObject);

    Color brace_col = style.get_color(ElementType::Brace, current_context);
    addColored(brace_col, "{");
    increaseIndent();

    bool first = true;
    bool empty = true;

    for (auto field : obj) {
      empty = false;

      // Get key
      std::string_view key_view = field.unescaped_key();
      std::string key_str(key_view);

      // Create child context
      StyleContext child_ctx = parent_ctx.enter_object(key_str);

      if (!first) {
        Color comma_col = style.get_color(ElementType::Comma, child_ctx);
        addColored(comma_col, ",");
      }
      addNewline();
      addIndent();

      // Print key
      printKey(key_str, child_ctx);

      // Colon
      Color colon_col = style.get_color(ElementType::Colon, child_ctx);
      addColored(colon_col, ":");
      if (!style.compact)
        output << " ";

      // Get and print value
      ondemand::value val;
      auto val_error = field.value().get(val);
      if (val_error) {
        throw std::runtime_error("Failed to get field value");
      }

      printValueJson(val, child_ctx);
      first = false;
    }

    if (empty) {
      decreaseIndent();
      addColored(brace_col, "}");
      emit_event(JsonEventType::EndObject);
      return;
    }

    decreaseIndent();
    addNewline();
    addIndent();
    addColored(brace_col, "}");
    emit_event(JsonEventType::EndObject);
  }

  void printListJson(ondemand::array arr, const StyleContext &parent_ctx = {}) {
    current_context = parent_ctx;
    emit_event(JsonEventType::BeginArray);

    Color bracket_col = style.get_color(ElementType::Bracket, current_context);
    addColored(bracket_col, "[");
    increaseIndent();

    bool first = true;
    bool empty = true;
    int index = 0;

    // Count array elements for context
    // Note: simdjson ondemand doesn't give us length easily, so we estimate
    int estimated_length = -1;

    for (auto element : arr) {
      empty = false;

      // Create child context with array index
      StyleContext child_ctx = parent_ctx.enter_array(index, estimated_length);

      if (!first) {
        Color comma_col = style.get_color(ElementType::Comma, child_ctx);
        addColored(comma_col, ",");
      }
      addNewline();
      addIndent();

      ondemand::value val;
      auto val_error = element.get(val);
      if (val_error) {
        throw std::runtime_error("Failed to get array element");
      }

      printValueJson(val, child_ctx);
      first = false;
      index++;
    }

    if (empty) {
      decreaseIndent();
      addColored(bracket_col, "]");
      emit_event(JsonEventType::EndArray);
      return;
    }

    decreaseIndent();
    addNewline();
    addIndent();
    addColored(bracket_col, "]");
    emit_event(JsonEventType::EndArray);
  }

  void printValueJson(ondemand::value val, const StyleContext &ctx) {
    current_context = ctx;

    ondemand::json_type type;
    auto type_error = val.type().get(type);
    if (type_error) {
      throw std::runtime_error("Failed to get JSON type");
    }

    switch (type) {
    case ondemand::json_type::string: {
      std::string_view str_view;
      auto str_error = val.get_string().get(str_view);
      if (str_error) {
        throw std::runtime_error("Failed to get string value");
      }
      printString(std::string(str_view), ctx);
      break;
    }
    case ondemand::json_type::object: {
      ondemand::object obj;
      auto obj_error = val.get_object().get(obj);
      if (obj_error) {
        throw std::runtime_error("Failed to get object");
      }
      printDictJson(obj, ctx);
      break;
    }
    case ondemand::json_type::array: {
      ondemand::array arr;
      auto arr_error = val.get_array().get(arr);
      if (arr_error) {
        throw std::runtime_error("Failed to get array");
      }
      printListJson(arr, ctx);
      break;
    }
    case ondemand::json_type::number: {
      std::string_view num_view = val.raw_json_token();
      printNumber(std::string(num_view), ctx);
      break;
    }
    case ondemand::json_type::boolean: {
      bool b;
      auto bool_error = val.get_bool().get(b);
      if (bool_error) {
        throw std::runtime_error("Failed to get boolean");
      }
      printBoolean(b, ctx);
      break;
    }
    case ondemand::json_type::null: {
      printNull(ctx);
      break;
    }
    default:
      throw std::runtime_error("Unknown or unsupported JSON type");
    }
  }

  std::string printFromJson(const std::string &json_str) {
    clear();

    try {
      JsonParser parser;
      simdjson::padded_string padded_json(json_str);
      auto doc = parser.parse(padded_json);

      ondemand::value val;
      auto val_error = doc.get_value().get(val);
      if (val_error) {
        throw std::runtime_error("Failed to get document value");
      }

      ondemand::json_type type;
      auto error = val.type().get(type);
      if (error) {
        throw std::runtime_error("Failed to get JSON type");
      }

      StyleContext root_ctx;
      root_ctx.path = "";
      root_ctx.depth = 0;
      root_ctx.parent_type = StyleContext::ParentType::Root;

      emit_event(JsonEventType::BeginDocument);

      if (type == ondemand::json_type::object) {
        ondemand::object obj;
        auto obj_error = val.get_object().get(obj);
        if (obj_error) {
          throw std::runtime_error("Failed to get object");
        }
        printDictJson(obj, root_ctx);
      } else if (type == ondemand::json_type::array) {
        ondemand::array arr;
        auto arr_error = val.get_array().get(arr);
        if (arr_error) {
          throw std::runtime_error("Failed to get array");
        }
        printListJson(arr, root_ctx);
      } else {
        printValueJson(val, root_ctx);
      }

      emit_event(JsonEventType::EndDocument);

    } catch (const simdjson_error &e) {
      throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    } catch (const std::exception &e) {
      throw std::runtime_error("JSON processing error: " + std::string(e.what()));
    }

    return output.str();
  }

  // Print from simdjson document directly
  void printDocument(ondemand::document &doc, const StyleContext &ctx = {}) {
    current_context = ctx;

    ondemand::value val;
    auto val_error = doc.get_value().get(val);
    if (val_error) {
      throw std::runtime_error("Failed to get document value");
    }

    ondemand::json_type type;
    auto error = val.type().get(type);
    if (error) {
      throw std::runtime_error("Failed to get JSON type");
    }

    emit_event(JsonEventType::BeginDocument);

    if (type == ondemand::json_type::object) {
      ondemand::object obj;
      auto obj_error = val.get_object().get(obj);
      if (obj_error) {
        throw std::runtime_error("Failed to get object");
      }
      printDictJson(obj, ctx);
    } else if (type == ondemand::json_type::array) {
      ondemand::array arr;
      auto arr_error = val.get_array().get(arr);
      if (arr_error) {
        throw std::runtime_error("Failed to get array");
      }
      printListJson(arr, ctx);
    } else {
      printValueJson(val, ctx);
    }

    emit_event(JsonEventType::EndDocument);
  }
};

} // namespace colored_json
