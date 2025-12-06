#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "printer.hpp"
#include "html_printer.hpp"
#include "markdown_printer.hpp"
#include "style.hpp"

namespace py = pybind11;

PYBIND11_MODULE(colored_json, m) {
    m.doc() = R"doc(
        Razendsnelle gekleurde JSON/dict printer voor Python.
        
        Deze module biedt een C++ geïmplementeerde, hoogperformante oplossing voor het
        weergeven van Python dicts en lists als gekleurde JSON. Ondersteunt presets,
        individuele kleuring, en export naar console, HTML en Markdown.
        
        Voorbeelden:
            >>> import colored_json
            >>> data = {"name": "Alice", "age": 30}
            >>> colored_json.print(data)
            >>> html = colored_json.to_html(data)
        )doc";
    
    py::enum_<colored_json::ColorMode>(m, "ColorMode", 
        R"doc(
            Kleur modus voor ANSI escape codes.
            
            Bepaalt hoe RGB kleuren worden geconverteerd naar terminal kleuren.
            AUTO detecteert automatisch de beste modus op basis van de terminal.
        )doc")
        .value("AUTO", colored_json::ColorMode::Auto, 
               "Automatische detectie van terminal capabilities")
        .value("ANSI16", colored_json::ColorMode::Ansi16, 
               "16 kleuren mode (basis ANSI)")
        .value("ANSI256", colored_json::ColorMode::Ansi256, 
               "256 kleuren mode (uitgebreide ANSI)")
        .value("TRUECOLOR", colored_json::ColorMode::TrueColor, 
               "24-bit truecolor mode (RGB)")
        .value("DISABLED", colored_json::ColorMode::Disabled, 
               "Geen kleuren (alleen tekst)");
    
    py::enum_<colored_json::ElementType>(m, "ElementType",
        R"doc(
            Type elementen in JSON structuur.
            
            Gebruikt voor categorisering van verschillende delen van de JSON output.
        )doc")
        .value("KEY", colored_json::ElementType::Key, "Dictionary keys")
        .value("STRING", colored_json::ElementType::String, "String waarden")
        .value("NUMBER", colored_json::ElementType::Number, "Numerieke waarden")
        .value("BOOLEAN", colored_json::ElementType::Boolean, "Boolean waarden (True/False)")
        .value("NULL", colored_json::ElementType::Null, "Null waarden")
        .value("BRACE", colored_json::ElementType::Brace, "Accolades { }")
        .value("BRACKET", colored_json::ElementType::Bracket, "Brackets [ ]")
        .value("COLON", colored_json::ElementType::Colon, "Dubbele punt :")
        .value("COMMA", colored_json::ElementType::Comma, "Komma ,")
        .value("WHITESPACE", colored_json::ElementType::Whitespace, "Whitespace (indentatie)");
    
    py::class_<colored_json::Color>(m, "Color",
        R"doc(
            RGB kleur met optionele tekst styling.
            
            Representeert een kleur met RGB waarden (0-255) en optionele
            tekst formatting opties zoals bold, italic en underline.
            
            Attributes:
                r (int): Rood component (0-255)
                g (int): Groen component (0-255)
                b (int): Blauw component (0-255)
                bold (bool): Vetgedrukt tekst
                italic (bool): Cursieve tekst
                underline (bool): Onderstreepte tekst
            
            Voorbeelden:
                >>> color = colored_json.Color(255, 0, 0)  # Rood
                >>> color.bold = True
                >>> color2 = colored_json.Color.from_rgb(0, 255, 0)  # Groen
            )doc")
        .def(py::init<uint8_t, uint8_t, uint8_t>(),
             py::arg("r"), py::arg("g"), py::arg("b"),
             R"doc(
                Maak een nieuwe Color aan.
                
                Args:
                    r: Rood component (0-255)
                    g: Groen component (0-255)
                    b: Blauw component (0-255)
            )doc")
        .def_readwrite("r", &colored_json::Color::r, "Rood component (0-255)")
        .def_readwrite("g", &colored_json::Color::g, "Groen component (0-255)")
        .def_readwrite("b", &colored_json::Color::b, "Blauw component (0-255)")
        .def_readwrite("bold", &colored_json::Color::bold, "Vetgedrukt tekst")
        .def_readwrite("italic", &colored_json::Color::italic, "Cursieve tekst")
        .def_readwrite("underline", &colored_json::Color::underline, "Onderstreepte tekst")
        .def("to_ansi", &colored_json::Color::toAnsi, 
             py::arg("mode") = colored_json::ColorMode::Auto,
             R"doc(
                Converteer naar ANSI escape code string.
                
                Args:
                    mode: ColorMode voor conversie (standaard: AUTO)
                
                Returns:
                    str: ANSI escape code string
            )doc")
        .def_static("from_rgb", &colored_json::Color::fromRgb,
                    py::arg("r"), py::arg("g"), py::arg("b"),
                    R"doc(
                        Maak Color van RGB waarden (factory method).
                        
                        Args:
                            r: Rood component (0-255)
                            g: Groen component (0-255)
                            b: Blauw component (0-255)
                        
                        Returns:
                            Color: Nieuwe Color instance
                    )doc");
    
    py::class_<colored_json::Style>(m, "Style",
        R"doc(
            Stijl configuratie voor kleuring en formatting van JSON.
            
            Bevat alle kleuren voor verschillende element types, formatting opties,
            en ondersteuning voor individuele kleuren per key/veld. Kan worden
            geïnitialiseerd met een preset of volledig custom worden geconfigureerd.
            
            Attributes:
                key_color (Color): Kleur voor dictionary keys
                string_color (Color): Kleur voor string waarden
                number_color (Color): Kleur voor numerieke waarden
                bool_color (Color): Kleur voor boolean waarden
                null_color (Color): Kleur voor null waarden
                brace_color (Color): Kleur voor accolades { }
                bracket_color (Color): Kleur voor brackets [ ]
                colon_color (Color): Kleur voor dubbele punten :
                comma_color (Color): Kleur voor komma's ,
                key_quote_color (Color): Kleur voor aanhalingstekens rond keys
                string_quote_color (Color): Kleur voor aanhalingstekens rond strings
                key_colors (dict): Dictionary voor individuele key kleuren
                value_colors (dict): Dictionary voor individuele value kleuren
                color_mode (ColorMode): Kleur modus voor ANSI conversie
                compact (bool): Compact mode (geen indentatie)
                indent_size (int): Aantal spaties per indent niveau
            
            Voorbeelden:
                >>> style = colored_json.Style.get_preset("dracula")
                >>> style = colored_json.Style()
                >>> style.key_color = colored_json.Color(255, 0, 0)
                >>> style.set_key_color("user", colored_json.Color(0, 255, 0))
            )doc")
        .def(py::init<>(),
             R"doc(
                Maak een nieuwe Style met standaard kleuren.
            )doc")
        .def_readwrite("key_color", &colored_json::Style::key_color, 
                       "Kleur voor dictionary keys")
        .def_readwrite("string_color", &colored_json::Style::string_color, 
                       "Kleur voor string waarden")
        .def_readwrite("number_color", &colored_json::Style::number_color, 
                       "Kleur voor numerieke waarden")
        .def_readwrite("bool_color", &colored_json::Style::bool_color, 
                       "Kleur voor boolean waarden")
        .def_readwrite("null_color", &colored_json::Style::null_color, 
                       "Kleur voor null waarden")
        .def_readwrite("brace_color", &colored_json::Style::brace_color, 
                       "Kleur voor accolades { }")
        .def_readwrite("bracket_color", &colored_json::Style::bracket_color, 
                       "Kleur voor brackets [ ]")
        .def_readwrite("colon_color", &colored_json::Style::colon_color, 
                       "Kleur voor dubbele punten :")
        .def_readwrite("comma_color", &colored_json::Style::comma_color, 
                       "Kleur voor komma's ,")
        .def_readwrite("key_quote_color", &colored_json::Style::key_quote_color, 
                       "Kleur voor aanhalingstekens rond keys")
        .def_readwrite("string_quote_color", &colored_json::Style::string_quote_color, 
                       "Kleur voor aanhalingstekens rond strings")
        .def_property("key_colors", 
            [](colored_json::Style& s) -> colored_json::KeyColorsMap& { return s.key_colors; },
            [](colored_json::Style& s, const colored_json::KeyColorsMap& v) { s.key_colors = v; },
            "Dictionary voor individuele key kleuren (key -> Color)")
        .def_property("value_colors",
            [](colored_json::Style& s) -> colored_json::ValueColorsMap& { return s.value_colors; },
            [](colored_json::Style& s, const colored_json::ValueColorsMap& v) { s.value_colors = v; },
            "Dictionary voor individuele value kleuren (path -> Color)")
        .def("set_key_color", &colored_json::Style::setKeyColor,
             py::arg("key"), py::arg("color"),
             R"doc(
                Stel individuele kleur in voor een specifieke key.
                
                Deze kleur wordt gebruikt voor alle keys met de gegeven naam,
                ongeacht waar ze voorkomen in de JSON structuur.
                
                Args:
                    key: De key naam (bijv. "user", "name", "status")
                    color: Color object voor deze key
                
                Voorbeelden:
                    >>> style.set_key_color("user", colored_json.Color(255, 0, 0))
                    >>> style.set_key_color("name", colored_json.colors.bright_green)
            )doc")
        .def("set_value_color", &colored_json::Style::setValueColor,
             py::arg("path"), py::arg("color"),
             R"doc(
                Stel individuele kleur in voor een value op basis van pad.
                
                Het pad kan eenvoudige keys zijn of geneste paths met punt notatie.
                
                Args:
                    path: Pad naar de value (bijv. "user.name", "products[0].price")
                    color: Color object voor deze value
                
                Pad notatie:
                    - Eenvoudige keys: "user", "status"
                    - Geneste keys: "user.name", "user.age"
                    - Array items: "products[0].name", "products[1].price"
                
                Voorbeelden:
                    >>> style.set_value_color("user.name", colored_json.Color(255, 20, 147))
                    >>> style.set_value_color("products[0].price", colored_json.Color(255, 215, 0))
            )doc")
        .def_readwrite("color_mode", &colored_json::Style::color_mode, 
                       "Kleur modus voor ANSI conversie (ColorMode)")
        .def_readwrite("compact", &colored_json::Style::compact, 
                       "Compact mode: True = geen indentatie, False = geïndenteerd")
        .def_readwrite("indent_size", &colored_json::Style::indent_size, 
                       "Aantal spaties per indent niveau (standaard: 2)")
        .def_static("get_preset", &colored_json::Style::getPreset,
                    py::arg("name"),
                    R"doc(
                        Laad een preset thema.
                        
                        Beschikbare presets: default, dracula, solarized, monokai,
                        github, minimal, neon
                        
                        Args:
                            name: Naam van de preset
                        
                        Returns:
                            Style: Style object met preset configuratie
                        
                        Raises:
                            RuntimeError: Als preset naam niet bestaat
                        
                        Voorbeelden:
                            >>> style = colored_json.Style.get_preset("dracula")
                            >>> style = colored_json.Style.get_preset("monokai")
                    )doc")
        .def_static("list_presets", &colored_json::Style::listPresets,
                    R"doc(
                        Lijst van beschikbare preset thema's.
                        
                        Returns:
                            list: Lijst van preset namen
                        
                        Voorbeelden:
                            >>> presets = colored_json.Style.list_presets()
                            >>> print(presets)
                            ['default', 'dracula', 'solarized', ...]
                    )doc");
    
    py::class_<colored_json::Printer>(m, "Printer",
        R"doc(
            Printer class voor console output met ANSI kleuren.
            
            Gebruikt een Style configuratie om Python objecten te formatteren
            als gekleurde JSON strings met ANSI escape codes.
            
            Voorbeelden:
                >>> style = colored_json.Style.get_preset("default")
                >>> printer = colored_json.Printer(style)
                >>> result = printer.print({"name": "Alice"})
            )doc")
        .def(py::init<const colored_json::Style&>(),
             py::arg("style"),
             R"doc(
                Maak een nieuwe Printer instance.
                
                Args:
                    style: Style object voor kleuring en formatting
            )doc")
        .def("print", &colored_json::Printer::print, 
             py::arg("obj"),
             R"doc(
                Formatteer en print een Python object.
                
                Args:
                    obj: Python dict, list, of ander object om te formatteren
                
                Returns:
                    str: Gekleurde string met ANSI escape codes
            )doc");
    
    // Convenience functies
    m.def("print", [](py::handle obj, const colored_json::Style& style) {
        colored_json::Printer printer(style);
        py::print(printer.print(obj));
    }, py::arg("obj"), py::arg("style") = colored_json::Style{},
      R"doc(
        Print een Python object direct naar de console met kleuren.
        
        Dit is een convenience functie die een Printer maakt en direct print.
        Gebruik format() als je de string wilt opslaan in plaats van printen.
        
        Args:
            obj: Python dict, list, of ander object om te printen
            style: Optionele Style object (standaard: Style())
        
        Voorbeelden:
            >>> colored_json.print({"name": "Alice", "age": 30})
            >>> style = colored_json.Style.get_preset("dracula")
            >>> colored_json.print(data, style)
      )doc");
    
    m.def("format", [](py::handle obj, const colored_json::Style& style) {
        colored_json::Printer printer(style);
        return printer.print(obj);
    }, py::arg("obj"), py::arg("style") = colored_json::Style{},
      R"doc(
        Formatteer een Python object als gekleurde string.
        
        Retourneert een string met ANSI escape codes zonder direct te printen.
        Handig voor opslaan in variabelen of verdere verwerking.
        
        Args:
            obj: Python dict, list, of ander object om te formatteren
            style: Optionele Style object (standaard: Style())
        
        Returns:
            str: Gekleurde string met ANSI escape codes
        
        Voorbeelden:
            >>> result = colored_json.format({"name": "Alice"})
            >>> print(result)
            >>> with open("output.txt", "w") as f:
            ...     f.write(colored_json.format(data, style))
      )doc");
    
    // HTML export
    py::class_<colored_json::HtmlPrinter>(m, "HtmlPrinter",
        R"doc(
            Printer class voor HTML output generatie.
            
            Genereert complete HTML pagina's met gekleurde JSON. Gebruikt inline
            CSS styles voor portabiliteit. Ondersteunt alle styling features
            inclusief presets en individuele kleuren.
            
            Voorbeelden:
                >>> style = colored_json.Style.get_preset("dracula")
                >>> printer = colored_json.HtmlPrinter(style)
                >>> html = printer.print(data, title="My JSON", background_color="#282a36")
            )doc")
        .def(py::init<const colored_json::Style&>(),
             py::arg("style"),
             R"doc(
                Maak een nieuwe HtmlPrinter instance.
                
                Args:
                    style: Style object voor kleuring en formatting
            )doc")
        .def("print", &colored_json::HtmlPrinter::print,
             py::arg("obj"),
             py::arg("title") = "Colored JSON",
             py::arg("background_color") = "#1e1e1e",
             py::arg("font_family") = "Consolas, 'Courier New', monospace",
             R"doc(
                Genereer complete HTML pagina met gekleurde JSON.
                
                Args:
                    obj: Python dict, list, of ander object om te formatteren
                    title: Titel voor de HTML pagina
                    background_color: Achtergrondkleur (CSS kleur string)
                    font_family: Font familie voor de JSON weergave
                
                Returns:
                    str: Complete HTML string met inline styles
            )doc");
    
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
        R"doc(
            Genereer HTML output van gekleurde JSON (convenience functie).
            
            Maakt een complete HTML pagina met gekleurde JSON. Alle styling
            features worden ondersteund, inclusief presets en individuele kleuren.
            
            Args:
                obj: Python dict, list, of ander object om te formatteren
                style: Optionele Style object (standaard: Style())
                title: Titel voor de HTML pagina
                background_color: Achtergrondkleur (CSS kleur string, bijv. "#1e1e1e")
                font_family: Font familie voor de JSON weergave
            
            Returns:
                str: Complete HTML string met inline styles
            
            Voorbeelden:
                >>> html = colored_json.to_html(data, style, title="My JSON")
                >>> with open("output.html", "w", encoding="utf-8") as f:
                ...     f.write(html)
        )doc");
    
    // Markdown export
    py::class_<colored_json::MarkdownPrinter>(m, "MarkdownPrinter",
        R"doc(
            Printer class voor Markdown output generatie.
            
            Genereert Markdown met code blocks. Biedt twee modi:
            - print(): Standaard Markdown code block (zonder kleuren)
            - print_html(): Markdown met HTML code block (gekleurd, werkt in GitHub/GitLab)
            
            Voorbeelden:
                >>> style = colored_json.Style.get_preset("default")
                >>> printer = colored_json.MarkdownPrinter(style)
                >>> md = printer.print(data, title="Example")
            )doc")
        .def(py::init<const colored_json::Style&>(),
             py::arg("style"),
             R"doc(
                Maak een nieuwe MarkdownPrinter instance.
                
                Args:
                    style: Style object voor kleuring en formatting
            )doc")
        .def("print", &colored_json::MarkdownPrinter::print,
             py::arg("obj"),
             py::arg("title") = "Colored JSON",
             py::arg("language") = "json",
             R"doc(
                Genereer Markdown met code block (zonder kleuren).
                
                Standaard Markdown code block. Kleuren worden niet behouden,
                maar de JSON formatting blijft intact.
                
                Args:
                    obj: Python dict, list, of ander object om te formatteren
                    title: Titel voor de Markdown sectie (optioneel)
                    language: Code block language identifier (standaard: "json")
                
                Returns:
                    str: Markdown string met code block
            )doc")
        .def("print_html", &colored_json::MarkdownPrinter::printHtml,
             py::arg("obj"),
             py::arg("title") = "Colored JSON",
             py::arg("background_color") = "#1e1e1e",
             py::arg("font_family") = "Consolas, 'Courier New', monospace",
             R"doc(
                Genereer Markdown met HTML code block voor gekleurde JSON.
                
                Gebruikt HTML in Markdown voor kleuren. Werkt in Markdown
                renderers die HTML ondersteunen (GitHub, GitLab, etc.).
                
                Args:
                    obj: Python dict, list, of ander object om te formatteren
                    title: Titel voor de Markdown sectie (optioneel)
                    background_color: Achtergrondkleur (CSS kleur string)
                    font_family: Font familie voor de JSON weergave
                
                Returns:
                    str: Markdown string met HTML code block
            )doc");
    
    m.def("to_markdown", [](py::handle obj, const colored_json::Style& style,
                           const std::string& title,
                           const std::string& language) {
        colored_json::MarkdownPrinter printer(style);
        return printer.print(obj, title, language);
    }, py::arg("obj"),
        py::arg("style") = colored_json::Style{},
        py::arg("title") = "Colored JSON",
        py::arg("language") = "json",
        R"doc(
            Genereer Markdown output met code block (convenience functie).
            
            Standaard Markdown code block zonder kleuren. De JSON formatting
            blijft behouden maar kleuren worden niet weergegeven.
            
            Args:
                obj: Python dict, list, of ander object om te formatteren
                style: Optionele Style object (standaard: Style())
                title: Titel voor de Markdown sectie
                language: Code block language identifier (standaard: "json")
            
            Returns:
                str: Markdown string met code block
            
            Voorbeelden:
                >>> md = colored_json.to_markdown(data, style, title="Example")
                >>> with open("output.md", "w", encoding="utf-8") as f:
                ...     f.write(md)
        )doc");
    
    m.def("to_markdown_html", [](py::handle obj, const colored_json::Style& style,
                                 const std::string& title,
                                 const std::string& background_color,
                                 const std::string& font_family) {
        colored_json::MarkdownPrinter printer(style);
        return printer.printHtml(obj, title, background_color, font_family);
    }, py::arg("obj"),
        py::arg("style") = colored_json::Style{},
        py::arg("title") = "Colored JSON",
        py::arg("background_color") = "#1e1e1e",
        py::arg("font_family") = "Consolas, 'Courier New', monospace",
        R"doc(
            Genereer Markdown met HTML code block voor gekleurde JSON (convenience functie).
            
            Gebruikt HTML in Markdown voor kleuren. Werkt in Markdown renderers
            die HTML ondersteunen zoals GitHub, GitLab, en veel andere platforms.
            
            Args:
                obj: Python dict, list, of ander object om te formatteren
                style: Optionele Style object (standaard: Style())
                title: Titel voor de Markdown sectie
                background_color: Achtergrondkleur (CSS kleur string, bijv. "#1e1e1e")
                font_family: Font familie voor de JSON weergave
            
            Returns:
                str: Markdown string met HTML code block
            
            Voorbeelden:
                >>> md = colored_json.to_markdown_html(data, style, title="Colored JSON")
                >>> with open("colored.md", "w", encoding="utf-8") as f:
                ...     f.write(md)
        )doc");
    
    // Expose builtin colors
    py::module_ colors = m.def_submodule("colors", 
        R"doc(
            Ingebouwde ANSI16 kleuren.
            
            Vooraf gedefinieerde Color objecten voor veelgebruikte kleuren.
            Gebaseerd op standaard ANSI16 terminal kleuren.
            
            Beschikbare kleuren:
                - black, red, green, yellow, blue, magenta, cyan, white
                - bright_black, bright_red, bright_green, bright_yellow,
                  bright_blue, bright_magenta, bright_cyan, bright_white
            
            Voorbeelden:
                >>> style.key_color = colored_json.colors.bright_cyan
                >>> style.string_color = colored_json.colors.bright_green
        )doc");
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

