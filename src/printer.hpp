#pragma once
#include "style.hpp"
#include "json_parser.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <simdjson.h>
#include <sstream>
#include <iomanip>

namespace py = pybind11;
using namespace simdjson;

namespace colored_json {

class Printer {
private:
    const Style& style;
    std::stringstream output;
    int indent_level = 0;
    std::string current_path;  // Voor tracking van geneste keys
    static constexpr const char* RESET = "\033[0m";
    
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
    
    template<typename T>
    void addColored(const Color& color, const T& value) {
        output << color.toAnsi(style.color_mode) << value << RESET;
    }
    
    void printString(const std::string& s) {
        std::stringstream escaped;
        
        for (char c : s) {
            if (c == '"') escaped << "\\\"";
            else if (c == '\\') escaped << "\\\\";
            else if (c == '\b') escaped << "\\b";
            else if (c == '\f') escaped << "\\f";
            else if (c == '\n') escaped << "\\n";
            else if (c == '\r') escaped << "\\r";
            else if (c == '\t') escaped << "\\t";
            else if (static_cast<unsigned char>(c) < 0x20) {
                escaped << "\\u00" << std::hex << std::setfill('0') << std::setw(2) 
                        << static_cast<int>(static_cast<unsigned char>(c)) << std::dec;
            }
            else escaped << c;
        }
        
        // Print quotes apart van de string content
        addColored(style.string_quote_color, "\"");
        addColored(style.string_color, escaped.str());
        addColored(style.string_quote_color, "\"");
    }
    
    void printKey(const std::string& s, const std::string& path = "") {
        std::stringstream escaped;
        
        for (char c : s) {
            if (c == '"') escaped << "\\\"";
            else if (c == '\\') escaped << "\\\\";
            else if (c == '\b') escaped << "\\b";
            else if (c == '\f') escaped << "\\f";
            else if (c == '\n') escaped << "\\n";
            else if (c == '\r') escaped << "\\r";
            else if (c == '\t') escaped << "\\t";
            else if (static_cast<unsigned char>(c) < 0x20) {
                escaped << "\\u00" << std::hex << std::setfill('0') << std::setw(2) 
                        << static_cast<int>(static_cast<unsigned char>(c)) << std::dec;
            }
            else escaped << c;
        }
        
        // Check voor individuele key kleur
        Color key_col = style.key_color;
        Color quote_col = style.key_quote_color;
        if (!style.key_colors.empty()) {
            // Check exacte key match
            auto it = style.key_colors.find(s);
            if (it != style.key_colors.end()) {
                key_col = it->second;
            } else if (!path.empty()) {
                // Check path-based match (bijv. "user.name")
                auto path_it = style.key_colors.find(path);
                if (path_it != style.key_colors.end()) {
                    key_col = path_it->second;
                }
            }
        }
        
        // Print quotes apart van de key content
        addColored(quote_col, "\"");
        addColored(key_col, escaped.str());
        addColored(quote_col, "\"");
    }
    
    void printDict(py::dict obj, const std::string& parent_path = "") {
        if (obj.empty()) {
            addColored(style.brace_color, "{}");
            return;
        }
        
        addColored(style.brace_color, "{");
        increaseIndent();
        
        bool first = true;
        for (auto item : obj) {
            if (!first) {
                addColored(style.comma_color, ",");
            }
            addNewline();
            addIndent();
            
            // Key (altijd string in Python dicts)
            std::string key_str = py::str(item.first);
            std::string current_key_path = parent_path.empty() ? key_str : parent_path + "." + key_str;
            
            printKey(key_str, current_key_path);
            
            addColored(style.colon_color, ":");
            if (!style.compact) output << " ";
            
            // Value
            printValue(item.second, current_key_path);
            first = false;
        }
        
        decreaseIndent();
        addNewline();
        addIndent();
        addColored(style.brace_color, "}");
    }
    
    void printList(py::list obj, const std::string& parent_path = "") {
        if (obj.empty()) {
            addColored(style.bracket_color, "[]");
            return;
        }
        
        addColored(style.bracket_color, "[");
        increaseIndent();
        
        bool first = true;
        int index = 0;
        for (auto item : obj) {
            if (!first) {
                addColored(style.comma_color, ",");
            }
            addNewline();
            addIndent();
            
            std::string item_path = parent_path.empty() ? "" : parent_path + "[" + std::to_string(index) + "]";
            printValue(item, item_path);
            first = false;
            index++;
        }
        
        decreaseIndent();
        addNewline();
        addIndent();
        addColored(style.bracket_color, "]");
    }
    
