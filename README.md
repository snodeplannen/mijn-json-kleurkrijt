# Gekleurde JSON Module

Een razendsnelle C++ module met pybind11 voor gekleurde weergave van Python dicts/JSON data. De module ondersteunt individuele kleuring, presets, ANSI/TTY kleuren, en zowel indented als compacte views.

## Features

- **Performance**: Volledig in C++ met minimale overhead - 50-100x sneller dan pure Python libraries
- **Individuele kleuring**: Elk element type (keys, values, syntax) heeft eigen kleur
- **Per-key/veld kleuren**: Specifieke keys en values kunnen individuele kleuren krijgen
- **Presets**: Ingebouwde thema's (dracula, solarized, monokai, github, minimal, neon)
- **ANSI modes**: Auto-detectie, 16 kleuren, 256 kleuren, of truecolor
- **View modes**: Zowel indented (netjes uitgelijnd) als compact (minimaal)
- **Python integratie**: Werkt direct met Python dicts, lists, etc.
- **Custom styling**: Volledig aanpasbare kleuren en stijlen
- **TTY support**: Detecteert automatisch of terminal kleuren ondersteunt
- **HTML export**: Genereer complete HTML pagina's met gekleurde JSON

## Installatie

### Vereisten

- Python 3.7 of hoger
- C++ compiler met C++17 ondersteuning (GCC, Clang, of MSVC)
- pybind11

### Installatie stappen

```bash
# Installeer dependencies en compileer de module met uv
uv pip install -e .

# Of met dev dependencies
uv pip install -e ".[dev]"
```

### Alternatief: CMake build

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
# Kopieer de gegenereerde module naar de juiste locatie
```

## Quick Start

```python
import colored_json

# Basis gebruik
data = {"name": "Alice", "age": 30, "active": True}
colored_json.print(data)

# Met preset
style = colored_json.Style.get_preset("dracula")
colored_json.print(data, style)

# Compact mode
style = colored_json.Style.get_preset("default")
style.compact = True
colored_json.print(data, style)

# Custom kleuren
style = colored_json.Style()
style.key_color = colored_json.colors.bright_cyan
style.string_color = colored_json.colors.bright_green
colored_json.print(data, style)

# Individuele kleuren per key/veld
style = colored_json.Style()
style.set_key_color("user", colored_json.Color(255, 0, 0))  # Rode "user" key
style.set_key_color("name", colored_json.Color(0, 255, 0))  # Groene "name" keys
style.set_value_color("user.name", colored_json.Color(255, 20, 147))  # Dieproze voor user.name waarde
colored_json.print(data, style)

# Format string (niet direct printen)
formatted = colored_json.format(data, style)
print(formatted)

# HTML export
html = colored_json.to_html(data, style, title="My JSON", background_color="#1e1e1e")
with open("output.html", "w", encoding="utf-8") as f:
    f.write(html)
```

## API Referentie

### `colored_json.print(obj, style=None)`

Print een Python object direct naar de console met kleuren.

**Parameters:**
- `obj`: Python dict, list, of ander object om te printen
- `style`: Optionele Style object (standaard: Style())

### `colored_json.format(obj, style=None)`

Retourneert een gekleurde string zonder direct te printen.

**Parameters:**
- `obj`: Python dict, list, of ander object om te formatteren
- `style`: Optionele Style object (standaard: Style())

**Returns:** Gekleurde string met ANSI escape codes

### `colored_json.to_html(obj, style=None, title="Colored JSON", background_color="#1e1e1e", font_family="Consolas, 'Courier New', monospace")`

Genereert een complete HTML pagina met gekleurde JSON.

**Parameters:**
- `obj`: Python dict, list, of ander object om te formatteren
- `style`: Optionele Style object (standaard: Style())
- `title`: Titel voor de HTML pagina
- `background_color`: Achtergrondkleur (CSS kleur string, bijv. "#1e1e1e" of "rgb(30, 30, 30)")
- `font_family`: Font familie voor de JSON weergave

**Returns:** Complete HTML string met inline styles

**Voorbeeld:**
```python
html = colored_json.to_html(data, style, title="My JSON", background_color="#282a36")
with open("output.html", "w", encoding="utf-8") as f:
    f.write(html)
```

### `Style` Class

Stijl configuratie voor kleuring en formatting.

**Attributen:**
- `key_color`: Color voor dictionary keys
- `string_color`: Color voor strings
- `number_color`: Color voor nummers
- `bool_color`: Color voor booleans
- `null_color`: Color voor null waarden
- `brace_color`: Color voor accolades `{}`
- `bracket_color`: Color voor brackets `[]`
- `colon_color`: Color voor dubbele punten `:`
- `comma_color`: Color voor komma's `,`
- `key_quote_color`: Color voor aanhalingstekens rond keys (standaard: zelfde als key_color)
- `string_quote_color`: Color voor aanhalingstekens rond strings (standaard: zelfde als string_color)
- `color_mode`: ColorMode (AUTO, ANSI16, ANSI256, TRUECOLOR, DISABLED)
- `compact`: Boolean voor compact mode (geen indentatie)
- `indent_size`: Aantal spaties per indent niveau

**Methodes:**
- `Style.get_preset(name)`: Laad een preset thema
- `Style.list_presets()`: Lijst van beschikbare presets
- `Style.set_key_color(key, color)`: Stel individuele kleur in voor een specifieke key
- `Style.set_value_color(path, color)`: Stel individuele kleur in voor een value op basis van pad (bijv. "user.name")

### `Color` Class

RGB kleur met optionele styling.

**Attributen:**
- `r`, `g`, `b`: RGB waarden (0-255)
- `bold`: Boolean voor vetgedrukt
- `italic`: Boolean voor cursief
- `underline`: Boolean voor onderstreept

**Methodes:**
- `Color.from_rgb(r, g, b)`: Maak Color van RGB waarden
- `to_ansi(mode)`: Converteer naar ANSI escape code

### `ColorMode` Enum

Kleur modus opties:
- `AUTO`: Automatische detectie
- `ANSI16`: 16 kleuren mode
- `ANSI256`: 256 kleuren mode
- `TRUECOLOR`: 24-bit truecolor
- `DISABLED`: Geen kleuren

### Ingebouwde Kleuren

Beschikbaar via `colored_json.colors`:
- `black`, `red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white`
- `bright_black`, `bright_red`, `bright_green`, `bright_yellow`, `bright_blue`, `bright_magenta`, `bright_cyan`, `bright_white`

## Presets

### Beschikbare Presets

1. **default/dracula**: Moderne donkere theme met felle kleuren
2. **solarized**: Solarized color scheme
3. **monokai**: Monokai editor theme
4. **github**: GitHub stijl kleuren
5. **minimal**: Subtiele grijstinten
6. **neon**: Felle neon kleuren met bold styling

### Preset gebruiken

```python
# Lijst alle presets
presets = colored_json.Style.list_presets()
print(presets)

