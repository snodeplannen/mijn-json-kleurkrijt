# JSON String Input Implementatie Plan

## Doel
Toevoegen van JSON string input ondersteuning met simdjson voor razendsnelle performance met GIL release.

## Keuzes
1. **simdjson** - SIMD-optimized JSON parser (sneller, vereist linking)
2. **Dual mode + expliciete functies** - Auto-detect + aparte functies
3. **Alle functies** - format, to_html, to_markdown, etc. ondersteunen JSON strings

## Architectuur

### Dual Mode Strategie
- **Auto-detect**: Check of input string is, probeer JSON parsing
- **Expliciete functies**: `format_from_json()`, `to_html_from_json()`, etc.
- **Fallback**: Als JSON parsing faalt, gebruik huidige Python object implementatie

### GIL Release Strategie
- **JSON parsing**: ZONDER GIL (pure C++)
- **String formatting**: ZONDER GIL (pure C++)
- **Input/output conversie**: MET GIL (Python string <-> C++ string)
- **Python object handling**: MET GIL (huidige implementatie)

## Implementatie Stappen

### Stap 1: simdjson Dependency Toevoegen
- Voeg simdjson toe aan CMakeLists.txt
- Download/bundel simdjson (header-only of via git submodule)
- Test compilatie

### Stap 2: Nieuwe Printer Methoden
- `printFromJson(simdjson::ondemand::value)` - voor parsed JSON
- Houd huidige `print(py::handle)` voor Python objects
- Deel code tussen beide methoden waar mogelijk

### Stap 3: JSON Parser Wrapper
- Maak `JsonParser` class die simdjson wrapt
- Error handling voor invalid JSON
- Path tracking voor individuele kleuren

### Stap 4: Dual Mode in Bindings
- Check input type (string vs dict/list)
- Als string: probeer JSON parsing, gebruik `printFromJson()`
- Als dict/list: gebruik huidige `print()` method
- Voeg `py::call_guard<py::gil_scoped_release>()` toe voor JSON string path

### Stap 5: Expliciete Functies
- `format_from_json(json_string, style)`
- `to_html_from_json(json_string, style, ...)`
- `to_markdown_from_json(json_string, style, ...)`
- `to_markdown_html_from_json(json_string, style, ...)`

### Stap 6: Update Alle Functies
- `format()` - ondersteun string input
- `to_html()` - ondersteun string input
- `to_markdown()` - ondersteun string input
- `to_markdown_html()` - ondersteun string input
- Printer classes: `print()` methoden ondersteunen string input

## Bestanden te Wijzigen

### Nieuwe Bestanden
1. **`src/json_parser.hpp`** - simdjson wrapper en JSON parsing logic
2. **`src/json_printer.hpp`** - Printer methoden voor simdjson::ondemand::value

### Te Wijzigen Bestanden
1. **`CMakeLists.txt`** - Voeg simdjson dependency toe
2. **`src/printer.hpp`** - Voeg `printFromJson()` method toe
3. **`src/html_printer.hpp`** - Voeg `printFromJson()` method toe
4. **`src/markdown_printer.hpp`** - Voeg `printFromJson()` method toe
5. **`src/bindings.cpp`** - Dual mode logic + expliciete functies + GIL release
6. **`colored_json.pyi`** - Type hints voor nieuwe functies
7. **`README.md`** - Documentatie voor JSON string input

## Technische Details

### simdjson Integratie
```cpp
#include <simdjson.h>
using namespace simdjson;

// In printFromJson():
ondemand::parser parser;
padded_string json = padded_string::load(json_string);
ondemand::document doc = parser.iterate(json);
```

### GIL Release Pattern
```cpp
std::string formatFromJson(const std::string& json_str, const Style& style) {
    // 1. Convert Python string to C++ (MET GIL)
    std::string json_cpp = json_str;
    
    // 2. Release GIL voor parsing en formatting
    py::gil_scoped_release release;
    
    // 3. Parse JSON (ZONDER GIL)
    JsonParser parser;
    auto doc = parser.parse(json_cpp);
    
    // 4. Format JSON (ZONDER GIL)
    Printer printer(style);
    return printer.printFromJson(doc);
    
    // 5. GIL wordt automatisch gere-acquired bij return
}
```

### Dual Mode Pattern
```cpp
m.def("format", [](py::handle obj, const Style& style) {
    // Check of input string is
    if (py::isinstance<py::str>(obj)) {
        // JSON string path
        std::string json_str = py::str(obj);
        return formatFromJson(json_str, style);
    } else {
        // Python object path (huidige implementatie)
        Printer printer(style);
        return printer.print(obj);
    }
}, py::call_guard<py::gil_scoped_release>());
```

## Error Handling

### JSON Parsing Errors
- Catch simdjson exceptions
- Convert naar Python ValueError met duidelijke error message
- Fallback naar Python object parsing als JSON parsing faalt (optioneel)

### Type Detection
- Check `py::isinstance<py::str>()` voor string input
- Check `py::isinstance<py::dict>()` of `py::isinstance<py::list>()` voor object input
- Fallback naar `printValue()` voor andere types

## Performance Optimalisaties

1. **String Copy Minimization**
   - Gebruik `padded_string` van simdjson direct waar mogelijk
   - Vermijd onnodige string kopieën

2. **Path Tracking**
   - Implementeer path tracking voor simdjson::ondemand::value
   - Ondersteun individuele kleuren voor JSON string input

3. **Memory Management**
   - Reuse simdjson parser instances waar mogelijk
   - Pre-allocate string buffers

## Testing

1. **Unit Tests**
   - Test JSON string parsing
   - Test dual mode auto-detect
   - Test expliciete functies
   - Test error handling

2. **Performance Tests**
   - Benchmark JSON string vs Python object input
   - Vergelijk met huidige implementatie
   - Test multi-threaded scenarios

3. **Integration Tests**
   - Test alle functies met JSON string input
   - Test fallback naar Python object parsing
   - Test individuele kleuren met JSON strings

## Risico's en Mitigatie

**Risico 1**: simdjson dependency complexiteit
- **Mitigatie**: Gebruik header-only versie of git submodule

**Risico 2**: Path tracking voor individuele kleuren met JSON
- **Mitigatie**: Implementeer path tracking tijdens JSON traversal

**Risico 3**: Error handling tussen JSON en Python object paths
- **Mitigatie**: Duidelijke error messages, fallback mechanisme

**Risico 4**: GIL release tijdens JSON parsing
- **Mitigatie**: Zorg dat alle Python API calls gebeuren VOOR GIL release

## Performance Impact (Verwacht)

- **Kleine datasets** (<100 items): 2-3x sneller
- **Middelgrote datasets** (100-1000 items): 3-5x sneller  
- **Grote datasets** (>1000 items): 5-10x sneller
- **Multi-threaded**: Nog sneller door GIL release

## Volgorde van Implementatie

1. Voeg simdjson toe aan CMakeLists.txt
2. Maak json_parser.hpp met simdjson wrapper
3. Implementeer printFromJson() in printer.hpp
4. Test JSON parsing en formatting
5. Voeg dual mode logic toe aan bindings.cpp
6. Implementeer expliciete functies
7. Update html_printer.hpp en markdown_printer.hpp
8. Voeg GIL release toe
9. Update type hints en documentatie
10. Test en benchmark