    void printValue(py::handle obj, const std::string& path = "") {
        // Check voor individuele value kleur op basis van path
        Color value_color;
        bool has_custom_color = false;
        
        if (!style.value_colors.empty() && !path.empty()) {
            auto it = style.value_colors.find(path);
            if (it != style.value_colors.end()) {
                value_color = it->second;
                has_custom_color = true;
            }
        }
        
        if (py::isinstance<py::str>(obj)) {
            if (has_custom_color) {
                std::string str_val = py::str(obj);
                std::stringstream escaped;
                for (char c : str_val) {
                    if (c == '"') escaped << "\\\"";
                    else if (c == '\\') escaped << "\\\\";
                    else if (c == '\b') escaped << "\\b";
                    else if (c == '\f') escaped << "\\f";
                    else if (c == '\n') escaped << "\\n";
                    else if (c == '\r') escaped << "\\r";
                    else if (c == '\t') escaped << "\\t";
                    else if (static_cast<unsigned char>(c) < 0x20) {
                        escaped << "\\u00" << std::hex << std::setfill('0') << std::setw(2) 
                                << static_cast<int>(static_cast<unsigned char>(c)) << std::dec;
                    }
                    else escaped << c;
                }
                // Print quotes apart voor custom colors
                addColored(style.string_quote_color, "\"");
                addColored(value_color, escaped.str());
                addColored(style.string_quote_color, "\"");
            } else {
                printString(py::str(obj));
            }
        } else if (py::isinstance<py::dict>(obj)) {
            printDict(py::reinterpret_borrow<py::dict>(obj), path);
        } else if (py::isinstance<py::list>(obj)) {
            printList(py::reinterpret_borrow<py::list>(obj), path);
        } else if (py::isinstance<py::int_>(obj) || py::isinstance<py::float_>(obj)) {
            if (has_custom_color) {
                addColored(value_color, py::str(obj));
            } else {
                addColored(style.number_color, py::str(obj));
            }
        } else if (py::isinstance<py::bool_>(obj)) {
            if (has_custom_color) {
                addColored(value_color, py::str(obj));
            } else {
                addColored(style.bool_color, py::str(obj));
            }
        } else if (obj.is_none()) {
            if (has_custom_color) {
                addColored(value_color, "null");
            } else {
                addColored(style.null_color, "null");
            }
        } else {
            // Fallback voor andere types
            printString(py::str(obj));
        }
    }
    
public:
    explicit Printer(const Style& s) : style(s) {}
    
    std::string print(py::handle obj) {
        output.str("");
        output.clear();
        indent_level = 0;
        current_path = "";
        
        if (py::isinstance<py::dict>(obj)) {
            printDict(py::reinterpret_borrow<py::dict>(obj), "");
        } else if (py::isinstance<py::list>(obj)) {
            printList(py::reinterpret_borrow<py::list>(obj), "");
        } else {
            printValue(obj, "");
        }
        
        return output.str();
    }
    
    // JSON string versies (werken met simdjson::ondemand::value)
    void printDictJson(ondemand::object obj, const std::string& parent_path = "") {
        addColored(style.brace_color, "{");
        increaseIndent();
        
        bool first = true;
        bool empty = true;
        for (auto field : obj) {
            empty = false;
            if (!first) {
                addColored(style.comma_color, ",");
            }
            addNewline();
            addIndent();
            
            // Key (altijd string in JSON objects)
            std::string_view key_view = field.unescaped_key();
            std::string key_str(key_view);
            std::string current_key_path = parent_path.empty() ? key_str : parent_path + "." + key_str;
            
            printKey(key_str, current_key_path);
            
            addColored(style.colon_color, ":");
            if (!style.compact) output << " ";
            
            // Value
            ondemand::value val;
            auto val_error = field.value().get(val);
            if (val_error) {
                throw std::runtime_error("Failed to get field value");
            }
            printValueJson(val, current_key_path);
            first = false;
        }
        
        if (empty) {
            decreaseIndent();
            addColored(style.brace_color, "}");
            return;
        }
        
        decreaseIndent();
        addNewline();
        addIndent();
        addColored(style.brace_color, "}");
    }
    
