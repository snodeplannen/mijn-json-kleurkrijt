#pragma once
#include "style.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <sstream>
#include <iomanip>

namespace py = pybind11;

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
        
        addColored(style.string_color, "\"" + escaped.str() + "\"");
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
        
        addColored(key_col, "\"" + escaped.str() + "\"");
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
                addColored(value_color, "\"" + escaped.str() + "\"");
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
};

} // namespace colored_json

