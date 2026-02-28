#pragma once
#include "json_parser.hpp"
#include "style.hpp"
#include <iomanip>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <simdjson.h>
#include <sstream>

namespace py = pybind11;
using namespace simdjson;

namespace colored_json {

class HtmlPrinter {
private:
  const Style &style;
  std::stringstream output;
  int indent_level = 0;
  StyleContext current_context;

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

  std::string colorToCss(const Color &color) const {
    std::stringstream css;
    css << "rgb(" << static_cast<int>(color.r) << ","
        << static_cast<int>(color.g) << "," << static_cast<int>(color.b) << ")";
    return css.str();
  }

  std::string getStyleString(const Color &color) const {
    std::stringstream style_str;
    style_str << "color: " << colorToCss(color);
    if (color.bold)
      style_str << "; font-weight: bold";
    if (color.italic)
      style_str << "; font-style: italic";
    if (color.underline)
      style_str << "; text-decoration: underline";
    return style_str.str();
  }

  void addSpan(const Color &color, const std::string &content) {
    output << "<span style=\"" << getStyleString(color) << "\">";
    // Escape HTML special characters
    for (char c : content) {
      if (c == '<')
        output << "&lt;";
      else if (c == '>')
        output << "&gt;";
      else if (c == '&')
        output << "&amp;";
      else if (c == '"')
        output << "&quot;";
      else if (c == '\'')
        output << "&#39;";
      else
        output << c;
    }
    output << "</span>";
  }

  void printString(const std::string &s) {
    printStringWithColor(s, style.string_color, style.string_quote_color);
  }

  void printStringWithColor(const std::string &s, const Color &str_col,
                            const Color &quote_col) {
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

    // Print quotes apart van de string content
    addSpan(style.string_quote_color, "\"");
    addSpan(style.string_color, escaped.str());
    addSpan(style.string_quote_color, "\"");
  }

  void printKey(const std::string &s, const StyleContext &ctx = {}) {
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

    Color key_col = style.get_color(ElementType::Key, ctx, s);
    Color quote_col = style.get_quote_color(ElementType::Key);

    // Print quotes apart van de key content
    addSpan(quote_col, "\"");
    addSpan(key_col, escaped.str());
    addSpan(quote_col, "\"");
  }

  void printDict(py::dict obj, const StyleContext &parent_ctx = {}) {
    if (obj.empty()) {
      addSpan(style.brace_color, "{}");
      return;
    }

    addSpan(style.brace_color, "{");
    increaseIndent();

    bool first = true;
    for (auto item : obj) {
      if (!first) {
        addSpan(style.comma_color, ",");
      }
      addNewline();
      addIndent();

      std::string key_str = py::str(item.first);
      StyleContext child_ctx = parent_ctx.enter_object(key_str);

      printKey(key_str, child_ctx);

      addSpan(style.colon_color, ":");
      if (!style.compact)
        output << " ";

      printValue(item.second, child_ctx);
      first = false;
    }

    decreaseIndent();
    addNewline();
    addIndent();
    addSpan(style.brace_color, "}");
  }

  void printList(py::list obj, const StyleContext &parent_ctx = {}) {
    if (obj.empty()) {
      addSpan(style.bracket_color, "[]");
      return;
    }

    addSpan(style.bracket_color, "[");
    increaseIndent();

    bool first = true;
    int index = 0;
    for (auto item : obj) {
      if (!first) {
        addSpan(style.comma_color, ",");
      }
      addNewline();
      addIndent();

      StyleContext child_ctx = parent_ctx.enter_array(index, -1);
      printValue(item, child_ctx);
      first = false;
      index++;
    }

    decreaseIndent();
    addNewline();
    addIndent();
    addSpan(style.bracket_color, "]");
  }

  void printValue(py::handle obj, const StyleContext &ctx = {}) {
    current_context = ctx;

    if (py::isinstance<py::str>(obj)) {
      std::string str_val = py::str(obj);
      Color str_col = style.get_color(ElementType::String, ctx, str_val);
      Color quote_col = style.get_quote_color(ElementType::String);
      printStringWithColor(str_val, str_col, quote_col);
    } else if (py::isinstance<py::dict>(obj)) {
      printDict(py::reinterpret_borrow<py::dict>(obj), ctx);
    } else if (py::isinstance<py::list>(obj)) {
      printList(py::reinterpret_borrow<py::list>(obj), ctx);
    } else if (py::isinstance<py::int_>(obj) ||
               py::isinstance<py::float_>(obj)) {
      std::string num_str = py::str(obj);
      Color num_col = style.get_color(ElementType::Number, ctx, num_str);
      addSpan(num_col, num_str);
    } else if (py::isinstance<py::bool_>(obj)) {
      std::string bool_str = py::str(obj);
      Color bool_col = style.get_color(ElementType::Boolean, ctx, bool_str);
      addSpan(bool_col, bool_str);
    } else if (obj.is_none()) {
      Color null_col = style.get_color(ElementType::Null, ctx, "null");
      addSpan(null_col, "null");
    } else {
      printString(py::str(obj));
    }
  }

public:
  explicit HtmlPrinter(const Style &s) : style(s) {}