    void printListJson(ondemand::array arr, const std::string& parent_path = "") {
        addColored(style.bracket_color, "[");
        increaseIndent();
        
        bool first = true;
        bool empty = true;
        int index = 0;
        for (auto element : arr) {
            empty = false;
            if (!first) {
                addColored(style.comma_color, ",");
            }
            addNewline();
            addIndent();
            
            std::string item_path = parent_path.empty() ? "" : parent_path + "[" + std::to_string(index) + "]";
            ondemand::value val;
            auto val_error = element.get(val);
            if (val_error) {
                throw std::runtime_error("Failed to get array element");
            }
            printValueJson(val, item_path);
            first = false;
            index++;
        }
        
        if (empty) {
            decreaseIndent();
            addColored(style.bracket_color, "]");
            return;
        }
        
        decreaseIndent();
        addNewline();
        addIndent();
        addColored(style.bracket_color, "]");
    }
    
    void printValueJson(ondemand::value val, const std::string& path = "") {
        // Check voor individuele value kleur op basis van path
        Color value_color;
        bool has_custom_color = false;
        
        if (!style.value_colors.empty() && !path.empty()) {
            auto it = style.value_colors.find(path);
            if (it != style.value_colors.end()) {
                value_color = it->second;
                has_custom_color = true;
            }
        }
        
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
                if (has_custom_color) {
                    std::stringstream escaped;
                    for (char c : str_val) {
                        if (c == '"') escaped << "\\\"";
                        else if (c == '\\') escaped << "\\\\";
                        else if (c == '\b') escaped << "\\b";
                        else if (c == '\f') escaped << "\\f";
                        else if (c == '\n') escaped << "\\n";
                        else if (c == '\r') escaped << "\\r";
                        else if (c == '\t') escaped << "\\t";
                        else if (static_cast<unsigned char>(c) < 0x20) {
                            escaped << "\\u00" << std::hex << std::setfill('0') << std::setw(2) 
                                    << static_cast<int>(static_cast<unsigned char>(c)) << std::dec;
                        }
                        else escaped << c;
                    }
                    addColored(style.string_quote_color, "\"");
                    addColored(value_color, escaped.str());
                    addColored(style.string_quote_color, "\"");
                } else {
                    printString(str_val);
                }
                break;
            }
            case ondemand::json_type::object: {
                ondemand::object obj;
                auto obj_error = val.get_object().get(obj);
                if (obj_error) {
                    throw std::runtime_error("Failed to get object");
                }
                printDictJson(obj, path);
                break;
            }
            case ondemand::json_type::array: {
                ondemand::array arr;
                auto arr_error = val.get_array().get(arr);
                if (arr_error) {
                    throw std::runtime_error("Failed to get array");
                }
                printListJson(arr, path);
                break;
            }
            case ondemand::json_type::number: {
                std::string_view num_view = val.raw_json_token();
                std::string num_str(num_view);
                if (has_custom_color) {
                    addColored(value_color, num_str);
                } else {
                    addColored(style.number_color, num_str);
                }
                break;
            }
            case ondemand::json_type::boolean: {
                bool b;
                auto bool_error = val.get_bool().get(b);
                if (bool_error) {
                    throw std::runtime_error("Failed to get boolean");
                }
                std::string bool_str = b ? "true" : "false";
                if (has_custom_color) {
                    addColored(value_color, bool_str);
                } else {
                    addColored(style.bool_color, bool_str);
                }
                break;
            }
            case ondemand::json_type::null: {
                if (has_custom_color) {
                    addColored(value_color, "null");
                } else {
                    addColored(style.null_color, "null");
                }
                break;
            }
        }
    }
    
    std::string printFromJson(const std::string& json_str) {
        output.str("");
        output.clear();
        indent_level = 0;
        current_path = "";
        
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
                printDictJson(obj, "");
            } else if (type == ondemand::json_type::array) {
                ondemand::array arr;
                auto arr_error = val.get_array().get(arr);
                if (arr_error) {
                    throw std::runtime_error("Failed to get array");
                }
                printListJson(arr, "");
            } else {
                printValueJson(val, "");
            }
        } catch (const simdjson_error& e) {
            throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
        } catch (const std::exception& e) {
            throw std::runtime_error("JSON processing error: " + std::string(e.what()));
        }
        
        return output.str();
    }
};

} // namespace colored_json

