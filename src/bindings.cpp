#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "printer.hpp"
#include "html_printer.hpp"
#include "style.hpp"

namespace py = pybind11;

PYBIND11_MODULE(colored_json, m) {
    m.doc() = "Razendsnelle gekleurde JSON/dict printer voor Python";
    
    py::enum_<colored_json::ColorMode>(m, "ColorMode")
        .value("AUTO", colored_json::ColorMode::Auto)
        .value("ANSI16", colored_json::ColorMode::Ansi16)
        .value("ANSI256", colored_json::ColorMode::Ansi256)
        .value("TRUECOLOR", colored_json::ColorMode::TrueColor)
        .value("DISABLED", colored_json::ColorMode::Disabled);
    
    py::enum_<colored_json::ElementType>(m, "ElementType")
        .value("KEY", colored_json::ElementType::Key)
        .value("STRING", colored_json::ElementType::String)
        .value("NUMBER", colored_json::ElementType::Number)
        .value("BOOLEAN", colored_json::ElementType::Boolean)
        .value("NULL", colored_json::ElementType::Null)
        .value("BRACE", colored_json::ElementType::Brace)
        .value("BRACKET", colored_json::ElementType::Bracket)
        .value("COLON", colored_json::ElementType::Colon)
        .value("COMMA", colored_json::ElementType::Comma)
        .value("WHITESPACE", colored_json::ElementType::Whitespace);
    
    py::class_<colored_json::Color>(m, "Color")
        .def(py::init<uint8_t, uint8_t, uint8_t>())
        .def_readwrite("r", &colored_json::Color::r)
        .def_readwrite("g", &colored_json::Color::g)
        .def_readwrite("b", &colored_json::Color::b)
        .def_readwrite("bold", &colored_json::Color::bold)
        .def_readwrite("italic", &colored_json::Color::italic)
        .def_readwrite("underline", &colored_json::Color::underline)
        .def("to_ansi", &colored_json::Color::toAnsi, 
             py::arg("mode") = colored_json::ColorMode::Auto)
        .def_static("from_rgb", &colored_json::Color::fromRgb);
    
    py::class_<colored_json::Style>(m, "Style")
        .def(py::init<>())
        .def_readwrite("key_color", &colored_json::Style::key_color)
        .def_readwrite("string_color", &colored_json::Style::string_color)
        .def_readwrite("number_color", &colored_json::Style::number_color)
        .def_readwrite("bool_color", &colored_json::Style::bool_color)
        .def_readwrite("null_color", &colored_json::Style::null_color)
        .def_readwrite("brace_color", &colored_json::Style::brace_color)
        .def_readwrite("bracket_color", &colored_json::Style::bracket_color)
        .def_readwrite("colon_color", &colored_json::Style::colon_color)
        .def_readwrite("comma_color", &colored_json::Style::comma_color)
        .def_readwrite("key_quote_color", &colored_json::Style::key_quote_color)
        .def_readwrite("string_quote_color", &colored_json::Style::string_quote_color)
        .def_property("key_colors",
            [](colored_json::Style& s) -> colored_json::KeyColorsMap& { return s.key_colors; },
            [](colored_json::Style& s, const colored_json::KeyColorsMap& v) { s.key_colors = v; })
        .def_property("value_colors",
            [](colored_json::Style& s) -> colored_json::ValueColorsMap& { return s.value_colors; },
            [](colored_json::Style& s, const colored_json::ValueColorsMap& v) { s.value_colors = v; })
        .def("set_key_color", &colored_json::Style::setKeyColor,
             py::arg("key"), py::arg("color"),
             "Stel individuele kleur in voor een specifieke key")
        .def("set_value_color", &colored_json::Style::setValueColor,
             py::arg("path"), py::arg("color"),
             "Stel individuele kleur in voor een value op basis van pad")
        .def_readwrite("color_mode", &colored_json::Style::color_mode)
        .def_readwrite("compact", &colored_json::Style::compact)
        .def_readwrite("indent_size", &colored_json::Style::indent_size)
        .def_static("get_preset", &colored_json::Style::getPreset)
        .def_static("list_presets", &colored_json::Style::listPresets);
    
    py::class_<colored_json::Printer>(m, "Printer")
        .def(py::init<const colored_json::Style&>())
        .def("print", &colored_json::Printer::print, 
             py::arg("obj"),
             "Print een Python object met kleuren");
    
    // Convenience functies
    m.def("print", [](py::handle obj, const colored_json::Style& style) {
        colored_json::Printer printer(style);
        py::print(printer.print(obj));
    }, py::arg("obj"), py::arg("style") = colored_json::Style{},
      "Print direct naar console");
    
    m.def("format", [](py::handle obj, const colored_json::Style& style) {
        colored_json::Printer printer(style);
        return printer.print(obj);
    }, py::arg("obj"), py::arg("style") = colored_json::Style{},
      "Geef gekleurde string terug");
    
    // HTML export
    py::class_<colored_json::HtmlPrinter>(m, "HtmlPrinter")
        .def(py::init<const colored_json::Style&>())
        .def("print", &colored_json::HtmlPrinter::print,
             py::arg("obj"),
             py::arg("title") = "Colored JSON",
             py::arg("background_color") = "#1e1e1e",
             py::arg("font_family") = "Consolas, 'Courier New', monospace",
             "Genereer HTML output");
    
    m.def("to_html", [](py::handle obj, const colored_json::Style& style,
                        const std::string& title,
                        const std::string& background_color,
                        const std::string& font_family) {
        colored_json::HtmlPrinter printer(style);
        return printer.print(obj, title, background_color, font_family);
    }, py::arg("obj"),
        py::arg("style") = colored_json::Style{},
        py::arg("title") = "Colored JSON",
        py::arg("background_color") = "#1e1e1e",
        py::arg("font_family") = "Consolas, 'Courier New', monospace",
        "Genereer HTML output van gekleurde JSON");
    
    // Expose builtin colors
    py::module_ colors = m.def_submodule("colors", "Ingebouwde kleuren");
    colors.attr("black") = colored_json::colors::black;
    colors.attr("red") = colored_json::colors::red;
    colors.attr("green") = colored_json::colors::green;
    colors.attr("yellow") = colored_json::colors::yellow;
    colors.attr("blue") = colored_json::colors::blue;
    colors.attr("magenta") = colored_json::colors::magenta;
    colors.attr("cyan") = colored_json::colors::cyan;
    colors.attr("white") = colored_json::colors::white;
    colors.attr("bright_black") = colored_json::colors::bright_black;
    colors.attr("bright_red") = colored_json::colors::bright_red;
    colors.attr("bright_green") = colored_json::colors::bright_green;
    colors.attr("bright_yellow") = colored_json::colors::bright_yellow;
    colors.attr("bright_blue") = colored_json::colors::bright_blue;
    colors.attr("bright_magenta") = colored_json::colors::bright_magenta;
    colors.attr("bright_cyan") = colored_json::colors::bright_cyan;
    colors.attr("bright_white") = colored_json::colors::bright_white;
}