  std::string
  print(py::handle obj, const std::string &title = "Colored JSON",
        const std::string &background_color = "#1e1e1e",
        const std::string &font_family = "Consolas, 'Courier New', monospace") {
    output.str("");
    output.clear();
    indent_level = 0;
    current_context = StyleContext{};

    // HTML header
    output << "<!DOCTYPE html>\n";
    output << "<html>\n<head>\n";
    output << "  <meta charset=\"UTF-8\">\n";
    output << "  <meta name=\"viewport\" content=\"width=device-width, "
              "initial-scale=1.0\">\n";
    output << "  <title>" << title << "</title>\n";
    output << "  <style>\n";
    output << "    body {\n";
    output << "      background-color: " << background_color << ";\n";
    output << "      color: #ffffff;\n";
    output << "      font-family: " << font_family << ";\n";
    output << "      padding: 20px;\n";
    output << "      margin: 0;\n";
    output << "    }\n";
    output << "    pre {\n";
    output << "      margin: 0;\n";
    output << "      white-space: pre-wrap;\n";
    output << "      word-wrap: break-word;\n";
    output << "    }\n";
    output << "  </style>\n";
    output << "</head>\n<body>\n";
    output << "<pre>";

    if (py::isinstance<py::dict>(obj)) {
      printDict(py::reinterpret_borrow<py::dict>(obj));
    } else if (py::isinstance<py::list>(obj)) {
      printList(py::reinterpret_borrow<py::list>(obj));
    } else {
      printValue(obj);
    }

    output << "</pre>\n";
    output << "</body>\n</html>";

    return output.str();
  }

  // JSON string versies (werken met simdjson::ondemand::value)
  void printDictJson(ondemand::object obj,
                     const StyleContext &parent_ctx = {}) {
    addSpan(style.brace_color, "{");
    increaseIndent();

    bool first = true;
    bool empty = true;
    for (auto field : obj) {
      empty = false;
      if (!first) {
        addSpan(style.comma_color, ",");
      }
      addNewline();
      addIndent();

      std::string_view key_view = field.unescaped_key();
      std::string key_str(key_view);
      StyleContext child_ctx = parent_ctx.enter_object(key_str);

      printKey(key_str, child_ctx);

      addSpan(style.colon_color, ":");
      if (!style.compact)
        output << " ";

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
      addSpan(style.brace_color, "}");
      return;
    }

    decreaseIndent();
    addNewline();
    addIndent();
    addSpan(style.brace_color, "}");
  }

  void printListJson(ondemand::array arr, const StyleContext &parent_ctx = {}) {
    addSpan(style.bracket_color, "[");
    increaseIndent();

    bool first = true;
    bool empty = true;
    int index = 0;
    for (auto element : arr) {
      empty = false;
      if (!first) {
        addSpan(style.comma_color, ",");
      }
      addNewline();
      addIndent();

      StyleContext child_ctx = parent_ctx.enter_array(index, -1);
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
      addSpan(style.bracket_color, "]");
      return;
    }

    decreaseIndent();
    addNewline();
    addIndent();
    addSpan(style.bracket_color, "]");
  }

  void printValueJson(ondemand::value val, const StyleContext &ctx = {}) {
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
      std::string str_val(str_view);
      Color str_col = style.get_color(ElementType::String, ctx, str_val);
      Color quote_col = style.get_quote_color(ElementType::String);
      printStringWithColor(str_val, str_col, quote_col);
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
      std::string num_str(num_view);
      Color num_col = style.get_color(ElementType::Number, ctx, num_str);
      addSpan(num_col, num_str);
      break;
    }
    case ondemand::json_type::boolean: {
      bool b;
      auto bool_error = val.get_bool().get(b);
      if (bool_error) {
        throw std::runtime_error("Failed to get boolean");
      }
      std::string bool_str = b ? "true" : "false";
      Color bool_col = style.get_color(ElementType::Boolean, ctx, bool_str);
      addSpan(bool_col, bool_str);
      break;
    }
    case ondemand::json_type::null: {
      Color null_col = style.get_color(ElementType::Null, ctx, "null");
      addSpan(null_col, "null");
      break;
    }
    default:
      break;
    }
  }

  std::string printFromJson(
      const std::string &json_str, const std::string &title = "Colored JSON",
      const std::string &background_color = "#1e1e1e",
      const std::string &font_family = "Consolas, 'Courier New', monospace") {
    output.str("");
    output.clear();
    indent_level = 0;
    current_context = StyleContext{};

    // HTML header
    output << "<!DOCTYPE html>\n";
    output << "<html>\n<head>\n";
    output << "  <meta charset=\"UTF-8\">\n";
    output << "  <meta name=\"viewport\" content=\"width=device-width, "
              "initial-scale=1.0\">\n";
    output << "  <title>" << title << "</title>\n";
    output << "  <style>\n";
    output << "    body {\n";
    output << "      background-color: " << background_color << ";\n";
    output << "      color: #ffffff;\n";
    output << "      font-family: " << font_family << ";\n";
    output << "      padding: 20px;\n";
    output << "      margin: 0;\n";
    output << "    }\n";
    output << "    pre {\n";
    output << "      margin: 0;\n";
    output << "      white-space: pre-wrap;\n";
    output << "      word-wrap: break-word;\n";
    output << "    }\n";
    output << "  </style>\n";
    output << "</head>\n<body>\n";
    output << "<pre>";

    try {
      JsonParser parser;
      auto doc = parser.parse(json_str);
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

      if (type == ondemand::json_type::object) {
        ondemand::object obj;
        auto obj_error = val.get_object().get(obj);
        if (obj_error) {
          throw std::runtime_error("Failed to get object");
        }
        printDictJson(obj);
      } else if (type == ondemand::json_type::array) {
        ondemand::array arr;
        auto arr_error = val.get_array().get(arr);
        if (arr_error) {
          throw std::runtime_error("Failed to get array");
        }
        printListJson(arr);
      } else {
        printValueJson(val);
      }
    } catch (const simdjson_error &e) {
      throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    } catch (const std::exception &e) {
      throw std::runtime_error("JSON processing error: " +
                               std::string(e.what()));
    }

    output << "</pre>\n";
    output << "</body>\n</html>";

    return output.str();
  }
};

} // namespace colored_json