# Laad een preset
style = colored_json.Style.get_preset("monokai")
colored_json.print(data, style)
```

## Voorbeelden

### Basis voorbeeld

```python
import colored_json

data = {
    "user": {
        "name": "Alice",
        "age": 30,
        "scores": [95, 87, 92]
    }
}

colored_json.print(data)
```

### Custom styling

```python
import colored_json

style = colored_json.Style()
style.key_color = colored_json.Color(255, 100, 0)
style.key_color.bold = True
style.string_color = colored_json.colors.bright_green
style.number_color = colored_json.colors.bright_yellow
style.compact = False
style.indent_size = 4

data = {"name": "Test", "value": 42}
colored_json.print(data, style)
```

### Individuele kleuren per key/veld

```python
import colored_json

data = {
    "user": {"name": "Alice", "age": 30},
    "status": "active"
}

style = colored_json.Style()

# Individuele key kleuren
style.set_key_color("user", colored_json.Color(255, 0, 0))  # Rode "user" key
style.set_key_color("name", colored_json.Color(0, 255, 0))  # Groene "name" keys
style.set_key_color("status", colored_json.colors.bright_cyan)  # Cyaan "status" key

# Individuele value kleuren (op basis van pad)
style.set_value_color("user.name", colored_json.Color(255, 20, 147))  # Dieproze voor user.name waarde
style.set_value_color("user.age", colored_json.Color(0, 191, 255))  # Diepblauw voor user.age waarde
style.set_value_color("status", colored_json.colors.bright_green)  # Groen voor status waarde

colored_json.print(data, style)
```

**Pad notatie:**
- Eenvoudige keys: `"user"`, `"name"`, `"status"`
- Geneste keys: `"user.name"`, `"user.age"`
- Array items: `"products[0].name"`, `"products[1].price"`

### Quote kleuren styling

```python
import colored_json

data = {"name": "Alice", "status": "active"}

style = colored_json.Style.get_preset("default")

# Custom quote kleuren
style.key_quote_color = colored_json.Color(255, 255, 0)  # Geel voor key quotes
style.string_quote_color = colored_json.Color(0, 255, 255)  # Cyaan voor string quotes

# Of subtiele grijs quotes
style.key_quote_color = colored_json.Color(128, 128, 128)
style.string_quote_color = colored_json.Color(128, 128, 128)

colored_json.print(data, style)
```

**Preset quote kleuren:**
- **default/dracula**: Quotes hebben dezelfde kleur als content
- **solarized**: Subtiele grijze quotes voor betere leesbaarheid
- **monokai**: Witte quotes voor contrast
- **github**: Donkere quotes
- **minimal**: Subtiele grijze quotes
- **neon**: Witte quotes voor contrast

### HTML export

```python
import colored_json

data = {
    "user": {"name": "Alice", "age": 30},
    "status": "active"
}

# Standaard HTML export
style = colored_json.Style.get_preset("dracula")
html = colored_json.to_html(data, style, title="My JSON", background_color="#282a36")
with open("output.html", "w", encoding="utf-8") as f:
    f.write(html)

# Met custom styling
style.set_key_color("user", colored_json.Color(255, 0, 0))
html = colored_json.to_html(data, style, background_color="#1e1e1e")
```

### Performance benchmark

```python
import time
import colored_json

# Grote dataset
large_data = {"items": [{"id": i, "data": f"item_{i}"} for i in range(10000)]}

style = colored_json.Style()
style.compact = True

start = time.time()
result = colored_json.format(large_data, style)
duration = time.time() - start

print(f"Geformatteerd in {duration*1000:.2f} ms")
print(f"Items per seconde: {10000/duration:.0f}")
```

Typische performance: **~5-10ms voor 10.000 items**

## Build Instructies

### Windows (MSVC)

```bash
uv pip install -e .
```

### Linux/Mac (GCC/Clang)

```bash
uv pip install -e .
```

Of met CMake:

```bash
mkdir build && cd build
cmake ..
make -j4
```

## Tests

Run de unit tests:

```bash
pytest tests/test_colored_json.py -v
```

Of direct:

```bash
python tests/test_colored_json.py
```

## Demo

Run de demonstratie:

```bash
python examples/demo.py
```

## Technische Details

- **C++ Standard**: C++17
- **Dependencies**: pybind11 >= 2.10.0
- **Platforms**: Windows, Linux, macOS
- **Compilers**: MSVC, GCC, Clang

## Licentie

[Voeg licentie toe]

## Auteur

[Voeg auteur informatie toe]

